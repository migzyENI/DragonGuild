//dg_rsm.h
// (C) MigzyENT 2026 - DragonGuild Engine

//Runtime Structure Model
#pragma once

#if !defined(__cplusplus) && __STDC_VERSION__ < 202311L
#error "This file requires C23 or later."
#endif

//
//
// Headers

#include <dg_typedef.h>
#include <dg_error.h>
#include <dg_ceph.h>
#include <dg_allocator.h>
#include <dg_hash.h>

//
//
// Errors

constexpr dg_u32 DG_RSM_DWORD_NAME = 0x52534D20; // 'RSM '

DG_ERRCODE_TABLE_GENERATE(
	DGE_RSM, _IDX, _COUNT, DG_RSM_DWORD_NAME,
	OBJECT_NULL,
	REGISTRAR_NULL,
	ALLOC_NULL,
	PICK_A_BIGGER_NUMBER, //Bro
)


//
//
// Base Types

typedef union dg_serial {
	dg_u64 Raw;
	struct {
		dg_u64 Entity : 48;
		dg_u64 DomainSalt : 15;
		dg_u64 IsEntity : 1;
	};
	struct {
		dg_u64 Key : 32;
		dg_u64 LibrarySalt : 31;
		dg_u64 _tag : 1;
	};
} dg_serial;

typedef dg_u64 dg_canon;

typedef union dg_inwords32_ptrbool {
		struct {
			bool IsPointer : 1;
			dg_inwords32 Value : 31;
		};
		dg_u32 Raw;
} dg_inwords32_ptrbool;

typedef union dg_inwords31 {
	struct {
		dg_inwords32 Value : 31;
	};
	dg_u32 Raw;
} dg_inwords31;

///
//
// Object types

typedef struct dg_rsm_subsequor {
	dg_u8 ItemCount;
	dg_u8 HashTableMask;
	dg_inwords16 MemoryLength; //Safety and memalloc check.
	dg_inwords16 CertsOffset;
	dg_inwords16 HashTableOffset;
	dg_inwords16 EarlyQualifierTableOffset;
	dg_inwords16 OffsetArrayOffset; //First member is payload starter
	dg_dword RemovedCertsIndexes;
	dg_canon SelfCert;

} dg_rsm_subsequor;

#ifndef DG_RSM_OFFSET_ARRAY_IS_RAW
#define DG_RSM_OFFSET_ARRAY_IS_RAW true
#endif

typedef struct dg_rsm_object {
	dg_rsm_subsequor Header;
	dg_ptr Memory[];
} dg_rsm_object;

//
//
// Serialization Decls

typedef struct dg_rsm_reference {
	dg_serial Serial;
	dg_canon  Cert;
} dg_rsm_reference;

typedef struct dg_rsm_cert_attrib {
	dg_rsm_reference Ref;
	dg_string  Name;   // dg_string = { char* Char; dg_u64 Length; }
} dg_rsm_cert_attrib;

// Do not concern with this right now. Its like needed but for later development of RSM. the ground work is inplace.
//well auctually there might be good reason to put this in head as an offset because currently its 24bytes
// To be question deeper.
typedef struct dg_rsm_serializationinsert_optional {
	dg_inwords RequestedWidth; //0x47535953  ... [Magic]
	// Object will only contain pointers on self.
	bool ObjectUsesOnlyPtrs; //0xEE01 ...
	bool ObjectFatLoadEverything; //0xEE02 ...
	bool ObjectPtrsAreLoose; //0xEE03 ...
	bool AttributeNameFromDebugReg; //0xDE01 ...

	const dg_rsm_cert_attrib* CertAttribs; //0xCC01 XXXX ... ]
	dg_u16 CertAttribCount; // X <-

	const dg_rsm_reference* IgnoreRefs; //0xCC02 XXXX ... ]
	dg_u16 IgnoreCount; // X <-

	const dg_rsm_reference* IfPointerIgnoreRefs; //0xCC03 XXXX ... ]
	dg_u16 IfPointerIgnoreCount; // X <-

	const dg_rsm_reference* IfPointerAlwaysRetrieveRefs; //0xCC04 XXXX ... ]
	dg_u16 IfPointerAlwaysRetrieveCount; // X <-

	const dg_rsm_reference* AlwaysSaveRefs; //0xCC05 XXXX ... ]
	dg_u16 AlwaysSaveCount; // X <-

} dg_rsm_serializationinsert_optional;

