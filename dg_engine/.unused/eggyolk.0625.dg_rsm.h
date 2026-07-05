//dg_rsm.h
// (C) MigzyENT 2026 - DragonGuild Engine

#pragma once

#if !defined(__cplusplus) && __STDC_VERSION__ < 202311L
#error "This file requires C23 or later."
#endif

//
//
// Headers

#include <dg_typedef.h>
#include <dg_error.h>
#include <dg_arena_pool.h>
#include <dg_allocator.h>
#include <dg_hash.h>

//
//
// Errors

constexpr dg_u32 DG_RSM_DWORD_NAME = 0x52534D20; // 'RSM '

DG_ERRCODE_TABLE_GENERATE(
	DGE_RSM, _IDX, _COUNT, DG_RSM_DWORD_NAME,
	RETRY,
	NOOP,
	OBJECT_NULL_PTR,
	REGISTRAR_NULL_PTR,
	ALLOCATOR_NULL_PTR,
	REQUESTED_CAPACITY_IS_ZERO,
	SERIAL_RESERVED,
	SERIAL_ALREADY_REGISTERED,
	REGISTRAR_FAILURE_TO_FEED,
	REGISTRAR_GROWTH_QUEUED
)

//
//
// Structs

typedef dg_u64 dg_serial;
typedef dg_u64 dg_canon;

typedef struct dg_special_inwords {
	union {
		struct {
			bool IsPointer : 1;
			dg_inwords Value : 31;
		};
		dg_u32 Raw;
	};
} dg_special_inwords;

constexpr dg_byte DG_RSM_8b_EMPTY_BUCKET = 0xFFu;

typedef struct dg_subsequor {
	dg_u8 ItemCount;
	dg_u8 HashTableCapacity;
	dg_inwords16 CertsOffset;
	dg_inwords16 SpecialWordsOffset;
	dg_inwords16 HashTableOffset;
	dg_inwords16 PayloadOffset;
	dg_inwords16 EarlyQualifyTableOffset;
	dg_u8 RemovedCerts[4];
	dg_canon Cert;
} dg_subsequor;

typedef struct dg_rsm_ref {
	dg_serial Serial;
	dg_canon  Cert;
} dg_rsm_ref;

typedef struct dg_rsm_cert_attrib {
	dg_rsm_ref Ref;
	dg_string  Name;   // dg_string = { char* Char; dg_u64 Length; }
} dg_rsm_cert_attrib;

typedef struct dg_rsm_serializationinsert_optional {
	dg_inwords RequestedWidth; //0x47535953  ... [Magic]
	// Object will only contain pointers on self.
	bool ObjectUsesOnlyPtrs; //0xEE01 ...
	bool ObjectFatLoadEverything; //0xEE02 ...
	bool ObjectPtrsAreLoose; //0xEE03 ...
	bool AttributeNameFromDebugReg; //0xDE01 ...

	const dg_rsm_cert_attrib* CertAttribs; //0xCC01 XXXX ... ]
	dg_u16 CertAttribCount;

	const dg_rsm_ref* IgnoreRefs; //0xCC02 XXXX ... ]
	dg_u16 IgnoreCount;

	const dg_rsm_ref* IfPointerIgnoreRefs; //0xCC03 XXXX ... ]
	dg_u16 IfPointerIgnoreCount;

	const dg_rsm_ref* IfPointerAlwaysRetrieveRefs; //0xCC04 XXXX ... ]
	dg_u16 IfPointerAlwaysRetrieveCount;

	const dg_rsm_ref* AlwaysSaveRefs; //0xCC05 XXXX ... ]
	dg_u16 AlwaysSaveCount;

} dg_rsm_serializationinsert_optional;

typedef struct dg_object {
	dg_subsequor Header;
	dg_u64 Memory[]; // [ Metadata ] [ Raw ]
} dg_object;

typedef struct dg_reg_shadow_node {
	_Atomic(dg_u64)* Bits;        // [WordCount] bitmask; bit i set => object i carries this cert
	dg_u32 WordCount;
	const dg_canon* Members;      // qualifier-cert: member component-certs to AND
	dg_u16 MemberCount;
	bool   IsQualifier;
} dg_rsm_shadow_node;

typedef struct dg_rsm_certpair {
	dg_inwords StructWidth;
	dg_serial Serial;
	dg_canon Cert;
	_Atomic(dg_rsm_shadow_node*) Shadow;
} dg_rsm_certpair;

