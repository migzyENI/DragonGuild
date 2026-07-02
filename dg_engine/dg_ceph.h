//dg_ceph.h
// (C) MigzyENT 2026 - DragonGuild Engine
//
// Crinale Systems Eccentric Pool Header — a SELF-MANAGING fixed-slot pool.
// The pool holds its own dg_allocator* and a grow size, so dg_ceph_pool_proc_alloc
// chains a new slab implicitly when every slab is full. Zero caller intervention;
// only a true backing-allocator OOM fails a growable pool.
//
// Design record: dg_ceph.DESIGN.txt   Milestone: 0.500.750 (retires dg_arena_pool.h).
//
// SCOPE NOTE — CEPH is intended to cover BOTH pool shapes:
//   1. Single fixed-width pool (dg_ceph_pool) ....... THIS FILE, current focus.
//   2. Multi-class pool (size variants 8/16/.../512) ... PLANNED, not yet built.
// The multi-class layer will be a thin wrapper over N fixed dg_ceph_pool instances
// (one per size class, routed by rounding the request up to a class), reusing the
// same slab/bitmask/self-grow machinery below. Retires dg_pool (multi-class) as well
// as dg_pool_fixed. Build it only after the single-width path lands and RSM ships.

#ifndef DG_CEPH_H
#define DG_CEPH_H
#pragma once

#if !defined(__cplusplus) && __STDC_VERSION__ < 202311L
#error "This file requires C23 or later. (dg_ceph)"
#endif

#include <dg_typedef.h>
#include <dg_error.h>
#include <dg_allocator.h>

#include <string.h>

//
//
// Error Codes

constexpr dg_u32 DG_CEPH_DWORD_NAME = 0x43455048; // 'CEPH'

DG_ERRCODE_TABLE_GENERATE(
	DGE_CEPH, _IDX, _COUNT, DG_CEPH_DWORD_NAME,
	NULL_POOL_PTR,
	NULL_OUT_PTR,
	NULL_MEMORY_PTR,
	NULL_SLAB_PTR,
	OUT_OF_MEMORY,
	SIZE_OVERFLOW,
	SIZE_UNINITIALIZED,
	OUT_OF_BOUNDS,
	DOUBLE_FREE,
	NOT_GROWABLE
)

//
//
// Slab / Pool

#ifndef DG_CEPH_SLAB_ATOMICS
#define DG_CEPH_SLAB_ATOMICS 8u
#endif

// One slab tracks its free slots in a fixed atomic bitmask: DG_CEPH_SLAB_ATOMICS
// words of 64 bits. Growth past this is by chaining more slabs, not bigger ones.
constexpr dg_u32 DG_CEPH_SLOTS_PER_SLAB = DG_CEPH_SLAB_ATOMICS * 64u; // 512

typedef struct dg_ceph_slab {
	dg_u32          SlotTotal;
	dg_u32          SlotWidth;
	_Atomic(dg_u64) FreeBits[DG_CEPH_SLAB_ATOMICS]; // bit set => slot free
	dg_byte*        Memory;
	_Atomic(struct dg_ceph_slab*) Next;
} dg_ceph_slab;

// Slabs is a chain whose HEAD (slab[0]) is caller-embedded — dg_ceph_pool_proc_destroy
// never frees it. GrowSlots == 0 (or Alloc == null) makes the pool fixed: alloc
// returns OUT_OF_MEMORY on exhaustion instead of growing.
typedef struct dg_ceph_pool {
	dg_ceph_slab* Slabs;
	_Atomic(dg_ceph_slab*) ActiveSlab; // O(1) alloc hint: slab most likely to hold a free slot
	dg_u32        Width;
	dg_u32        GrowSlots;    // slots per auto-added slab, clamped to 512 at init
	dg_allocator* Alloc;        // backing for auto-grow; must outlive the pool
} dg_ceph_pool;

//
//
// Slab internals