/*
 * For (RSM) Objects their memory is layout as such:
 *
 *  - Header
 *  - Memory [
 *    	Serialization Encode ( Always at 0 but if Cert Array Offset is Zero then itself is removed.)
 *    	Cert Array
 *    	Widths Array
 *    	Hash Table
 *    	Early Qualf BitMasks
 *
 *    ]
 *	The trade off this ECS-like does is footprint for differing ergonomics and serialization being simple.
 *	I'm more supprised by accessor speed which would make sense since no ptr chasing.
 */

//
//
// Higher Level Abstraction Object
//     Only Use if it makes object pattern for accessing better for a library.
//     Full adapter in 1.250.000
/*
typedef struct dg_rsm_conventional_object  {
	dg_u8 RemovedCerts[4];
	dg_canon SelfCert;
	dg_ptr SerializationBytes; //Normal Serialization
	dg_defined_memory CertArray;
	dg_defined_memory HashTable;
	dg_defined_memory EarlyQualMasks;

	dg_defined_memory Payloads[]; // IsPtr, Pointer Arithmatic in dg_ptr size ( machine indepedent ), Length (Original Word Width)

} dg_rsm_conventional_object;
*/
//
//
// Registry Decls

typedef struct dg_rsm_shadowguild_node dg_rsm_shadowguild_node;

typedef _Atomic(dg_rsm_shadowguild_node*) dg_rsm_atomic_shadowguild_node_ptr;

typedef struct dg_rsm_entry {
	dg_canon Cert;
	dg_rsm_atomic_shadowguild_node_ptr Shadow;
	dg_inwords32_ptrbool Width;
	dg_u32 PADDING;
	dg_virtual* VTable; //Frankly this is currently padding, don't use but also not the worst idea.
} dg_rsm_entry;

typedef _Atomic(dg_rsm_entry*) dg_rsm_atomic_entry_ptr;
typedef _Atomic(dg_canon*) dg_rsm_atomic_canon_ptr;

typedef struct dg_rsm_registrar {

	dg_allocator Alloc;

	dg_ceph_pool CertPF;
	dg_ceph_slab BaseCertPF;

	dg_u32 TableCapacity;
	dg_u32 TableMask;

	dg_atomic_u32 OccupiedSlotsCount;
	dg_atomic_u32 LivingSlotsCount;
	dg_atomic_u32 TombstoneCount;
	dg_atomic_u32 AdditionalRequiredCount;

	dg_u32 ShadowMaxObjects;

	dg_rsm_atomic_entry_ptr* CertMap;

	// Libray serial certs.
	// Per Domain Counts.

} dg_rsm_registrar;

typedef struct dg_rsm_registry_accessor {
	const dg_rsm_entry* Pair;
	dg_serial ExpectedSerial;
	dg_canon ExpectedCert;
} dg_rsm_registry_accessor;

//
//
// Qualifier Decls

typedef struct dg_rsm_qualifier {
	const dg_canon* Members;   // member component-certs (object must carry ALL)
	dg_u16 MemberCount;
	dg_u8  Bit;                // 0..63 within the object's qualify word
} dg_rsm_qualifier;

typedef struct dg_rsm_qualifier_set {
	const dg_rsm_qualifier* Items;
	dg_u16 Count;
} dg_rsm_qualifier_set;

typedef struct dg_rsm_shadowguild_node {
	_Atomic(dg_u64)* Bits;        // [WordCount] bitmask; bit i set => object i carries this cert
	const dg_canon* Members;      // qualifier-cert: member component-certs to AND
	dg_u32 WordCount;
	dg_u16 MemberCount;
	bool   IsQualifier;
	dg_u8 PADDING;
} dg_rsm_shadowguild_node;

//
//
// DBs and EQUs

constexpr dg_byte DG_RSM_EMPTY_BUCKET = 0xFFu; // Used for Hash buckets.

constexpr dg_byte DG_RSM_EMPTY_INDEX = 0xFFu; // Used for tombstoning certs on objects.