typedef _Atomic(dg_rsm_certpair*) dg_atomic_rsm_certpair_ptr;
typedef _Atomic(dg_serial)* dg_atomic_serial_ptr;
typedef _Atomic(dg_canon)* dg_atomic_canon_ptr;

typedef struct dg_registrar {
	dg_allocator Alloc;
	dg_pool_fixed CertPool;
	dg_pool_slab BaseSlab;
	dg_atomic_u32 Count;
	dg_atomic_u32 PhysicalCount; //Tombstones + Reals
	dg_atomic_u32 WarningOrderCount; //Given to registry to grow ahead of a process cycle.
	// The Lock-Free Lookup Tables (Cap. Must be a Power of 2)
	dg_u32 ShadowMaxObjects;
	dg_u32 TableCapacity;
	dg_u32 TableMask;
	dg_u8 TableExponent;
	dg_atomic_rsm_certpair_ptr* MapBySerial;
	dg_atomic_rsm_certpair_ptr* MapByCert;
} dg_registrar;

typedef struct dg_registry_accessor {
	const dg_rsm_certpair* Pair;
	dg_serial ExpectedSerial;
	dg_canon ExpectedCert;
} dg_registry_accessor;

typedef struct dg_rsm_unfinished_object {
	dg_canon ObjectCert;
	dg_u16 Count;
	const dg_rsm_ref* CertRefs;
	const bool* RequestedPtrs;
	const dg_rsm_serializationinsert_optional* Metadata;
} dg_rsm_unfinished_object;
/*
typedef struct dg_rsm_layout {
	dg_u32 Capacity;       // POT bucket count for the embedded hash table
	dg_u32 HashMask;       // Capacity - 1
	dg_inwords16 CertsOff; // word offsets, measured from dg_object.Memory
	dg_inwords16 OffsetsOff;
	dg_inwords16 HashOff;
	dg_inwords16 EqOff;    // hidden early-qualify bitmask
	dg_inwords16 PayloadOff;
	dg_u64 TotalBytes;     // full dg_object allocation size, payload included
} dg_rsm_layout;
*/

typedef struct dg_rsm_qualifier {
	const dg_canon* Members;   // member component-certs (object must carry ALL)
	dg_u16 MemberCount;
	dg_u8  Bit;                // 0..63 within the object's qualify word
} dg_rsm_qualifier;

typedef struct dg_rsm_qualifier_set {
	const dg_rsm_qualifier* Items;
	dg_u16 Count;
} dg_rsm_qualifier_set;

//
//
// Macros

#define DG_RSM_OFFSET_ACCESS( casttype , varobj, subsequorfield ) \
( casttype )( varobj->Memory + varobj->Header.subsequorfield)

constexpr dg_byte DG_RSM_EMPTY_BUCKET = 0xFFu;

#ifndef DG_RSM_LINEAR_SCAN_MAX_VALUE
#define DG_RSM_LINEAR_SCAN_MAX_VALUE 16u
#endif
constexpr dg_u16 DG_RSM_LINEAR_SCAN_MAX = DG_RSM_LINEAR_SCAN_MAX_VALUE;

#define DG_RSM_CERTPAIR_REQ( Reg , Key, Entry, Probe ) \
	dg_rsm_certpair* Entry = atomic_load_explicit( (dg_atomic_rsm_certpair_ptr*)&Reg->MapByCert[Probe], memory_order_acquire ) \


//
//
// Helpers : SCRegistry

static inline dg_canon dg_rsm_registry_helper_serial_to_cert( dg_serial Serial) {
	return (dg_canon)dg_hash_mm3_fmix64(Serial);
}

static inline dg_serial dg_rsm_registry_helper_cert_to_serial( dg_canon Cert) {
	return (dg_serial)dg_hash_mm3_fmix64_inverse( (dg_u64)Cert );
	;
}

static inline dg_u32 dg_rsm_registry_helper_count_open_entries( dg_registrar* Reg ) {
	return Reg->TableCapacity - Reg->PhysicalCount;
}

static inline dg_u32 dg_rsm_registry_helper_count_open_lives( dg_registrar* Reg ) {
	return Reg->TableCapacity - Reg->Count;
}

