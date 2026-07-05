//dg_arena_pool.h
// (C) MigzyENTI
//
#ifndef DG_ARENA_POOL_H
#define DG_ARENA_POOL_H
//
#include <stdatomic.h>
#pragma once
//
#if !defined(__cplusplus) && __STDC_VERSION__ < 202311L
#error "This file requires C23 or later."
#endif

#include <dg_typedef.h>
#include <dg_error.h>
#include <dg_raii.h>


constexpr dg_u32 DG_POOL_SIZE_VARIANTS_COUNT = 7u;
constexpr dg_u32 DG_POOL_SIZE_VARIANTS[ DG_POOL_SIZE_VARIANTS_COUNT ] = {
	8u,
	16u,
	32u,
	64u,
	128u,
	256u,
	512u
};

#ifndef DG_POOL_SLAB_64bitATOMICS_PER_SLAB
#define DG_POOL_SLAB_64bitATOMICS_PER_SLAB 8u
#endif

typedef struct dg_pool_slab {
	dg_u32         SlotTotal;
	dg_u32         SlotWidth;
	dg_atomic_u64  FreeBits[DG_POOL_SLAB_64bitATOMICS_PER_SLAB];   // 512 slots max (8 x 64)
	dg_byte*       Memory;
	_Atomic(struct dg_pool_slab*) Next; //Atomic for next safety.
} dg_pool_slab;

typedef struct dg_pool {
	dg_pool_slab* Slabs[ DG_POOL_SIZE_VARIANTS_COUNT ];
} dg_pool;

[[nodiscard]] static inline dg_errcode_64 dg_pool_alloc_no_size_check_( dg_pool* Pool, dg_u32 Size, void** OutPtr ) {
	if ( DG_UNLIKELY( Pool   == nullptr ) ) {
		return DGE_RAII_NULL_POOL_PTR;
	}
	if ( DG_UNLIKELY( OutPtr == nullptr ) ) {
		return DGE_RAII_NULL_OUT_PTR;
	}
	dg_u32 VariantIndex = ( Size <= 8u )
	? 0u
	: ( 32u - (dg_u32)__builtin_clz( (dg_u32)(Size - 1u) ) ) - 3u;
	if ( VariantIndex >= DG_POOL_SIZE_VARIANTS_COUNT ) {
		VariantIndex = DG_POOL_SIZE_VARIANTS_COUNT - 1u;
	}
	dg_u32 Width = DG_POOL_SIZE_VARIANTS[ VariantIndex ];

	dg_pool_slab* Slab = Pool->Slabs[ VariantIndex ];
	while ( DG_LIKELY( Slab != nullptr ) ) {
		for ( dg_u32 i = 0; i < DG_POOL_SLAB_64bitATOMICS_PER_SLAB; i = i + 1 ) {
			dg_u64 Bits = atomic_load_explicit( &Slab->FreeBits[i], memory_order_relaxed );
			while ( DG_LIKELY( Bits != 0u ) ) {
				dg_u32 Slot = (dg_u32)__builtin_ctzll( Bits );
				dg_u64 Mask = 1ULL << Slot;
				if (
					atomic_compare_exchange_weak_explicit(
						&Slab->FreeBits[i],
						&Bits,
						Bits & ~Mask,
						memory_order_acquire,
						memory_order_relaxed
					)
				) {
					dg_u32 AbsoluteSlot = ( i * 64u ) + Slot;
					*OutPtr = Slab->Memory + (dg_u64)( AbsoluteSlot * Width );
					return DGE_RAII_NONE;
				}
			}
		}
		Slab = Slab->Next;
	}
	return DGE_RAII_OUT_OF_MEMORY;
}

[[gnu::error(
	"dg_pool_alloc_no_size_check must not be called directly. "
	"Use dg_pool_alloc() for runtime sizes, or DG_POOL_CT_ALLOC_RAII() for compile-time-known types."
)]]
static dg_errcode_64 dg_pool_alloc_no_size_check( dg_pool* Pool, dg_u32 Size, void** OutPtr );

[[nodiscard]] static inline dg_errcode_64 dg_pool_alloc( dg_pool* Pool, dg_u32 Size, void** OutPtr ) {
	if ( DG_UNLIKELY( Size > DG_POOL_SIZE_VARIANTS[ DG_POOL_SIZE_VARIANTS_COUNT - 1u ] ) ) {
		return DGE_RAII_SIZE_OVERFLOW;
	}
	return dg_pool_alloc_no_size_check_( Pool, Size, OutPtr );
}

// Compile-time size guard — routes to unchecked alloc path.
#define DG_POOL_ALLOC_COMPT( Pool, OutPtr, Type ) \
	( (void)sizeof(struct { \
		_Static_assert( \
			sizeof(Type) <= 512u, \
			"DG_POOL_CT_ALLOC_RAII: Type exceeds maximum pool slot size of 512 bytes." \
		); \
		int _; \
	}), \
dg_pool_alloc_no_size_check_( (Pool), (dg_u32)sizeof(Type), (void**)(OutPtr) ) )