[[nodiscard]] static inline dg_errcode dg_ceph_slab_suro_init( dg_ceph_slab* Slab, dg_u32 SlotTotal, dg_u32 SlotWidth, dg_byte* Memory ) {
	if ( DG_UNLIKELY( Slab == nullptr ) ) {
		return DGE_CEPH_NULL_SLAB_PTR;
	}
	if ( DG_UNLIKELY( Memory == nullptr ) ) {
		return DGE_CEPH_NULL_MEMORY_PTR;
	}
	if ( DG_UNLIKELY( SlotTotal == 0u ) ) {
		return DGE_CEPH_SIZE_UNINITIALIZED;
	}
	if ( DG_UNLIKELY( SlotTotal > DG_CEPH_SLOTS_PER_SLAB ) ) {
		return DGE_CEPH_SIZE_OVERFLOW;
	}

	Slab->SlotTotal = SlotTotal;
	Slab->SlotWidth = SlotWidth;
	Slab->Memory    = Memory;
	Slab->Next      = nullptr;

	dg_u32 FullWords = SlotTotal / 64u;
	dg_u32 Remainder = SlotTotal % 64u;
	for ( dg_u32 Idx = 0; Idx < DG_CEPH_SLAB_ATOMICS; ++Idx ) {
		dg_u64 Mask;
		if ( Idx < FullWords ) {
			Mask = ~0ULL;
		} else if ( Idx == FullWords && Remainder > 0u ) {
			Mask = ( 1ULL << Remainder ) - 1ULL;
		} else {
			Mask = 0ULL;
		}
		atomic_store_explicit( &Slab->FreeBits[Idx], Mask, memory_order_relaxed );
	}
	return DGE_CEPH_NONE;
}

[[nodiscard]] static inline dg_errcode dg_ceph_slab_suro_free_slot( dg_ceph_slab* Slab, dg_u32 Slot ) {
	if ( DG_UNLIKELY( Slab == nullptr ) ) {
		return DGE_CEPH_NULL_SLAB_PTR;
	}
	if ( DG_UNLIKELY( Slot >= Slab->SlotTotal ) ) {
		return DGE_CEPH_OUT_OF_BOUNDS;
	}
	dg_u32 WordIdx = Slot / 64u;
	dg_u32 BitIdx  = Slot % 64u;
	dg_u64 Mask    = 1ULL << BitIdx;
	dg_u64 Prev    = atomic_fetch_or_explicit( &Slab->FreeBits[WordIdx], Mask, memory_order_release );
	if ( DG_UNLIKELY( Prev & Mask ) ) {
		return DGE_CEPH_DOUBLE_FREE;
	}
	return DGE_CEPH_NONE;
}

// Try to claim one free slot from a SINGLE slab. Sets *OutPtr and returns true on
// success; returns false if the slab is full. Lock-free ( ctzll + CAS clear ).
static inline bool dg_ceph_slab_lieu_try_take( dg_ceph_slab* Slab, dg_u32 Width, void** OutPtr ) {
	for ( dg_u32 Idx = 0; Idx < DG_CEPH_SLAB_ATOMICS; ++Idx ) {
		dg_u64 Bits = atomic_load_explicit( &Slab->FreeBits[Idx], memory_order_relaxed );
		while ( Bits != 0u ) {
			dg_u32 Slot = (dg_u32)__builtin_ctzll( Bits );
			dg_u64 Mask = 1ULL << Slot;
			if ( atomic_compare_exchange_weak_explicit( &Slab->FreeBits[Idx], &Bits, Bits & ~Mask, memory_order_acquire, memory_order_relaxed ) ) {
				dg_u32 AbsoluteSlot = ( Idx * 64u ) + Slot;
				*OutPtr = Slab->Memory + (dg_u64)( AbsoluteSlot * Width );
				return true;
			}
		}
	}
	return false;
}

//
//
// Pool

// Initializes a pool over a CALLER-EMBEDDED first slab. Pass Alloc + GrowSlots for a
// self-growing pool; pass Alloc == null (GrowSlots ignored) for a fixed one. GrowSlots
// is clamped to DG_CEPH_SLOTS_PER_SLAB. The Alloc pointer must outlive the pool.
[[nodiscard]] static inline dg_errcode dg_ceph_pool_proc_init( dg_ceph_pool* Pool, dg_u32 Width, dg_ceph_slab* FirstSlab, dg_u32 SlotTotal, dg_byte* Memory, dg_allocator* Alloc, dg_u32 GrowSlots ) {
	if ( DG_UNLIKELY( Pool == nullptr ) ) {
		return DGE_CEPH_NULL_POOL_PTR;
	}
	for ( dg_errcode E = dg_ceph_slab_suro_init( FirstSlab, SlotTotal, Width, Memory ); !dg_error_ok(E); ) {
		return E;
	}

	Pool->Slabs     = FirstSlab;
	Pool->Width     = Width;
	Pool->Alloc     = Alloc;
	Pool->GrowSlots = ( GrowSlots > DG_CEPH_SLOTS_PER_SLAB ) ? DG_CEPH_SLOTS_PER_SLAB : GrowSlots;
	atomic_store_explicit( &Pool->ActiveSlab, FirstSlab, memory_order_relaxed );
	return DGE_CEPH_NONE;
}