static inline dg_errcode dg_rsm_registry_helper_double_size ( dg_registrar* Reg ) {
	dg_inbytes Backing = Reg->TableCapacity * sizeof(dg_rsm_certpair);
	dg_inbytes Grow = Backing * 2;
	dg_byte* Block = nullptr;

	dg_errcode E = dg_alloc_zeroed(&Reg->Alloc, Grow, alignof(dg_pool_slab), (void**)&Block);

	if ( !dg_error_ok(E) ) {
		return E; // Bubbles up DGE_ALOC_OUT_OF_MEMORY, DGE_ALOC_NULLCTX, etc.
	}

	E = dg_pool_fixed_chain(&Reg->CertPool, (dg_pool_slab*)Block, Reg->TableCapacity, Block + sizeof(dg_pool_slab));

	if ( !dg_error_ok(E) ) {
		// We allocated the block, but chaining failed. Free the orphaned block to prevent a memory leak!
		dg_release(&Reg->Alloc, Grow, Block);
	}
	return E;
}

static inline dg_errcode dg_rsm_registry_helper_half_size( dg_registrar* Reg ) {
	return DGE_RSM_NOOP;
}


//
//dg_canon* OutPtr
// Functions : SCRegistry
// Note: merge into dg_rsm_impl.c aftwards.

//Start up the show.
[[nodiscard]] static inline dg_errcode dg_rsm_registry_process_init( dg_allocator* Alloc , dg_u8 Exponent , dg_registrar* OutPtr ) {
	if ( DG_UNLIKELY( OutPtr == nullptr || Alloc == nullptr )) {

		if ( OutPtr == nullptr ) {
			return DGE_RSM_REGISTRAR_NULL_PTR;
		}

		if ( Alloc == nullptr ) {
			return DGE_RSM_ALLOCATOR_NULL_PTR;
		}
	}


	 // I see, well, can you make a a table of differences in application please. (also I know the DG_HASH_MM3_LINEAR_PROBE in draft is currently broken becuase it used to be a singular entry point but now refers if Key or Cert.

	//This might need some fine tuning later.
	dg_u32 RequestedCapcity = 1u << Exponent;

	dg_u64 MapBytes  = (dg_u64)RequestedCapcity * sizeof(dg_atomic_rsm_certpair_ptr);
	dg_u64 PoolBytes = (dg_u64)RequestedCapcity * sizeof(dg_rsm_certpair);

	dg_byte* CertPoolMemory = {nullptr};

	dg_errcode E = dg_alloc_zeroed(Alloc, PoolBytes, alignof(dg_rsm_certpair), (void**)&CertPoolMemory);
	if ( !dg_error_ok(E) ) {
		return E;
	}
	dg_atomic_rsm_certpair_ptr* SerialMapMemory = {nullptr};
	E = dg_alloc_zeroed(Alloc, MapBytes, alignof(dg_atomic_rsm_certpair_ptr), (void**)&SerialMapMemory);
	if ( !dg_error_ok(E) ) {
		return E;
	}
	dg_atomic_rsm_certpair_ptr* CertMapMemory = {nullptr};
	E = dg_alloc_zeroed(Alloc, MapBytes, alignof(dg_atomic_rsm_certpair_ptr), (void**)&CertMapMemory);
	if ( !dg_error_ok(E) ) {
		return E;
	}

	*OutPtr = (dg_registrar){
		.Alloc = *Alloc,
		.Count = 0,
		.PhysicalCount = 0,
		.TableCapacity = RequestedCapcity,
		.TableMask = RequestedCapcity - 1u,
		.MapBySerial = SerialMapMemory,
		.MapByCert = CertMapMemory
	};

	return dg_pool_fixed_init(&OutPtr->CertPool, sizeof(dg_rsm_certpair), &OutPtr->BaseSlab, RequestedCapcity, CertPoolMemory);
}