[[nodiscard]] static inline dg_errcode_64 dg_pool_free( dg_pool_slab* Slab, dg_u32 Slot ) {
	if ( DG_UNLIKELY( Slab == nullptr ) ) {
		return DGE_RAII_NULL_SLAB_PTR;
	}
	if ( DG_UNLIKELY( Slot >= Slab->SlotTotal ) ) {
		return DGE_RAII_OUT_OF_BOUNDS;
	}
	dg_u32 WordIdx = Slot / 64u;
	dg_u32 BitIdx  = Slot % 64u;
	dg_u64 Mask    = 1ULL << BitIdx;
	dg_u64 Prev    = atomic_fetch_or_explicit(
		&Slab->FreeBits[WordIdx],
		Mask,
		memory_order_release
	);
	if ( DG_UNLIKELY( Prev & Mask ) ) {
		return DGE_RAII_DOUBLE_FREE;
	}
	return DGE_RAII_NONE;
}

#if !defined(NDEBUG) && defined(DG_POOL_POISON_FREE)
[[nodiscard]] static inline dg_errcode_64 dg_pool_free_poisoned_( dg_pool_slab* Slab, dg_u32 Slot ) {
	dg_errcode_64 E = dg_pool_free( Slab, Slot );
	if ( dg_error_ok(E) ) {
		memset( Slab->Memory + (dg_u64)( Slot * Slab->SlotWidth ), 0xCD, Slab->SlotWidth );
	}
	return E;
}
#undef  dg_pool_free
#define dg_pool_free dg_pool_free_poisoned_
#endif

[[nodiscard]] static inline dg_errcode_64 dg_pool_slab_validate( const dg_pool_slab* Slab, dg_u32 Slot ) {
	if ( DG_UNLIKELY( Slab == nullptr ) ) {
		return DGE_RAII_NULL_SLAB_PTR;
	}
	if ( DG_UNLIKELY( Slab->Memory == nullptr ) ) {
		return DGE_RAII_NULL_MEMORY_PTR;
	}
	if ( DG_UNLIKELY( Slab->SlotWidth == 0u ) ) {
		return DGE_RAII_SIZE_UNINITIALIZED;
	}
	if ( DG_UNLIKELY( Slab->SlotTotal == 0u ) ) {
		return DGE_RAII_SIZE_UNINITIALIZED;
	}
	if ( DG_UNLIKELY( Slab->SlotTotal > DG_POOL_SLAB_64bitATOMICS_PER_SLAB * 64u ) ) {
		return DGE_RAII_SIZE_OVERFLOW;
	}
	if ( DG_UNLIKELY( Slot >= Slab->SlotTotal ) ) {
		return DGE_RAII_OUT_OF_BOUNDS;
	}
	return DGE_RAII_NONE;
}

[[nodiscard]] static inline dg_errcode_64 dg_pool_slab_init( dg_pool_slab* Slab, dg_u32 SlotTotal, dg_u32 SlotWidth, dg_byte* Memory ) {
	if ( DG_UNLIKELY( Slab == nullptr ) ) {
		return DGE_RAII_NULL_SLAB_PTR;
	}
	if ( DG_UNLIKELY( Memory == nullptr ) ) {
		return DGE_RAII_NULL_MEMORY_PTR;
	}
	if ( DG_UNLIKELY( SlotTotal == 0u ) ) {
		return DGE_RAII_SIZE_UNINITIALIZED;
	}
	if ( DG_UNLIKELY( SlotTotal > DG_POOL_SLAB_64bitATOMICS_PER_SLAB * 64u ) ) {
		return DGE_RAII_SIZE_OVERFLOW;
	}
	Slab->SlotTotal = SlotTotal;
	Slab->SlotWidth = SlotWidth;
	Slab->Memory    = Memory;
	Slab->Next      = nullptr;
	dg_u32 FullWords = SlotTotal / 64u;
	dg_u32 Remainder = SlotTotal % 64u;
	for ( dg_u32 i = 0; i < DG_POOL_SLAB_64bitATOMICS_PER_SLAB; ++i ) {
		dg_u64 Mask;
		if ( i < FullWords ) {
			Mask = ~0ULL;
		} else if (
			i == FullWords &&
			Remainder > 0u
		) {
			Mask = ( 1ULL << Remainder ) - 1ULL;
		} else {
			Mask = 0ULL;
		}
		atomic_store_explicit(
			&Slab->FreeBits[i],
			Mask,
			memory_order_relaxed
		);
	}
	return DGE_RAII_NONE;
}

//
// Fixed Variant Pool
/////

typedef struct dg_pool_fixed {
	dg_pool_slab* Slabs;
	dg_u32 Width;
} dg_pool_fixed;