// Tombstone tag: pool slots are >=8-byte aligned, so the low bit of a stored
// dg_rsm_entry* is free as a logical delete marker.
constexpr dg_ptr DG_RSM_DEAD_BIT = 1u;

//With modest consumer hardware the average is around here.
#ifndef DG_RSM_LINEAR_SCAN_MAX_VALUE
#define DG_RSM_LINEAR_SCAN_MAX_VALUE 8u
#endif
constexpr dg_u16 DG_RSM_LINEAR_SCAN_MAX = DG_RSM_LINEAR_SCAN_MAX_VALUE;
#define DG_RSM_PREFER_LINEAR(ItemCount) ( (dg_u8)(ItemCount) <= DG_RSM_LINEAR_SCAN_MAX )

// 1 word of hidden qualifier bitmask = 64 declared qualifier-certs. Bump if needed.
constexpr dg_inwords16 DG_RSM_QUALIFY_WORDS = 1u;

#define DG_RSM_OFFSET_ACCESS( type , varobj, subsequorfield ) \
( type )( varobj->Memory + varobj->Header.subsequorfield)

//
//
// Registry Helpers

static inline dg_canon dg_rsm_general_lieu_serial_to_cert( dg_serial Serial) {
	return dg_hash_mm3_fmix64(Serial.Raw);
}

static inline dg_serial dg_rsm_general_lieu_cert_to_serial( dg_canon Canon ) {
	return (dg_serial)dg_hash_mm3_fmix64_inverse(Canon);
}

//Was slot considered dead?
static inline bool dg_rsm_registry_helper_lieu_is_disenrolled( dg_rsm_entry* Entry )     {
	return ( (dg_ptr) Entry & DG_RSM_DEAD_BIT ) != 0u;
}

static inline dg_rsm_entry* dg_rsm_registry_lieu_entry_kill( dg_rsm_entry*   Entry )     {
	return (dg_rsm_entry*) ( (dg_ptr) Entry | DG_RSM_DEAD_BIT );
}

static inline dg_rsm_entry* dg_rsm_registry_lieu_entry_resuscitate( dg_rsm_entry*   Entry )     {
	return (dg_rsm_entry*) ( (dg_ptr) Entry & ~ DG_RSM_DEAD_BIT );
}

//
//
// Object Subroutines and Helpers

static inline  dg_canon* dg_rsm_object_suro_ptr_certs ( const dg_rsm_object*   Object )     {
	return DG_RSM_OFFSET_ACCESS(  dg_canon* , Object , CertsOffset );
}

static inline  dg_inwords32_ptrbool* dg_rsm_object_suro_ptr_payload_locations ( const dg_rsm_object*   Object )     {
	return DG_RSM_OFFSET_ACCESS(  dg_inwords32_ptrbool*, Object, OffsetArrayOffset );
}

static inline  dg_byte* dg_rsm_object_suro_ptr_hashtable ( const dg_rsm_object*   Object )     {
	return DG_RSM_OFFSET_ACCESS(  dg_byte*, Object, HashTableOffset );
}

// static inline dg_ptr* dg_rsm_object_access_ptr_payload( dg_rsm_object*   Object )     {
// 	return DG_RSM_OFFSET_ACCESS( dg_ptr* , Object , PayloadOffset );
// }

static inline dg_u64* dg_rsm_object_suro_ptr_earlyqualfier( dg_rsm_object*   Object )     {
	return DG_RSM_OFFSET_ACCESS( dg_u64* , Object , EarlyQualifierTableOffset );
}

static inline bool dg_rsm_object_lieu_index_is_disenrolled( dg_dword Phrase, dg_u8 Slot ) {

	dg_u32 Exchange =  ( Phrase.u32 ) ^ (0x01010101u * (dg_u32) Slot );
	return ( ( Exchange - 0x01010101u ) & ~ Exchange & 0x80808080u) != 0u;

}