//Put a cert into the DG.
[[nodiscard]] static inline dg_errcode dg_rsm_registry_process_enroll( dg_registrar* Reg, dg_serial SerialSubmited, dg_inwords Width, dg_registry_accessor* OutPtr ) {
	if ( DG_UNLIKELY( Reg == nullptr )) {
		return DGE_RSM_REGISTRAR_NULL_PTR;
	}

	if ( DG_UNLIKELY( SerialSubmited <= 1)) {
		return DGE_RSM_SERIAL_RESERVED;
	}
	dg_canon Cert = dg_rsm_registry_helper_serial_to_cert( SerialSubmited );
	dg_rsm_certpair* Entry = {nullptr};

	DG_HASH_MM3_LINEAR_PROBE_KEY( SerialSubmited, Reg->TableCapacity, Reg->TableMask, SlotIdx, Attempt_) {
		dg_rsm_certpair* AttainedEntry = atomic_load_explicit( &Reg->MapBySerial[SlotIdx],memory_order_acquire );

		if ( DG_LIKELY(AttainedEntry == nullptr) ) {
			dg_rsm_certpair* NewEntry = {nullptr};

			for (dg_errcode E = dg_pool_fixed_alloc(&Reg->CertPool, (void**)&NewEntry); !dg_error_ok(E); ) {

				if ( dg_error_acopy_no_domain( E, DGE_RAII_OUT_OF_MEMORY ) ) {
						atomic_fetch_add_explicit(&Reg->WarningOrderCount, 1u, memory_order_relaxed);
						return DGE_RSM_REGISTRAR_GROWTH_QUEUED;
					}
				return E;
			}

			*NewEntry = (dg_rsm_certpair){
				.Serial = SerialSubmited,
				.Cert = Cert,
				.StructWidth = { .Value = Width }
			};
			dg_rsm_certpair* ExpectedNullEntry = nullptr;

			if( atomic_compare_exchange_strong_explicit( &Reg->MapBySerial[SlotIdx], &ExpectedNullEntry, NewEntry, memory_order_release, memory_order_relaxed )) {
				AttainedEntry = NewEntry;
				atomic_fetch_add_explicit(&Reg->Count, 1u, memory_order_relaxed);
				atomic_fetch_add_explicit(&Reg->PhysicalCount, 1u, memory_order_relaxed);

			} else {
			//Failure on CAS.

				for (dg_errcode E = dg_pool_fixed_free_ptr(&Reg->CertPool, NewEntry); !dg_error_ok(E);) {
					return E;
				}
				AttainedEntry = ExpectedNullEntry;
				Attempt_SlotIdx = Attempt_SlotIdx - 1;
				continue;
			}

		} else if ( AttainedEntry->Serial == SerialSubmited ) {

			return DGE_RSM_SERIAL_ALREADY_REGISTERED;
		}
	}//for serial

	if ( Entry == nullptr ) {
		return DGE_RSM_REGISTRAR_FAILURE_TO_FEED;
	}

	DG_HASH_MM3_LINEAR_PROBE_HASH(Cert, Reg->TableCapacity, Reg->TableMask, SlotIdx, Attempt_) {
		dg_rsm_certpair* AttainedEntry = atomic_load_explicit(&Reg->MapByCert[SlotIdx], memory_order_acquire);

		if ( AttainedEntry == nullptr ) {
			dg_rsm_certpair* ExpectedNullEntry = nullptr;

			if ( DG_LIKELY(atomic_compare_exchange_strong_explicit( &Reg->MapByCert[SlotIdx], &ExpectedNullEntry, Entry, memory_order_release, memory_order_relaxed ))) {
				break;

			} else {
				AttainedEntry = ExpectedNullEntry;
				Attempt_SlotIdx = Attempt_SlotIdx - 1;
			}

		} else if (AttainedEntry->Cert == Cert ){
			break;
		}
	}
	//null check already done.
	*OutPtr = (dg_registry_accessor){
			.Pair = Entry,
			.ExpectedSerial = SerialSubmited,
			.ExpectedCert = Cert
	};

	return DGE_RSM_NONE;
}

[[nodiscard]] static inline dg_errcode dg_rsm_registry_process_disenroll(dg_registrar* Reg, dg_canon MerderestCert) {
	if ( Reg == nullptr ) {
		return DGE_RSM_REGISTRAR_NULL_PTR;
	}

	DG_HASH_MM3_LINEAR_PROBE_HASH(MerderestCert, Reg->TableCapacity, Reg->TableMask, SlotIdx, Attempt_) {
		DG_RSM_CERTPAIR_REQ( Reg , MerderestCert, Entry, SlotIdx );
		if (Entry == nullptr ) {
			return DGE_RSM_NOOP;
		}
		if ( Entry->Tombstone ) {
			continue;
		}


	}

	return DGE_RSM_NONE;
};

[[nodiscard]] static inline dg_errcode dg_rsm_registry_process_graveduty(dg_registrar* Reg);


[[nodiscard]] static inline dg_errcode dg_rsm_registry_process_warningorders();
//EOF