// Appends an already-initialized slab to the tail of the chain (lock-free CAS).
[[nodiscard]] static inline dg_errcode dg_ceph_pool_suro_chain( dg_ceph_pool* Pool, dg_ceph_slab* NewSlab, dg_u32 SlotTotal, dg_byte* Memory ) {
	if ( DG_UNLIKELY( Pool == nullptr ) ) {
		return DGE_CEPH_NULL_POOL_PTR;
	}
	for ( dg_errcode E = dg_ceph_slab_suro_init( NewSlab, SlotTotal, Pool->Width, Memory ); !dg_error_ok(E); ) {
		return E;
	}

	dg_ceph_slab* Tail = Pool->Slabs;
	while ( true ) {
		dg_ceph_slab* Next = atomic_load_explicit( &Tail->Next, memory_order_acquire );
		if ( Next != nullptr ) {
			Tail = Next;
		} else {
			dg_ceph_slab* ExpectedNull = nullptr;
			if ( atomic_compare_exchange_strong_explicit( &Tail->Next, &ExpectedNull, NewSlab, memory_order_release, memory_order_relaxed ) ) {
				break;
			}
		}
	}
	return DGE_CEPH_NONE;
}

// Self-grow: allocate one slab struct + its slot backing from the held allocator,
// then chain it. Only reached for growable pools ( Alloc != null, GrowSlots > 0 ).
// The backing-allocator's own error domain is passed through on failure (by design).
[[nodiscard]] static inline dg_errcode dg_ceph_pool_suro_grow( dg_ceph_pool* Pool ) {
	dg_ceph_slab* Slab = {nullptr};
	for ( dg_errcode E = dg_alloc_zeroed( Pool->Alloc, sizeof(dg_ceph_slab), alignof(dg_ceph_slab), (void**)&Slab ); !dg_error_ok(E); ) {
		return E;
	}
	dg_byte* Backing = {nullptr};
	for ( dg_errcode E = dg_alloc_zeroed( Pool->Alloc, (dg_inbytes)Pool->GrowSlots * Pool->Width, 8u, (void**)&Backing ); !dg_error_ok(E); ) {
		return E;
	}
	for ( dg_errcode E = dg_ceph_pool_suro_chain( Pool, Slab, Pool->GrowSlots, Backing ); !dg_error_ok(E); ) {
		return E;
	}
	// Point the hint at the fresh slab so the alloc retry claims from it in O(1).
	atomic_store_explicit( &Pool->ActiveSlab, Slab, memory_order_release );
	return DGE_CEPH_NONE;
}

// Hands out one slot. Scans every slab; if all are full, grows and retries (growable),
// else returns OUT_OF_MEMORY (fixed). Lock-free — a growth race just over-provisions.
[[nodiscard]] static inline dg_errcode dg_ceph_pool_proc_alloc( dg_ceph_pool* Pool, void** OutPtr ) {
	if ( DG_UNLIKELY( Pool == nullptr ) ) {
		return DGE_CEPH_NULL_POOL_PTR;
	}
	if ( DG_UNLIKELY( OutPtr == nullptr ) ) {
		return DGE_CEPH_NULL_OUT_PTR;
	}

	for ( ;; ) {
		// Fast path: claim from the cached active slab ( O(1) in steady state ).
		dg_ceph_slab* Active = atomic_load_explicit( &Pool->ActiveSlab, memory_order_acquire );
		if ( Active != nullptr && dg_ceph_slab_lieu_try_take( Active, Pool->Width, OutPtr ) ) {
			return DGE_CEPH_NONE;
		}

		// Slow path: sweep the whole chain — catches slots freed in older slabs.
		for ( dg_ceph_slab* Slab = Pool->Slabs; Slab != nullptr; Slab = atomic_load_explicit( &Slab->Next, memory_order_acquire ) ) {
			if ( Slab == Active ) {
				continue;
			}
			if ( dg_ceph_slab_lieu_try_take( Slab, Pool->Width, OutPtr ) ) {
				atomic_store_explicit( &Pool->ActiveSlab, Slab, memory_order_release );
				return DGE_CEPH_NONE;
			}
		}

		// Every slab full.
		if ( Pool->Alloc == nullptr || Pool->GrowSlots == 0u ) {
			return DGE_CEPH_OUT_OF_MEMORY;
		}
		for ( dg_errcode E = dg_ceph_pool_suro_grow( Pool ); !dg_error_ok(E); ) {
			return E;
		}
		// grow succeeded ( ActiveSlab now points at the new slab ) — retry hits the fast path
	}
}