//Does not check removed certs.
static inline dg_i16 dg_rsm_object_lieu_retrieve_cert_index( dg_rsm_object*   Object, dg_canon Cert )     {
	if ( Object == nullptr ) {
		return -1;
	}

	dg_u8 Numeric = Object->Header.ItemCount;
	const dg_canon* Certs = dg_rsm_object_suro_ptr_certs( Object );

	if ( DG_RSM_PREFER_LINEAR(Numeric)) {
		for ( dg_u8 idx = 0; idx < Numeric; idx = idx + 1u ) {
			if ( Certs[idx] == Cert ) {
				return (dg_i16)idx;
			}
		}
		return -1;
	} else {
		dg_u32 Mask = (dg_u32)Object->Header.HashTableMask;
		const dg_byte* HashTable = dg_rsm_object_suro_ptr_hashtable(Object);
		DG_HASH_MM3_LINEAR_PROBE_HASH(Cert, Mask, SlotIdx, Attempt_) {
			dg_byte Slot = HashTable[SlotIdx];
			if ( Slot == DG_RSM_EMPTY_BUCKET ) {
				return -1;
			}
			if ( Certs[Slot] == Cert ) {
				return (dg_i16)Slot;
			}
		}
		return -1;
	}
}

// Might want a dg_errcode instead since return is a pointer. Consideration.
static inline dg_ptr* dg_rsm_object_lieu_retrieve_at_index( dg_rsm_object* Object, dg_u8 Index) {
	if ( Object == nullptr ) {
		return nullptr;
	}
	dg_dword Removed = Object->Header.RemovedCertsIndexes;
	if ( Removed.byte[0] != DG_RSM_EMPTY_INDEX && dg_rsm_object_lieu_index_is_disenrolled( Removed , Index )) {
		return nullptr;
	}
	if ( Index >= Object->Header.ItemCount ) {
		return nullptr;
	}

	dg_inwords32_ptrbool Locale = dg_rsm_object_suro_ptr_payload_locations(Object)[Index];
	dg_ptr* Slot = (dg_ptr*)( Object->Memory + DG_AS_BYTES((dg_u64)Locale.Value) );
	return Locale.IsPointer ? (dg_ptr*)*Slot : Slot;
}

static inline dg_ptr* dg_rsm_object_suro_struct_via_cert( dg_rsm_object* Object, dg_canon Cert ) {
	dg_i16 Index = dg_rsm_object_lieu_retrieve_cert_index(Object, Cert);
	if ( Index < 0 ) {
		return nullptr;
	}
	return dg_rsm_object_lieu_retrieve_at_index( Object, (dg_u8)Index );
}

static inline dg_ptr* dg_rsm_object_suro_struct_via_serial( dg_rsm_object* Object, dg_serial Serial ) {
	return dg_rsm_object_suro_struct_via_cert(Object, dg_rsm_general_lieu_serial_to_cert( Serial ));
}

//If optimizing do not use this, just pack simd instructions of the i16 to check for nulls and shit.
static inline bool dg_rsm_object_lieu_contains_cert( dg_rsm_object* Object, dg_canon Cert) {
	return !(dg_rsm_object_lieu_retrieve_cert_index(Object, Cert) < 0);
}

static inline dg_canon dg_rsm_object_lieu_raw_cert_at( dg_rsm_object* Object, dg_u8 Index ) {
	return dg_rsm_object_suro_ptr_certs( Object )[Index];
}

#define DG_RSM_GET( Type, Obj, Cert ) \
	((Type*)dg_rsm_object_access_struct_via_cert((Obj),(Cert)))

static inline dg_rsm_object dg_rsm_object_lieu_compute_simple_offsets( dg_u8 ItemCount, dg_inwords16 CertsOffset ) {

	dg_u16 HashCapacity = 1;
	while (HashCapacity < ItemCount) {
		HashCapacity <<= 1u;
	}

	return (dg_rsm_object) {
		.Header = {
			.ItemCount = ItemCount,
			.HashTableMask = HashCapacity - 1,
			.RemovedCertsIndexes = { .byte = { DG_RSM_EMPTY_INDEX, DG_RSM_EMPTY_INDEX, DG_RSM_EMPTY_INDEX, DG_RSM_EMPTY_INDEX } },
			.CertsOffset = CertsOffset,
			.HashTableOffset = CertsOffset + ItemCount,
			.EarlyQualifierTableOffset = CertsOffset + ItemCount + HashCapacity,
			.OffsetArrayOffset = CertsOffset + ItemCount + HashCapacity + (DG_RSM_QUALIFY_WORDS*sizeof(dg_u64)),
		}
	};
}