[[nodiscard]] static inline dg_errcode_64 dg_pool_fixed_init( dg_pool_fixed* Pool, dg_u32 Width, dg_pool_slab* FirstSlab, dg_u32 SlotTotal, dg_byte* Memory ) {
	if ( DG_UNLIKELY( Pool == nullptr ) ) {
		return DGE_RAII_NULL_POOL_FIXED_PTR;
	}

	for ( dg_errcode E = dg_pool_slab_init( FirstSlab, SlotTotal, Width, Memory); E.Err; ) {
		return E;
	}

	Pool->Slabs = FirstSlab;
	Pool->Width = Width;
	return DGE_RAII_NONE;
}

[[nodiscard]] static inline dg_errcode_64 dg_pool_fixed_alloc( dg_pool_fixed* Pool, void** OutPtr) {
	if ( DG_UNLIKELY( Pool == nullptr ) ) {
		return DGE_RAII_NULL_POOL_FIXED_PTR;
	}
	if (DG_UNLIKELY(OutPtr == nullptr) ) {
		return DGE_RAII_NULL_OUT_PTR;
	}

	//We get the first slab
	dg_pool_slab* Slab = Pool->Slabs;

	while ( DG_LIKELY(Slab != nullptr) ) {
		for (dg_u32 i = 0; i < DG_POOL_SLAB_64bitATOMICS_PER_SLAB; ++i) {
			dg_u64 Bits = atomic_load_explicit(&Slab->FreeBits[i], memory_order_relaxed);
			while ( DG_LIKELY(Bits != 0u) ) {
				dg_u32 Slot = (dg_u32)__builtin_ctzll(Bits);
				dg_u64 Mask = 1ULL << Slot;
				if (atomic_compare_exchange_weak_explicit(&Slab->FreeBits[i], &Bits, Bits & ~Mask, memory_order_acquire, memory_order_relaxed)) {
					// Total absolute slot index is (i * 64) + Slot
					dg_u32 AbsoluteSlot = (i * 64u) + Slot;
					*OutPtr = Slab->Memory + (dg_u64)(AbsoluteSlot * Pool->Width);
					return DGE_RAII_NONE;
				}
			}
		}
		Slab = atomic_load_explicit(&Slab->Next, memory_order_acquire);
	}
	return DGE_RAII_OUT_OF_MEMORY;
}

#define DG_POOL_FIXED_ALLOC_COMPT(Pool, OutPtr, Type) \
( (void)sizeof(struct { \
	_Static_assert( \
	sizeof(Type) <= 512u, \
	"DG_POOL_FIXED_ALLOC_COMPT: Type exceeds 512 bytes" \
	); \
	int _; \
}), \
dg_pool_fixed_alloc((Pool), (void**)(OutPtr)) )

[[nodiscard]] static inline dg_errcode_64 dg_pool_fixed_chain(
	dg_pool_fixed* Pool,
	dg_pool_slab*  NewSlab,
	dg_u32         SlotTotal,
	dg_byte*       Memory
) {
	if (DG_UNLIKELY(Pool == nullptr)) {
		 return DGE_RAII_NULL_POOL_FIXED_PTR;
	}

	for ( dg_errcode E = dg_pool_slab_init(NewSlab, SlotTotal, Pool->Width, Memory); E.Err;) {
		return E;
	}

	// walk to tail and append
	dg_pool_slab* Tail = Pool->Slabs;

	while (true) {
		dg_pool_slab* NextSlab = atomic_load_explicit(&Tail->Next, memory_order_acquire);

		if (NextSlab != nullptr) {
			Tail = NextSlab; // Keep walking
		} else {
			dg_pool_slab* ExpectedNull = nullptr;
			if (atomic_compare_exchange_strong_explicit(
				&Tail->Next, &ExpectedNull, NewSlab,
				memory_order_release, memory_order_relaxed)
			) {
				break; // Successfully appended!
			}
		}
	}
	return DGE_RAII_NONE;
}

[[nodiscard]] static inline dg_errcode_64 dg_pool_fixed_free_ptr(dg_pool_fixed* Pool, void* Ptr) {
	if (DG_UNLIKELY(Pool == nullptr)) {
		return DGE_RAII_NULL_POOL_FIXED_PTR;
	}
	if (DG_UNLIKELY(Ptr == nullptr)) {
		return DGE_RAII_NULL_MEMORY_PTR; // Correct explicit error!
	}

	dg_pool_slab* Slab = Pool->Slabs;
	while (Slab) {
		dg_u64 SlabBytes = (dg_u64)Slab->SlotTotal * Pool->Width;
		if (
			(dg_byte*)Ptr >= Slab->Memory &&
			(dg_byte*)Ptr < Slab->Memory + SlabBytes
		) {
			dg_u32 Slot = (dg_u32)(((dg_byte*)Ptr - Slab->Memory) / Pool->Width);
			return dg_pool_free(Slab, Slot);
		}
		Slab = Slab->Next;
	}
	return DGE_RAII_OUT_OF_BOUNDS; // Pointer didn't belong to this pool
}

#endif //DG_ARENA_POOL_H