// Frees a slot back to its owning slab by locating which slab a pointer belongs to.
[[nodiscard]] static inline dg_errcode dg_ceph_pool_proc_free_ptr( dg_ceph_pool* Pool, void* Ptr ) {
	if ( DG_UNLIKELY( Pool == nullptr ) ) {
		return DGE_CEPH_NULL_POOL_PTR;
	}
	if ( DG_UNLIKELY( Ptr == nullptr ) ) {
		return DGE_CEPH_NULL_MEMORY_PTR;
	}

	dg_ceph_slab* Slab = Pool->Slabs;
	while ( Slab != nullptr ) {
		dg_u64 SlabBytes = (dg_u64)Slab->SlotTotal * Pool->Width;
		if ( (dg_byte*)Ptr >= Slab->Memory && (dg_byte*)Ptr < Slab->Memory + SlabBytes ) {
			dg_u32 Slot = (dg_u32)( ( (dg_byte*)Ptr - Slab->Memory ) / Pool->Width );
			dg_errcode E = dg_ceph_slab_suro_free_slot( Slab, Slot );
			if ( dg_error_ok(E) ) {
				// This slab just gained a free slot — steer the next alloc back here.
				atomic_store_explicit( &Pool->ActiveSlab, Slab, memory_order_release );
			}
			return E;
		}
		Slab = atomic_load_explicit( &Slab->Next, memory_order_acquire );
	}
	return DGE_CEPH_OUT_OF_BOUNDS; // pointer didn't belong to this pool
}

// Releases every AUTO-GROWN slab (struct + backing) back through the held allocator.
// slab[0] and its backing are caller-owned and left untouched. No-op-safe on arena
// backends (Free does nothing; the arena reset reclaims everything anyway).
static inline void dg_ceph_pool_proc_destroy( dg_ceph_pool* Pool ) {
	if ( Pool == nullptr || Pool->Slabs == nullptr || Pool->Alloc == nullptr ) {
		return;
	}
	dg_ceph_slab* Slab = atomic_load_explicit( &Pool->Slabs->Next, memory_order_acquire );
	while ( Slab != nullptr ) {
		dg_ceph_slab* Next = atomic_load_explicit( &Slab->Next, memory_order_acquire );
		dg_inbytes Backing = (dg_inbytes)Slab->SlotTotal * Pool->Width;
		DGE_ERROR_IGNORE( dg_free( Pool->Alloc, Backing, Slab->Memory ) );
		DGE_ERROR_IGNORE( dg_free( Pool->Alloc, sizeof(dg_ceph_slab), Slab ) );
		Slab = Next;
	}
	atomic_store_explicit( &Pool->Slabs->Next, nullptr, memory_order_release );
}

//
//
// Debug poison (opt-in): overwrite a freed slot with 0xCD in debug builds.

#if !defined(NDEBUG) && defined(DG_CEPH_POISON_FREE)
[[nodiscard]] static inline dg_errcode dg_ceph_pool_dbug_free_ptr( dg_ceph_pool* Pool, void* Ptr ) {
	dg_errcode E = dg_ceph_pool_proc_free_ptr( Pool, Ptr );
	if ( dg_error_ok(E) ) {
		memset( Ptr, 0xCD, Pool->Width );
	}
	return E;
}
#undef  dg_ceph_pool_proc_free_ptr
#define dg_ceph_pool_proc_free_ptr dg_ceph_pool_dbug_free_ptr
#endif

#endif //DG_CEPH_H