static inline dg_inbytes dg_rsm_object_lieu_sum_of_bytes( dg_rsm_object* Object ) {
	return sizeof(dg_rsm_subsequor) + Object->Header.MemoryLength;
}

//
//
// Registry

dg_errcode dg_rsm_registry_suro_initialize( dg_rsm_registrar* OutRegistrar, dg_allocator* Alloc, dg_u32 RequestedCapacity ) {
	if ( OutRegistrar == nullptr ) {
		return DGE_RSM_REGISTRAR_NULL;
	}
	if ( Alloc == nullptr ) {
		return DGE_RSM_ALLOC_NULL;
	}

	dg_u32 AchievedCapacity = DG_PO2_RETURN_32b( RequestedCapacity , dg_u32 );

	// The entry pool self-grows, so the embedded first slab only needs to cover the
	// per-slab cap ( 512 ). Anything past it is chained on demand by dg_ceph_pool_proc_alloc.
	dg_u32 FirstSlots = ( AchievedCapacity < DG_CEPH_SLOTS_PER_SLAB ) ? AchievedCapacity : DG_CEPH_SLOTS_PER_SLAB;

	dg_inbytes PoolBackingSize = (dg_inbytes)FirstSlots      * sizeof(dg_rsm_entry);
	dg_inbytes CertMapSize     = (dg_inbytes)AchievedCapacity * sizeof(dg_rsm_atomic_entry_ptr);

	dg_byte* CertPoolBacking = {nullptr};
	dg_errcode E = dg_alloc_zeroed(Alloc, PoolBackingSize, alignof(dg_rsm_entry), (void**)&CertPoolBacking);
	if ( !dg_error_ok(E) ) {
		return E;
	}

	dg_rsm_atomic_entry_ptr* CertMapBacking = {nullptr};
	E = dg_alloc_zeroed(Alloc, CertMapSize, alignof(dg_rsm_atomic_entry_ptr), (void**)&CertMapBacking);
	if ( !dg_error_ok(E) ) {
		DGE_ERROR_IGNORE( dg_free( Alloc, PoolBackingSize, CertPoolBacking ) );
		return E;
	}

	*OutRegistrar =  (dg_rsm_registrar) {
		.Alloc = *Alloc,
		.TableCapacity = AchievedCapacity,
		.TableMask = AchievedCapacity - 1u, //Given Cap is always 1:1 with mask semantically it clear like this.

		.CertPF = {0},
		.BaseCertPF = {0},

		.OccupiedSlotsCount = 0u,
		.LivingSlotsCount = 0u,
		.TombstoneCount = 0u,
		.AdditionalRequiredCount = 0u,

		.CertMap = CertMapBacking,
	};

	// Growable: grown slabs draw from the registrar's OWN allocator copy ( stable address ),
	// not the caller's Alloc pointer, so the pool is safe once this function returns. -ai
	E = dg_ceph_pool_proc_init( &OutRegistrar->CertPF, sizeof(dg_rsm_entry), &OutRegistrar->BaseCertPF, FirstSlots, CertPoolBacking, &OutRegistrar->Alloc, DG_CEPH_SLOTS_PER_SLAB );
	if ( !dg_error_ok(E) ) {
		DGE_ERROR_IGNORE( dg_free( Alloc, CertMapSize,     (void*)CertMapBacking ) );
		DGE_ERROR_IGNORE( dg_free( Alloc, PoolBackingSize, CertPoolBacking ) );
		return E;
	}

	return DGE_RSM_NONE;
}

dg_errcode dg_rsm_registry_proc_enroll( dg_rsm_registrar* Registrar, dg_serial Serial,  dg_rsm_registry_accessor* OutAccessor );

dg_errcode dg_rsm_registry_proc_disenroll( dg_rsm_registrar* Registrar, dg_serial Serial );

dg_errcode dg_rsm_registry_suro_reacclimatize( dg_rsm_registrar* Registrar );

dg_errcode dg_rsm_registry_suro_grow( dg_rsm_registrar* Registrar );

dg_errcode dg_rsm_registry_proc_maintenance( dg_rsm_registrar* Registrar );

//dg_errcode dg_rsm_registry_process_become_library( dg_rsm_registrar* Registrar, dg_uqword Secert, dg_serial EmptySerial )

//
//
// Objects


//
//
// ShadowGuild

//
//
// DungeonQualifer
