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
	NOT_VALID,
	CERTS_NULL,
	SIZE_IS_ZERO,
	WIDTH_HAS_31b_OVERFLOW,
	ITEM_COUNT_EXCEEDS_255,
	CERT_IS_NOT_CURRENTLY_CONTEXTED,
	ALREADY_ENROLLED_DIFFERENTLY,
	GROWTH_QUEUED,
	REMOVED_CERTS_FULL,
	NOT_A_POINTER_SLOT,
	SHADOW_UNINITIALIZED,
	OBJECT_SIZE_OVERFLOW, //Creation: recalculate widths and retry — soft stop.
	OBJECT_SIZE_IMPOSSIBLE_MUTATION, //Mutation: even after dropping removed certs it cannot fit — hard stop.
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
	dg_u32 ShadowIndex; //Dense guild index; DG_RSM_SHADOW_NO_INDEX when untracked.
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
typedef _Atomic(dg_canon)* dg_rsm_atomic_canon_ptr; //Pointer TO an atomic canon: for lock-free field reads.

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

	_Atomic(dg_u64)* ShadowIndexMask; //Implicit dense-index allocator ( ceph-style bitmask ).

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

//Object not tracked by the ShadowGuild ( guild inactive at create, or cleaned ).
constexpr dg_u32 DG_RSM_SHADOW_NO_INDEX = 0xFFFFFFFFu;

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

// Lock-free probe of the ONE canon map. Slot states: null = EMPTY ( probe stops, the
// cert was never here ), live pointer, or dead-bit-tagged TOMBSTONE ( skip, keep
// probing — tombstoning preserves probe-chain integrity where nulling would cut it ).
[[nodiscard]] static inline const dg_rsm_entry* dg_rsm_registry_suro_lookup_via_cert( const dg_rsm_registrar* Registrar, dg_canon Cert ) {
	if ( DG_UNLIKELY( Registrar == nullptr ) ) {
		return nullptr;
	}
	DG_HASH_MM3_LINEAR_PROBE_HASH( Cert, Registrar->TableMask, SlotIdx, _Attempt_ ) {
		dg_rsm_entry* Found = atomic_load_explicit( &Registrar->CertMap[SlotIdx], memory_order_acquire );
		if ( Found == nullptr ) {
			return nullptr;
		}
		if ( dg_rsm_registry_helper_lieu_is_disenrolled( Found ) ) {
			continue;
		}
		if ( Found->Cert == Cert ) {
			return Found;
		}
	}
	return nullptr;
}

// Serial resolves through the bijection — no second map to keep coherent.
[[nodiscard]] static inline const dg_rsm_entry* dg_rsm_registry_suro_lookup_via_serial( const dg_rsm_registrar* Registrar, dg_serial Serial ) {
	return dg_rsm_registry_suro_lookup_via_cert( Registrar, dg_rsm_general_lieu_serial_to_cert( Serial ) );
}

// Re-check that an accessor's entry still carries the cert it was issued for
// ( detects pool-slot reuse after a disenroll + reacclimatize ).
[[nodiscard]] static inline bool dg_rsm_registry_lieu_accessor_validity( const dg_rsm_registry_accessor* Accessor ) {
	if ( DG_UNLIKELY( Accessor == nullptr || Accessor->Pair == nullptr ) ) {
		return false;
	}
	return atomic_load_explicit( (dg_rsm_atomic_canon_ptr)&Accessor->Pair->Cert, memory_order_acquire ) == Accessor->ExpectedCert;
}

// Non-atomic probe-insert into a fresh map being built. Single writer, no contention:
// reacclimatize/grow only, at a SAFE POINT.
static inline void dg_rsm_registry_lieu_rebuild_insert( dg_rsm_atomic_entry_ptr* Map, dg_u32 Mask, dg_rsm_entry* Entry ) {
	DG_HASH_MM3_LINEAR_PROBE_HASH( Entry->Cert, Mask, SlotIdx, _Attempt_ ) {
		if ( atomic_load_explicit( &Map[SlotIdx], memory_order_relaxed ) == nullptr ) {
			atomic_store_explicit( &Map[SlotIdx], Entry, memory_order_relaxed );
			return;
		}
	}
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

//Is Slot's index one of the four removal-ledger bytes? SWAR membership via dg_hash.
static inline bool dg_rsm_object_lieu_index_is_disenrolled( dg_dword Phrase, dg_u8 Slot ) {
	return dg_hash_swar_has_byte32( Phrase.u32, (dg_byte)Slot );
}

//Does not check removed certs.
static inline dg_i16 dg_rsm_object_lieu_retrieve_cert_index( dg_rsm_object*   Object, dg_canon Cert )     {
	if ( Object == nullptr ) {
		return -1;
	}

	dg_u8 Numeric = Object->Header.ItemCount;
	const dg_canon* Certs = dg_rsm_object_suro_ptr_certs( Object );

	if ( DG_RSM_PREFER_LINEAR(Numeric)) {
		for ( dg_u8 Idx = 0; Idx < Numeric; ++Idx ) {
			if ( Certs[Idx] == Cert ) {
				return (dg_i16)Idx;
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

	// Memory is dg_ptr[]: pointer arithmetic is already in words. Value is a raw
	// word offset from Memory base ( DG_RSM_OFFSET_ARRAY_IS_RAW ).
	dg_inwords32_ptrbool Locale = dg_rsm_object_suro_ptr_payload_locations(Object)[Index];
	dg_ptr* Slot = (dg_ptr*)( Object->Memory + Locale.Value );
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
	((Type*)dg_rsm_object_suro_struct_via_cert((Obj),(Cert)))

// All offsets are in WORDS from Memory[0] ( Memory is dg_ptr[], arithmetic is already
// word-wide ). Certs are 1 word each; the hash table is HashCapacity BYTES; the offset
// array is 4 bytes per item PLUS ONE FAKE TAIL ENTRY ( ItemCount + 1 entries: the tail
// is pinned to MemoryLength so physical width is branchless offset[i+1] - offset[i] ).
// Payload begins right after the offset array — offset[0].Value of a fresh object
// equals OffsetArrayOffset + its own word span.
// MemoryLength == 0 in the returned proto means THE LAYOUT DOES NOT FIT dg_inwords16 —
// callers map that to their own error ( a real layout is never 0: overhead > 0 ).
static inline dg_rsm_object dg_rsm_object_lieu_compute_simple_offsets( dg_u8 ItemCount, dg_inwords16 CertsOffset, dg_inwords PayloadWords ) {

	dg_u16 HashCapacity = 1;
	while ( HashCapacity < ItemCount ) {
		HashCapacity <<= 1u;
	}
	// Load-factor headroom: probe chains rot in a near-full table. 255 items already
	// forces capacity 256 ( mask 255 caps out the dg_u8 field ), so only pad below it.
	if ( HashCapacity < 256u && ItemCount * 4u > HashCapacity * 3u ) {
		HashCapacity <<= 1u;
	}

	dg_inwords16 HashWords         = (dg_inwords16)DG_AS_WORDS( HashCapacity );
	dg_inwords16 OffsetArrayWords  = (dg_inwords16)DG_AS_WORDS( ( (dg_u16)ItemCount + 1u ) * sizeof(dg_inwords32_ptrbool) );
	dg_inwords16 OffsetArrayOffset = (dg_inwords16)( CertsOffset + ItemCount + HashWords + DG_RSM_QUALIFY_WORDS );

	dg_inwords64 TotalWords = (dg_inwords64)OffsetArrayOffset + OffsetArrayWords + PayloadWords;
	if ( TotalWords > 0xFFFFu ) {
		TotalWords = 0u;
	}

	return (dg_rsm_object) {
		.Header = {
			.ItemCount = ItemCount,
			.HashTableMask = (dg_u8)( HashCapacity - 1u ),
			.MemoryLength = (dg_inwords16)TotalWords,
			.ShadowIndex = DG_RSM_SHADOW_NO_INDEX,
			.RemovedCertsIndexes = { .byte = { DG_RSM_EMPTY_INDEX, DG_RSM_EMPTY_INDEX, DG_RSM_EMPTY_INDEX, DG_RSM_EMPTY_INDEX } },
			.CertsOffset = CertsOffset,
			.HashTableOffset = (dg_inwords16)( CertsOffset + ItemCount ),
			.EarlyQualifierTableOffset = (dg_inwords16)( CertsOffset + ItemCount + HashWords ),
			.OffsetArrayOffset = OffsetArrayOffset,
		}
	};
}

static inline dg_inbytes dg_rsm_object_lieu_sum_of_bytes( dg_rsm_object* Object ) {
	return sizeof(dg_rsm_subsequor) + DG_AS_BYTES( (dg_inbytes)Object->Header.MemoryLength );
}

//Presence check that RESPECTS removal tombstones — the qualify/serialize truth.
[[nodiscard]] static inline bool dg_rsm_object_lieu_carries_cert( dg_rsm_object* Object, dg_canon Cert ) {
	dg_i16 Index = dg_rsm_object_lieu_retrieve_cert_index( Object, Cert );
	if ( Index < 0 ) {
		return false;
	}
	dg_dword Removed = Object->Header.RemovedCertsIndexes;
	if ( Removed.byte[0] != DG_RSM_EMPTY_INDEX && dg_rsm_object_lieu_index_is_disenrolled( Removed, (dg_u8)Index ) ) {
		return false;
	}
	return true;
}

//
//
// ShadowGuild
//
// Reverse-qualify over the ONE registrar: "which live objects carry cert X", one bit
// per dense object index, hanging lazily off dg_rsm_entry.Shadow. submit/clear/qualify
// are lock-free any-thread; drain/fini are SINGLE WRITER at a safe point.
// IMPLICIT MODE: once dg_rsm_shadow_suro_init has run, object create / remove / readd /
// mutate / cleanfn mirror themselves into the guild on their own — the registrar hands
// out dense indexes ( ceph-style atomic bitmask ) and the object carries its index in
// the header. submit/clear stay public for manual control, like grow/reacclimatize do.
// Qualifier masks refresh inside dg_rsm_registry_proc_maintenance: the autonomous tick.

static inline dg_u32 dg_rsm_shadow_lieu_words( dg_u32 MaxObjects ) {
	return ( MaxObjects + 63u ) >> 6u;
}

// Arm the guild ONCE, before objects are born: allocates the dense-index words. Tail
// bits past MaxObjects are pre-claimed so alloc can never hand out an invalid index.
[[nodiscard]] static inline dg_errcode dg_rsm_shadow_suro_init( dg_rsm_registrar* Registrar, dg_u32 MaxObjects ) {
	if ( DG_UNLIKELY( Registrar == nullptr ) ) {
		return DGE_RSM_REGISTRAR_NULL;
	}
	if ( DG_UNLIKELY( MaxObjects == 0u ) ) {
		return DGE_RSM_SIZE_IS_ZERO;
	}
	if ( DG_UNLIKELY( Registrar->ShadowIndexMask != nullptr ) ) {
		return DGE_RSM_NOT_VALID; //Armed already.
	}
	dg_u32 Words = dg_rsm_shadow_lieu_words( MaxObjects );
	dg_u64* Mask = {nullptr};
	for ( dg_errcode E = dg_alloc_zeroed( &Registrar->Alloc, (dg_inbytes)Words * sizeof(dg_u64), alignof(dg_u64), (void**)&Mask ); !dg_error_ok(E); ) {
		return E;
	}
	dg_u32 Remainder = MaxObjects & 63u;
	if ( Remainder != 0u ) {
		Mask[Words - 1u] = ~(dg_u64)0u << Remainder;
	}
	Registrar->ShadowIndexMask = (_Atomic(dg_u64)*)Mask;
	Registrar->ShadowMaxObjects = MaxObjects;
	return DGE_RSM_NONE;
}

// Claim the lowest free dense index, lock-free. DG_RSM_SHADOW_NO_INDEX means the guild
// is full ( or unarmed ): pick a bigger MaxObjects.
[[nodiscard]] static inline dg_u32 dg_rsm_shadow_lieu_index_alloc( dg_rsm_registrar* Registrar ) {
	if ( Registrar->ShadowIndexMask == nullptr ) {
		return DG_RSM_SHADOW_NO_INDEX;
	}
	dg_u32 Words = dg_rsm_shadow_lieu_words( Registrar->ShadowMaxObjects );
	for ( dg_u32 Wx = 0; Wx < Words; ++Wx ) {
		dg_u64 Current = atomic_load_explicit( &Registrar->ShadowIndexMask[Wx], memory_order_relaxed );
		while ( Current != ~(dg_u64)0u ) {
			dg_u32 Bit = (dg_u32)__builtin_ctzll( ~Current );
			dg_u64 Claimed = Current | ( (dg_u64)1u << Bit );
			if ( atomic_compare_exchange_weak_explicit( &Registrar->ShadowIndexMask[Wx], &Current, Claimed, memory_order_acq_rel, memory_order_relaxed ) ) {
				return ( Wx << 6u ) + Bit;
			}
		}
	}
	return DG_RSM_SHADOW_NO_INDEX;
}

static inline void dg_rsm_shadow_lieu_index_free( dg_rsm_registrar* Registrar, dg_u32 Index ) {
	if ( Registrar->ShadowIndexMask == nullptr || Index >= Registrar->ShadowMaxObjects ) {
		return;
	}
	atomic_fetch_and_explicit( &Registrar->ShadowIndexMask[Index >> 6u], ~( (dg_u64)1u << ( Index & 63u ) ), memory_order_release );
}

// Get ( or lazily create + CAS-publish ) the bitmask node for an entry. The lost
// racer frees its node and adopts the winner's — last-write never leaks.
static inline dg_rsm_shadowguild_node* dg_rsm_shadow_lieu_node( dg_rsm_registrar* Registrar, dg_rsm_entry* Entry, bool Create ) {
	dg_rsm_shadowguild_node* Node = atomic_load_explicit( &Entry->Shadow, memory_order_acquire );
	if ( Node != nullptr || !Create ) {
		return Node;
	}
	dg_u32 Words = dg_rsm_shadow_lieu_words( Registrar->ShadowMaxObjects );
	dg_rsm_shadowguild_node* Fresh = {nullptr};
	if ( !dg_error_ok( dg_alloc_zeroed( &Registrar->Alloc, sizeof(dg_rsm_shadowguild_node), alignof(dg_rsm_shadowguild_node), (void**)&Fresh ) ) ) {
		return nullptr;
	}
	dg_u64* Bits = {nullptr};
	if ( !dg_error_ok( dg_alloc_zeroed( &Registrar->Alloc, (dg_inbytes)Words * sizeof(dg_u64), alignof(dg_u64), (void**)&Bits ) ) ) {
		DGE_ERROR_IGNORE( dg_free( &Registrar->Alloc, sizeof(dg_rsm_shadowguild_node), Fresh ) );
		return nullptr;
	}
	Fresh->Bits = (_Atomic(dg_u64)*)Bits;
	Fresh->WordCount = Words;

	dg_rsm_shadowguild_node* Expected = {nullptr};
	if ( atomic_compare_exchange_strong_explicit( &Entry->Shadow, &Expected, Fresh, memory_order_release, memory_order_relaxed ) ) {
		return Fresh;
	}
	DGE_ERROR_IGNORE( dg_free( &Registrar->Alloc, (dg_inbytes)Words * sizeof(dg_u64), Bits ) );
	DGE_ERROR_IGNORE( dg_free( &Registrar->Alloc, sizeof(dg_rsm_shadowguild_node), Fresh ) );
	return Expected;
}

//Object ObjectIndex now carries ComponentCert. Lock-free atomic OR, no alloc steady-state.
[[nodiscard]] static inline dg_errcode dg_rsm_shadow_proc_submit( dg_rsm_registrar* Registrar, dg_canon ComponentCert, dg_u32 ObjectIndex ) {
	if ( DG_UNLIKELY( Registrar == nullptr ) ) {
		return DGE_RSM_REGISTRAR_NULL;
	}
	if ( DG_UNLIKELY( Registrar->ShadowMaxObjects == 0u ) ) {
		return DGE_RSM_SHADOW_UNINITIALIZED;
	}
	if ( DG_UNLIKELY( ObjectIndex >= Registrar->ShadowMaxObjects ) ) {
		return DGE_RSM_NOT_VALID;
	}
	dg_rsm_entry* Entry = (dg_rsm_entry*)dg_rsm_registry_suro_lookup_via_cert( Registrar, ComponentCert );
	if ( Entry == nullptr ) {
		return DGE_RSM_CERT_IS_NOT_CURRENTLY_CONTEXTED;
	}
	dg_rsm_shadowguild_node* Node = dg_rsm_shadow_lieu_node( Registrar, Entry, true );
	if ( Node == nullptr ) {
		return DGE_RSM_GROWTH_QUEUED;
	}
	atomic_fetch_or_explicit( &Node->Bits[ObjectIndex >> 6u], (dg_u64)1u << ( ObjectIndex & 63u ), memory_order_release );
	return DGE_RSM_NONE;
}

//Object no longer carries the cert. Lock-free atomic AND-NOT.
[[nodiscard]] static inline dg_errcode dg_rsm_shadow_proc_clear( dg_rsm_registrar* Registrar, dg_canon ComponentCert, dg_u32 ObjectIndex ) {
	if ( DG_UNLIKELY( Registrar == nullptr || ObjectIndex >= Registrar->ShadowMaxObjects ) ) {
		return DGE_RSM_NOT_VALID;
	}
	dg_rsm_entry* Entry = (dg_rsm_entry*)dg_rsm_registry_suro_lookup_via_cert( Registrar, ComponentCert );
	if ( Entry == nullptr ) {
		return DGE_RSM_CERT_IS_NOT_CURRENTLY_CONTEXTED;
	}
	dg_rsm_shadowguild_node* Node = atomic_load_explicit( &Entry->Shadow, memory_order_acquire );
	if ( Node == nullptr ) {
		return DGE_RSM_NONE;
	}
	atomic_fetch_and_explicit( &Node->Bits[ObjectIndex >> 6u], ~( (dg_u64)1u << ( ObjectIndex & 63u ) ), memory_order_release );
	return DGE_RSM_NONE;
}

// Mirror EVERY live cert of an object into the guild at its own index, allocating an
// index if untracked. The RETROFIT path: objects born before the guild was armed ( or
// through a manual flow ) join the implicit world here.
[[nodiscard]] static inline dg_errcode dg_rsm_shadow_proc_submit_object( dg_rsm_registrar* Registrar, dg_rsm_object* Object ) {
	if ( DG_UNLIKELY( Registrar == nullptr ) ) {
		return DGE_RSM_REGISTRAR_NULL;
	}
	if ( DG_UNLIKELY( Object == nullptr ) ) {
		return DGE_RSM_OBJECT_NULL;
	}
	if ( DG_UNLIKELY( Registrar->ShadowMaxObjects == 0u ) ) {
		return DGE_RSM_SHADOW_UNINITIALIZED;
	}
	if ( Object->Header.ShadowIndex == DG_RSM_SHADOW_NO_INDEX ) {
		Object->Header.ShadowIndex = dg_rsm_shadow_lieu_index_alloc( Registrar );
		if ( Object->Header.ShadowIndex == DG_RSM_SHADOW_NO_INDEX ) {
			return DGE_RSM_PICK_A_BIGGER_NUMBER;
		}
	}
	dg_dword Removed = Object->Header.RemovedCertsIndexes;
	bool HasRemovals = Removed.byte[0] != DG_RSM_EMPTY_INDEX;
	const dg_canon* Certs = dg_rsm_object_suro_ptr_certs( Object );
	for ( dg_u8 Idx = 0; Idx < Object->Header.ItemCount; ++Idx ) {
		if ( HasRemovals && dg_rsm_object_lieu_index_is_disenrolled( Removed, Idx ) ) {
			continue;
		}
		for ( dg_errcode E = dg_rsm_shadow_proc_submit( Registrar, Certs[Idx], Object->Header.ShadowIndex ); !dg_error_ok(E); ) {
			return E;
		}
	}
	return DGE_RSM_NONE;
}

//Mirror of submit_object for teardown: AND-NOT every cert bit ( stale clears are no-ops ).
static inline void dg_rsm_shadow_lieu_clear_object( dg_rsm_registrar* Registrar, dg_rsm_object* Object ) {
	if ( Registrar == nullptr || Registrar->ShadowMaxObjects == 0u || Object->Header.ShadowIndex == DG_RSM_SHADOW_NO_INDEX ) {
		return;
	}
	const dg_canon* Certs = dg_rsm_object_suro_ptr_certs( Object );
	for ( dg_u8 Idx = 0; Idx < Object->Header.ItemCount; ++Idx ) {
		DGE_ERROR_IGNORE( dg_rsm_shadow_proc_clear( Registrar, Certs[Idx], Object->Header.ShadowIndex ) );
	}
}

//Declare a qualifier-cert: its mask = AND of these members' masks, recomputed at drain.
[[nodiscard]] static inline dg_errcode dg_rsm_shadow_suro_define_qualifier( dg_rsm_registrar* Registrar, dg_canon QualifierCert, const dg_canon* Members, dg_u16 MemberCount ) {
	if ( DG_UNLIKELY( Registrar == nullptr ) ) {
		return DGE_RSM_REGISTRAR_NULL;
	}
	if ( DG_UNLIKELY( Members == nullptr ) ) {
		return DGE_RSM_CERTS_NULL;
	}
	dg_rsm_entry* Entry = (dg_rsm_entry*)dg_rsm_registry_suro_lookup_via_cert( Registrar, QualifierCert );
	if ( Entry == nullptr ) {
		return DGE_RSM_CERT_IS_NOT_CURRENTLY_CONTEXTED;
	}
	dg_rsm_shadowguild_node* Node = dg_rsm_shadow_lieu_node( Registrar, Entry, true );
	if ( Node == nullptr ) {
		return DGE_RSM_GROWTH_QUEUED;
	}
	Node->IsQualifier = true;
	Node->Members = Members;
	Node->MemberCount = MemberCount;
	return DGE_RSM_NONE;
}

// Drain ( SINGLE WRITER, safe point ): recompute each qualifier mask as the
// word-parallel AND of its members' live masks. Base masks need no work — submit and
// clear keep them current. Runs by itself inside registry maintenance.
[[nodiscard]] static inline dg_errcode dg_rsm_shadow_proc_drain( dg_rsm_registrar* Registrar ) {
	if ( DG_UNLIKELY( Registrar == nullptr ) ) {
		return DGE_RSM_REGISTRAR_NULL;
	}
	for ( dg_u32 Idx = 0; Idx < Registrar->TableCapacity; ++Idx ) {
		dg_rsm_entry* Entry = atomic_load_explicit( &Registrar->CertMap[Idx], memory_order_acquire );
		if ( Entry == nullptr || dg_rsm_registry_helper_lieu_is_disenrolled( Entry ) ) {
			continue;
		}
		dg_rsm_shadowguild_node* Qualifier = atomic_load_explicit( &Entry->Shadow, memory_order_acquire );
		if ( Qualifier == nullptr || !Qualifier->IsQualifier ) {
			continue;
		}
		// Resolve member nodes ONCE — the registrar probes do not belong in the word loop.
		bool AllPresent = true;
		dg_rsm_shadowguild_node* MemberNodes[Qualifier->MemberCount + 1u]; //+1 dodges a zero-length VLA.
		for ( dg_u16 Je = 0; Je < Qualifier->MemberCount; ++Je) {
			dg_rsm_entry* Member = (dg_rsm_entry*)dg_rsm_registry_suro_lookup_via_cert( Registrar, Qualifier->Members[Je] );
			MemberNodes[Je] = ( Member != nullptr ) ? atomic_load_explicit( &Member->Shadow, memory_order_acquire ) : nullptr;
			if ( MemberNodes[Je] == nullptr ) {
				AllPresent = false;
				break;
			}
		}
		for ( dg_u32 Xe = 0; Xe < Qualifier->WordCount; ++Xe ) {
			dg_u64 Accumulate = 0u;
			if ( AllPresent ) {
				Accumulate = ~(dg_u64)0u;
				for ( dg_u16 Je = 0; Je < Qualifier->MemberCount; ++Je) {
					Accumulate &= atomic_load_explicit( &MemberNodes[Je]->Bits[Xe], memory_order_acquire );
				}
			}
			atomic_store_explicit( &Qualifier->Bits[Xe], Accumulate, memory_order_release );
		}
	}
	return DGE_RSM_NONE;
}

// THE hot-loop variant: caller resolved the node ONCE ( lookup + Shadow load, or
// dg_rsm_shadow_lieu_node with Create false ) and holds it across the frame — qualify
// is then one load + bit test with ZERO registrar traffic.
[[nodiscard]] static inline bool dg_rsm_shadow_lieu_qualify_node( const dg_rsm_shadowguild_node* Node, dg_u32 ObjectIndex ) {
	if ( DG_UNLIKELY( Node == nullptr || Node->Bits == nullptr ) ) {
		return false;
	}
	return ( ( atomic_load_explicit( &Node->Bits[ObjectIndex >> 6u], memory_order_acquire ) >> ( ObjectIndex & 63u ) ) & 1u ) != 0u;
}

//Does object ObjectIndex satisfy Cert ( base OR qualifier )? One bit test, uniform.
[[nodiscard]] static inline bool dg_rsm_shadow_lieu_qualify( const dg_rsm_registrar* Registrar, dg_canon Cert, dg_u32 ObjectIndex ) {
	if ( DG_UNLIKELY( Registrar == nullptr || ObjectIndex >= Registrar->ShadowMaxObjects ) ) {
		return false;
	}
	const dg_rsm_entry* Entry = dg_rsm_registry_suro_lookup_via_cert( Registrar, Cert );
	if ( Entry == nullptr ) {
		return false;
	}
	dg_rsm_shadowguild_node* Node = atomic_load_explicit( (dg_rsm_atomic_shadowguild_node_ptr*)&Entry->Shadow, memory_order_acquire );
	return dg_rsm_shadow_lieu_qualify_node( Node, ObjectIndex );
}

//Direct qualify for a tracked object in hand: its header knows its own index.
[[nodiscard]] static inline bool dg_rsm_shadow_lieu_qualify_object( const dg_rsm_registrar* Registrar, dg_canon Cert, const dg_rsm_object* Object ) {
	if ( DG_UNLIKELY( Object == nullptr || Object->Header.ShadowIndex == DG_RSM_SHADOW_NO_INDEX ) ) {
		return false;
	}
	return dg_rsm_shadow_lieu_qualify( Registrar, Cert, Object->Header.ShadowIndex );
}

// Free every shadow node + its bitmask + the dense-index words. SINGLE WRITER, safe
// point. Disarms implicit mode ( ShadowMaxObjects returns to 0 ).
static inline void dg_rsm_shadow_suro_fini( dg_rsm_registrar* Registrar ) {
	if ( Registrar == nullptr ) {
		return;
	}
	for ( dg_u32 Idx = 0; Idx < Registrar->TableCapacity; ++Idx ) {
		dg_rsm_entry* Entry = atomic_load_explicit( &Registrar->CertMap[Idx], memory_order_relaxed );
		if ( Entry == nullptr ) {
			continue;
		}
		Entry = dg_rsm_registry_lieu_entry_resuscitate( Entry ); //Tombstoned entries can hold nodes too.
		dg_rsm_shadowguild_node* Node = atomic_load_explicit( &Entry->Shadow, memory_order_relaxed );
		if ( Node == nullptr ) {
			continue;
		}
		DGE_ERROR_IGNORE( dg_free( &Registrar->Alloc, (dg_inbytes)Node->WordCount * sizeof(dg_u64), (void*)Node->Bits ) );
		DGE_ERROR_IGNORE( dg_free( &Registrar->Alloc, sizeof(dg_rsm_shadowguild_node), Node ) );
		atomic_store_explicit( &Entry->Shadow, nullptr, memory_order_relaxed );
	}
	if ( Registrar->ShadowIndexMask != nullptr ) {
		DGE_ERROR_IGNORE( dg_free( &Registrar->Alloc, (dg_inbytes)dg_rsm_shadow_lieu_words( Registrar->ShadowMaxObjects ) * sizeof(dg_u64), (void*)Registrar->ShadowIndexMask ) );
		Registrar->ShadowIndexMask = nullptr;
		Registrar->ShadowMaxObjects = 0u;
	}
}

//
//
// Registry

// THREADING CONTRACT:
//   enroll / disenroll / lookup — lock-free, ANY thread ( CAS on one map slot ).
//   reacclimatize / grow / maintenance — SINGLE WRITER, at a quiescent SAFE POINT
//   ( no enroll/disenroll/lookup in flight ), so the rebuild can free dead entries
//   with no use-after-free. maintenance is the autonomous tick: park it on a
//   background thread ( or the frame flip ) and it decides purge-vs-grow itself;
//   grow/reacclimatize stay public for manual control.
[[nodiscard]] static inline dg_errcode dg_rsm_registry_suro_initialize( dg_rsm_registrar* OutRegistrar, dg_allocator* Alloc, dg_u32 RequestedCapacity ) {
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

// Register Serial -> Cert. Width.Value in WORDS ( 0 is legal: a zero-size cert, pure
// qualification tag — it costs no payload on any object that carries it ). Idempotent
// when re-enrolled with an identical Width; a differing Width is an error, not a
// silent overwrite. NO inline growth: a full map queues one and bails.
[[nodiscard]] static inline dg_errcode dg_rsm_registry_proc_enroll( dg_rsm_registrar* Registrar, dg_serial Serial, dg_inwords32_ptrbool Width, dg_rsm_registry_accessor* OutAccessor ) {
	if ( DG_UNLIKELY( Registrar == nullptr ) ) {
		return DGE_RSM_REGISTRAR_NULL;
	}
	if ( DG_UNLIKELY( Serial.Raw == 0u ) ) {
		return DGE_RSM_NOT_VALID;
	}

	dg_canon Cert = dg_rsm_general_lieu_serial_to_cert( Serial );

	DG_HASH_MM3_LINEAR_PROBE_HASH( Cert, Registrar->TableMask, SlotIdx, _Attempt_ ) {
		dg_rsm_entry* Real = atomic_load_explicit( &Registrar->CertMap[SlotIdx], memory_order_acquire );

		if ( Real == nullptr ) {
			dg_rsm_entry* New = {nullptr};
			// The ceph pool self-grows off the registrar's own allocator; only a true
			// backing OOM fails this, and that error passes through in its own domain.
			for ( dg_errcode E = dg_ceph_pool_proc_alloc( &Registrar->CertPF, (void**)&New ); !dg_error_ok(E); ) {
				return E;
			}
			*New = (dg_rsm_entry){ .Cert = Cert, .Width = Width, .Shadow = nullptr, .VTable = nullptr };

			dg_rsm_entry* ExpectedNull = {nullptr};
			bool ClaimedSlot = atomic_compare_exchange_strong_explicit( &Registrar->CertMap[SlotIdx], &ExpectedNull, New, memory_order_release, memory_order_relaxed );
			if ( ClaimedSlot ) {
				atomic_fetch_add_explicit( &Registrar->OccupiedSlotsCount, 1u, memory_order_relaxed );
				atomic_fetch_add_explicit( &Registrar->LivingSlotsCount,   1u, memory_order_relaxed );
				if ( OutAccessor != nullptr ) {
					*OutAccessor = (dg_rsm_registry_accessor){ .Pair = New, .ExpectedSerial = Serial, .ExpectedCert = Cert };
				}
				return DGE_RSM_NONE;
			}
			// Lost the CAS: give the slot back and re-evaluate this SAME map index.
			DGE_ERROR_IGNORE( dg_ceph_pool_proc_free_ptr( &Registrar->CertPF, New ) );
			_Attempt_SlotIdx = _Attempt_SlotIdx - 1u;
			continue;
		}
		if ( dg_rsm_registry_helper_lieu_is_disenrolled( Real ) ) {
			continue;
		}
		if ( Real->Cert == Cert ) {
			if ( Real->Width.Raw != Width.Raw ) {
				return DGE_RSM_ALREADY_ENROLLED_DIFFERENTLY;
			}
			if ( OutAccessor != nullptr ) {
				*OutAccessor = (dg_rsm_registry_accessor){ .Pair = Real, .ExpectedSerial = Serial, .ExpectedCert = Cert };
			}
			return DGE_RSM_NONE;
		}
	}

	// Whole probe span occupied: queue a growth for the next safe point and bail.
	atomic_fetch_add_explicit( &Registrar->AdditionalRequiredCount, 1u, memory_order_relaxed );
	return DGE_RSM_GROWTH_QUEUED;
}

// Logical delete, lock-free, any thread: CAS the slot to its dead-bit-tagged self.
// The entry memory is NOT freed here — reacclimatize does that at a safe point.
[[nodiscard]] static inline dg_errcode dg_rsm_registry_proc_disenroll( dg_rsm_registrar* Registrar, dg_serial Serial ) {
	if ( DG_UNLIKELY( Registrar == nullptr ) ) {
		return DGE_RSM_REGISTRAR_NULL;
	}

	dg_canon Cert = dg_rsm_general_lieu_serial_to_cert( Serial );

	DG_HASH_MM3_LINEAR_PROBE_HASH( Cert, Registrar->TableMask, SlotIdx, _Attempt_ ) {
		dg_rsm_entry* Raw = atomic_load_explicit( &Registrar->CertMap[SlotIdx], memory_order_acquire );
		if ( Raw == nullptr ) {
			return DGE_RSM_CERT_IS_NOT_CURRENTLY_CONTEXTED;
		}
		if ( dg_rsm_registry_helper_lieu_is_disenrolled( Raw ) ) {
			continue;
		}
		if ( Raw->Cert != Cert ) {
			continue;
		}

		dg_rsm_entry* Dead = dg_rsm_registry_lieu_entry_kill( Raw );
		if ( !atomic_compare_exchange_strong_explicit( &Registrar->CertMap[SlotIdx], &Raw, Dead, memory_order_acq_rel, memory_order_relaxed ) ) {
			// Someone changed it under us; re-evaluate this same slot.
			_Attempt_SlotIdx = _Attempt_SlotIdx - 1u;
			continue;
		}
		atomic_fetch_sub_explicit( &Registrar->LivingSlotsCount, 1u, memory_order_relaxed );
		atomic_fetch_add_explicit( &Registrar->TombstoneCount,   1u, memory_order_relaxed );
		return DGE_RSM_NONE;
	}
	return DGE_RSM_CERT_IS_NOT_CURRENTLY_CONTEXTED;
}

// SAFE-POINT rebuild core: fresh map at NewCapacity, carry live entries, free dead
// ones back to the pool, swap, reset counters. Shared by reacclimatize ( same size,
// pure tombstone purge ) and grow ( double ). The pool never shrinks — freed slots
// just become enroll headroom.
[[nodiscard]] static inline dg_errcode dg_rsm_registry_lieu_rebuild( dg_rsm_registrar* Registrar, dg_u32 NewCapacity ) {
	if ( DG_UNLIKELY( Registrar == nullptr ) ) {
		return DGE_RSM_REGISTRAR_NULL;
	}

	dg_u32 OldCapacity = Registrar->TableCapacity;
	dg_u32 NewMask     = NewCapacity - 1u;
	dg_inbytes NewMapBytes = (dg_inbytes)NewCapacity * sizeof(dg_rsm_atomic_entry_ptr);

	dg_rsm_atomic_entry_ptr* NewMap = {nullptr};
	for ( dg_errcode E = dg_alloc_zeroed( &Registrar->Alloc, NewMapBytes, alignof(dg_rsm_atomic_entry_ptr), (void**)&NewMap ); !dg_error_ok(E); ) {
		return E;
	}

	dg_u32 Living = 0u;
	for ( dg_u32 Idx = 0; Idx < OldCapacity; ++Idx ) {
		dg_rsm_entry* Raw = atomic_load_explicit( &Registrar->CertMap[Idx], memory_order_relaxed );
		if ( Raw == nullptr ) {
			continue;
		}
		if ( dg_rsm_registry_helper_lieu_is_disenrolled( Raw ) ) {
			DGE_ERROR_IGNORE( dg_ceph_pool_proc_free_ptr( &Registrar->CertPF, dg_rsm_registry_lieu_entry_resuscitate( Raw ) ) );
			continue;
		}
		dg_rsm_registry_lieu_rebuild_insert( NewMap, NewMask, Raw );
		Living = Living + 1u;
	}

	dg_inbytes OldMapBytes = (dg_inbytes)OldCapacity * sizeof(dg_rsm_atomic_entry_ptr);
	DGE_ERROR_IGNORE( dg_free( &Registrar->Alloc, OldMapBytes, (void*)Registrar->CertMap ) );

	Registrar->CertMap       = NewMap;
	Registrar->TableCapacity = NewCapacity;
	Registrar->TableMask     = NewMask;
	atomic_store_explicit( &Registrar->OccupiedSlotsCount, Living, memory_order_relaxed );
	atomic_store_explicit( &Registrar->LivingSlotsCount,   Living, memory_order_relaxed );
	atomic_store_explicit( &Registrar->TombstoneCount,     0u,     memory_order_relaxed );
	return DGE_RSM_NONE;
}

// Pure tombstone purge at the SAME capacity.
[[nodiscard]] static inline dg_errcode dg_rsm_registry_suro_reacclimatize( dg_rsm_registrar* Registrar ) {
	if ( DG_UNLIKELY( Registrar == nullptr ) ) {
		return DGE_RSM_REGISTRAR_NULL;
	}
	return dg_rsm_registry_lieu_rebuild( Registrar, Registrar->TableCapacity );
}

// Double the map ( also purges tombstones as a side effect ).
[[nodiscard]] static inline dg_errcode dg_rsm_registry_suro_grow( dg_rsm_registrar* Registrar ) {
	if ( DG_UNLIKELY( Registrar == nullptr ) ) {
		return DGE_RSM_REGISTRAR_NULL;
	}
	if ( DG_UNLIKELY( Registrar->TableCapacity >= 0x80000000u ) ) {
		return DGE_RSM_PICK_A_BIGGER_NUMBER; //Told you.
	}
	return dg_rsm_registry_lieu_rebuild( Registrar, Registrar->TableCapacity << 1u );
}

// The autonomous safe-point pass. Consumes queued growth requests from the lock-free
// path: purge first when tombstone-heavy ( cheap, frees probe slots ), else grow.
// With no pressure it opportunistically purges a table >= 25% clogged by tombstones.
// An armed ShadowGuild gets its qualifier masks drained here too — the tick IS the
// drain site in implicit mode, nobody else has to remember.
[[nodiscard]] static inline dg_errcode dg_rsm_registry_proc_maintenance( dg_rsm_registrar* Registrar ) {
	if ( DG_UNLIKELY( Registrar == nullptr ) ) {
		return DGE_RSM_REGISTRAR_NULL;
	}

	dg_u32 Capacity   = Registrar->TableCapacity;
	dg_u32 Tombstones = atomic_load_explicit( &Registrar->TombstoneCount, memory_order_relaxed );

	if ( atomic_load_explicit( &Registrar->AdditionalRequiredCount, memory_order_relaxed ) > 0u ) {
		dg_errcode E;
		if ( Tombstones * 4u >= Capacity ) {
			E = dg_rsm_registry_suro_reacclimatize( Registrar );
		} else {
			E = dg_rsm_registry_suro_grow( Registrar );
		}
		if ( !dg_error_ok( E ) ) {
			return E;
		}
		atomic_store_explicit( &Registrar->AdditionalRequiredCount, 0u, memory_order_relaxed );
	} else if ( Tombstones * 4u >= Capacity ) {
		for ( dg_errcode E = dg_rsm_registry_suro_reacclimatize( Registrar ); !dg_error_ok(E); ) {
			return E;
		}
	}

	if ( Registrar->ShadowMaxObjects != 0u ) {
		return dg_rsm_shadow_proc_drain( Registrar );
	}
	return DGE_RSM_NONE;
}

// Terminal teardown, SINGLE WRITER: releases auto-grown pool slabs, the embedded
// first slab's backing, and the map. Entries die with their slabs — no per-entry
// walk. Shadow nodes are NOT freed here; run dg_rsm_shadow_suro_fini FIRST if the
// ShadowGuild was ever used.
static inline void dg_rsm_registry_suro_destroy( dg_rsm_registrar* Registrar ) {
	if ( Registrar == nullptr || Registrar->CertMap == nullptr ) {
		return;
	}
	dg_ceph_pool_proc_destroy( &Registrar->CertPF );
	DGE_ERROR_IGNORE( dg_free( &Registrar->Alloc, (dg_inbytes)Registrar->BaseCertPF.SlotTotal * sizeof(dg_rsm_entry), Registrar->BaseCertPF.Memory ) );
	DGE_ERROR_IGNORE( dg_free( &Registrar->Alloc, (dg_inbytes)Registrar->TableCapacity * sizeof(dg_rsm_atomic_entry_ptr), (void*)Registrar->CertMap ) );
	Registrar->CertMap = nullptr;
	// Backstop for a skipped fini: the index words die with the registrar.
	if ( Registrar->ShadowIndexMask != nullptr ) {
		DGE_ERROR_IGNORE( dg_free( &Registrar->Alloc, (dg_inbytes)dg_rsm_shadow_lieu_words( Registrar->ShadowMaxObjects ) * sizeof(dg_u64), (void*)Registrar->ShadowIndexMask ) );
		Registrar->ShadowIndexMask = nullptr;
	}
}

//dg_errcode dg_rsm_registry_process_become_library( dg_rsm_registrar* Registrar, dg_uqword Secert, dg_serial EmptySerial )

//
//
// Objects
//
// THREADING CONTRACT ( per object ):
//   struct reads / mutate_to_pointer / pointer_exchange — lock-free, any thread.
//   create / remove_item / readd_item / cleanfn — SINGLE WRITER for that object.
// Ownership model: BORROWED. Pointer slots reference external memory the object
// does not own; cleanfn invalidates in place and frees nothing.

//Slot width decision: overrides win, else the registrar width. A held pointer is one word.
static inline dg_inwords32_ptrbool dg_rsm_object_lieu_field_of( const dg_rsm_entry* Entry, const bool* PointerOverrides, dg_u8 Index ) {
	dg_inwords32_ptrbool Field = Entry->Width;
	if ( PointerOverrides != nullptr ) {
		Field.IsPointer = PointerOverrides[Index];
	}
	if ( Field.IsPointer ) {
		Field.Value = 1u;
	}
	return Field;
}

static inline void dg_rsm_object_lieu_hash_insert( dg_byte* HashTable, dg_u32 Mask, dg_canon Cert, dg_byte Index ) {
	DG_HASH_MM3_LINEAR_PROBE_HASH( Cert, Mask, SlotIdx, _Attempt_ ) {
		if ( HashTable[SlotIdx] == DG_RSM_EMPTY_BUCKET ) {
			HashTable[SlotIdx] = Index;
			return;
		}
	}
}

// THE constructor. Resolves every cert against the live registrar, lays out one flat
// blob, writes all regions. Zero-width certs cost nothing in payload — they exist for
// carries/qualify only; do NOT dereference their slot. ItemCount is dg_u8, so the
// 255 ceiling is enforced by the type ( max index 254 never collides with 0xFF ).
// An ARMED ShadowGuild enrolls the newborn implicitly: dense index + one bit per cert.
[[nodiscard]] static inline dg_errcode dg_rsm_object_suro_create( dg_allocator* Alloc, dg_rsm_registrar* Registrar, dg_u8 ItemCount, const dg_canon* RequiredCerts, dg_canon SelfCert, const bool* PointerOverrides, dg_rsm_object** OutObject ) {
	if ( DG_UNLIKELY( Alloc == nullptr ) ) {
		return DGE_RSM_ALLOC_NULL;
	}
	if ( DG_UNLIKELY( Registrar == nullptr ) ) {
		return DGE_RSM_REGISTRAR_NULL;
	}
	if ( DG_UNLIKELY( OutObject == nullptr ) ) {
		return DGE_RSM_OBJECT_NULL;
	}
	if ( DG_UNLIKELY( RequiredCerts == nullptr ) ) {
		return DGE_RSM_CERTS_NULL;
	}
	if ( ItemCount == 0u ) {
		return DGE_RSM_SIZE_IS_ZERO;
	}

	dg_inwords64 TotalPayloadWords = 0;
	const dg_rsm_entry* Entries[ItemCount];
	for ( dg_u8 Idx = 0; Idx < ItemCount; ++Idx ) {
		// Certs are UNIQUE per object: a duplicate would shadow its twin in the hash
		// table and double-allocate payload.
		for ( dg_u8 Xe = 0; Xe < Idx; ++Xe ) {
			if ( RequiredCerts[Xe] == RequiredCerts[Idx] ) {
				return DGE_RSM_NOT_VALID;
			}
		}
		Entries[Idx] = dg_rsm_registry_suro_lookup_via_cert( Registrar, RequiredCerts[Idx] );
		if ( Entries[Idx] == nullptr ) {
			return DGE_RSM_CERT_IS_NOT_CURRENTLY_CONTEXTED;
		}
		TotalPayloadWords += dg_rsm_object_lieu_field_of( Entries[Idx], PointerOverrides, Idx ).Value;
	}
	// SOFT STOP: creation is fixable — recalculate widths and come back.
	if ( TotalPayloadWords > 0xFFFFu ) {
		return DGE_RSM_OBJECT_SIZE_OVERFLOW;
	}

	dg_rsm_object Proto = dg_rsm_object_lieu_compute_simple_offsets( ItemCount, 0u, (dg_inwords)TotalPayloadWords );
	Proto.Header.SelfCert = SelfCert;
	if ( Proto.Header.MemoryLength == 0u ) {
		return DGE_RSM_OBJECT_SIZE_OVERFLOW; //Payload fit but payload + overhead does not.
	}

	dg_inbytes TotalBytes = sizeof(dg_rsm_subsequor) + DG_AS_BYTES( (dg_inbytes)Proto.Header.MemoryLength );
	dg_rsm_object* FObject = {nullptr};
	for ( dg_errcode E = dg_alloc_zeroed( Alloc, TotalBytes, alignof(dg_rsm_object), (void**)&FObject ); !dg_error_ok(E); ) {
		return E;
	}
	FObject->Header = Proto.Header;

	dg_canon* OutCerts               = dg_rsm_object_suro_ptr_certs( FObject );
	dg_inwords32_ptrbool* OutOffsets = dg_rsm_object_suro_ptr_payload_locations( FObject );
	dg_byte* OutHash                 = dg_rsm_object_suro_ptr_hashtable( FObject );

	// alloc_zeroed gives zeros; the hash table needs 0xFF empty sentinels.
	dg_u32 HashCapacity = (dg_u32)FObject->Header.HashTableMask + 1u;
	for ( dg_u32 Idx = 0; Idx < HashCapacity; ++Idx ) {
		OutHash[Idx] = DG_RSM_EMPTY_BUCKET;
	}

	// Raw word offsets from Memory[0]: payload runs [MemoryLength - TotalPayloadWords, MemoryLength).
	dg_inwords Cursor = (dg_inwords)FObject->Header.MemoryLength - (dg_inwords)TotalPayloadWords;
	for ( dg_u8 Idx = 0; Idx < ItemCount; ++Idx) {
		dg_inwords32_ptrbool Field = dg_rsm_object_lieu_field_of( Entries[Idx], PointerOverrides, Idx );
		OutCerts[Idx] = RequiredCerts[Idx];
		OutOffsets[Idx].IsPointer = Field.IsPointer;
		OutOffsets[Idx].Value = Cursor;
		Cursor += Field.Value;
		dg_rsm_object_lieu_hash_insert( OutHash, FObject->Header.HashTableMask, RequiredCerts[Idx], Idx );
	}
	//FAKE tail entry [ItemCount]: pinned to MemoryLength so physical width is branchless.
	OutOffsets[ItemCount] = (dg_inwords32_ptrbool){ .Value = FObject->Header.MemoryLength };

	// IMPLICIT ShadowGuild enroll. Nodes are resolved BEFORE any bit lands, so a failed
	// node alloc unwinds with zero stale bits on the fresh index.
	if ( Registrar->ShadowMaxObjects != 0u ) {
		dg_u32 FreshIndex = dg_rsm_shadow_lieu_index_alloc( Registrar );
		if ( FreshIndex == DG_RSM_SHADOW_NO_INDEX ) {
			DGE_ERROR_IGNORE( dg_free( Alloc, TotalBytes, FObject ) );
			return DGE_RSM_PICK_A_BIGGER_NUMBER; //Guild full.
		}
		dg_rsm_shadowguild_node* Nodes[ItemCount];
		for ( dg_u8 Idx = 0; Idx < ItemCount; ++Idx ) {
			Nodes[Idx] = dg_rsm_shadow_lieu_node( Registrar, (dg_rsm_entry*)Entries[Idx], true );
			if ( Nodes[Idx] == nullptr ) {
				dg_rsm_shadow_lieu_index_free( Registrar, FreshIndex );
				DGE_ERROR_IGNORE( dg_free( Alloc, TotalBytes, FObject ) );
				return DGE_RSM_GROWTH_QUEUED;
			}
		}
		for ( dg_u8 Idx = 0; Idx < ItemCount; ++Idx ) {
			atomic_fetch_or_explicit( &Nodes[Idx]->Bits[FreshIndex >> 6u], (dg_u64)1u << ( FreshIndex & 63u ), memory_order_release );
		}
		FObject->Header.ShadowIndex = FreshIndex;
	}

	*OutObject = FObject;
	return DGE_RSM_NONE;
}

// Remove a component the FREE way: tombstone its slot index in the header dword. No
// allocation, no blob rewrite — the slot stays physically present but is masked out
// of lookups/carries/qualify. Up to 4 concurrent removals; idempotent.
// IMPLICIT: the guild bit falls with the ledger ( pass the registrar; nullptr or an
// unarmed guild degrades to ledger-only ).
[[nodiscard]] static inline dg_errcode dg_rsm_object_proc_remove_item( dg_rsm_registrar* Registrar, dg_rsm_object* Object, dg_canon Cert ) {
	if ( DG_UNLIKELY( Object == nullptr ) ) {
		return DGE_RSM_OBJECT_NULL;
	}
	dg_i16 Index = dg_rsm_object_lieu_retrieve_cert_index( Object, Cert );
	if ( Index < 0 ) {
		return DGE_RSM_CERT_IS_NOT_CURRENTLY_CONTEXTED;
	}
	dg_byte* Removed = Object->Header.RemovedCertsIndexes.byte;
	for ( dg_u8 Idx = 0; Idx < 4u; ++Idx ) {
		if ( Removed[Idx] == (dg_byte)Index ) {
			return DGE_RSM_NONE;
		}
	}
	for ( dg_u8 Idx = 0; Idx < 4u; ++Idx ) {
		if ( Removed[Idx] == DG_RSM_EMPTY_INDEX ) {
			Removed[Idx] = (dg_byte)Index;
			if ( Registrar != nullptr && Registrar->ShadowMaxObjects != 0u && Object->Header.ShadowIndex != DG_RSM_SHADOW_NO_INDEX ) {
				DGE_ERROR_IGNORE( dg_rsm_shadow_proc_clear( Registrar, Cert, Object->Header.ShadowIndex ) );
			}
			return DGE_RSM_NONE;
		}
	}
	return DGE_RSM_REMOVED_CERTS_FULL;
}

// Un-tombstone a still-physically-present component, compacting left so byte[0] ==
// 0xFF keeps meaning "no removals" ( the hot-path fast gate in retrieve_at_index ).
// IMPLICIT: the guild bit rises again with the ledger.
[[nodiscard]] static inline dg_errcode dg_rsm_object_proc_readd_item( dg_rsm_registrar* Registrar, dg_rsm_object* Object, dg_canon Cert ) {
	if ( DG_UNLIKELY( Object == nullptr ) ) {
		return DGE_RSM_OBJECT_NULL;
	}
	dg_i16 Index = dg_rsm_object_lieu_retrieve_cert_index( Object, Cert );
	if ( Index < 0 ) {
		return DGE_RSM_CERT_IS_NOT_CURRENTLY_CONTEXTED;
	}
	dg_byte* Removed = Object->Header.RemovedCertsIndexes.byte;
	for ( dg_u8 Idx = 0; Idx < 4u; ++Idx ) {
		if ( Removed[Idx] == (dg_byte)Index ) {
			for ( dg_u8 Xe = Idx; Xe + 1u < 4u; ++Xe ) {
				Removed[Xe] = Removed[Xe + 1u];
			}
			Removed[3] = DG_RSM_EMPTY_INDEX;
			if ( Registrar != nullptr && Registrar->ShadowMaxObjects != 0u && Object->Header.ShadowIndex != DG_RSM_SHADOW_NO_INDEX ) {
				return dg_rsm_shadow_proc_submit( Registrar, Cert, Object->Header.ShadowIndex );
			}
			return DGE_RSM_NONE;
		}
	}
	return DGE_RSM_NONE; //Present and live already.
}

// LAYOUT INVARIANT: slots are laid out in INDEX ORDER ( create and rebuild both walk
// 0..N-1 with an advancing cursor ), so a slot's PHYSICAL width is the delta to the
// next offset — BRANCHLESS because the offset array carries a FAKE tail entry at
// [ItemCount] pinned to MemoryLength. This stays truthful after a mutate_to_pointer
// flip, where the registrar width and the physical width diverge.
static inline dg_inwords dg_rsm_object_lieu_slot_physical_words( const dg_rsm_object* Object, dg_u8 Index ) {
	const dg_inwords32_ptrbool* Offsets = dg_rsm_object_suro_ptr_payload_locations( Object );
	return Offsets[(dg_u16)Index + 1u].Value - Offsets[Index].Value;
}

// The one structural rebuilder. SINGLE WRITER for the object. Survivors keep index
// order and their exact physical widths; DropRemoved compacts tombstoned slots out
// for real ( the ledger resets ). FreshCert == 0 means pure repack. The OLD object
// is NOT freed — the caller owns the swap and frees the old blob by its own
// dg_rsm_object_lieu_sum_of_bytes. Carried early-qualify bits stay valid ( append
// only widens matches, dropped slots were already masked ) but combos involving the
// fresh cert need a recompute.
[[nodiscard]] static inline dg_errcode dg_rsm_object_lieu_rebuild( dg_allocator* Alloc, dg_rsm_object* Object, dg_canon FreshCert, dg_inwords32_ptrbool FreshField, bool DropRemoved, dg_rsm_object** OutFreshObject ) {
	if ( DG_UNLIKELY( Alloc == nullptr ) ) {
		return DGE_RSM_ALLOC_NULL;
	}
	if ( DG_UNLIKELY( Object == nullptr || OutFreshObject == nullptr ) ) {
		return DGE_RSM_OBJECT_NULL;
	}
	dg_u8 OldCount = Object->Header.ItemCount;
	if ( OldCount == 0u ) {
		return DGE_RSM_SIZE_IS_ZERO; //Cleaned or corrupt: nothing to rebuild from.
	}

	dg_dword Removed  = Object->Header.RemovedCertsIndexes;
	bool HasRemovals  = Removed.byte[0] != DG_RSM_EMPTY_INDEX;
	bool Appending    = FreshCert != 0u;

	dg_u8 Survivors[OldCount];
	dg_u8 SurvivorCount = 0;
	dg_inwords PayloadWords = 0;
	for ( dg_u8 Idx = 0; Idx < OldCount; ++Idx ) {
		if ( DropRemoved && HasRemovals && dg_rsm_object_lieu_index_is_disenrolled( Removed, Idx ) ) {
			continue;
		}
		Survivors[SurvivorCount] = Idx;
		SurvivorCount = SurvivorCount + 1u;
		PayloadWords += dg_rsm_object_lieu_slot_physical_words( Object, Idx );
	}

	dg_u16 FreshCount = (dg_u16)( SurvivorCount + ( Appending ? 1u : 0u ) );
	if ( FreshCount == 0u ) {
		return DGE_RSM_SIZE_IS_ZERO;
	}
	if ( FreshCount > 255u ) {
		return DGE_RSM_ITEM_COUNT_EXCEEDS_255;
	}
	dg_inwords FreshWords = 0;
	if ( Appending ) {
		FreshWords = FreshField.IsPointer ? 1u : FreshField.Value;
	}
	// HARD STOP: removals were already dropped above — if it still cannot fit, no
	// runtime fix exists for this object.
	if ( (dg_inwords64)PayloadWords + FreshWords > 0xFFFFu ) {
		return DGE_RSM_OBJECT_SIZE_IMPOSSIBLE_MUTATION;
	}

	dg_rsm_object Proto = dg_rsm_object_lieu_compute_simple_offsets( (dg_u8)FreshCount, 0u, PayloadWords + FreshWords );
	Proto.Header.SelfCert = Object->Header.SelfCert;
	Proto.Header.ShadowIndex = Object->Header.ShadowIndex; //Identity survives a rebuild.
	if ( Proto.Header.MemoryLength == 0u ) {
		return DGE_RSM_OBJECT_SIZE_IMPOSSIBLE_MUTATION; //Payload fit but payload + overhead does not.
	}

	dg_inbytes TotalBytes = sizeof(dg_rsm_subsequor) + DG_AS_BYTES( (dg_inbytes)Proto.Header.MemoryLength );
	dg_rsm_object* FObject = {nullptr};
	for ( dg_errcode E = dg_alloc_zeroed( Alloc, TotalBytes, alignof(dg_rsm_object), (void**)&FObject ); !dg_error_ok(E); ) {
		return E;
	}
	FObject->Header = Proto.Header;

	dg_canon* FreshCerts               = dg_rsm_object_suro_ptr_certs( FObject );
	dg_inwords32_ptrbool* FreshOffsets = dg_rsm_object_suro_ptr_payload_locations( FObject );
	dg_byte* FreshHash                 = dg_rsm_object_suro_ptr_hashtable( FObject );

	dg_u32 HashCapacity = (dg_u32)FObject->Header.HashTableMask + 1u;
	for ( dg_u32 Idx = 0; Idx < HashCapacity; ++Idx ) {
		FreshHash[Idx] = DG_RSM_EMPTY_BUCKET;
	}

	const dg_canon* OldCerts               = dg_rsm_object_suro_ptr_certs( Object );
	const dg_inwords32_ptrbool* OldOffsets = dg_rsm_object_suro_ptr_payload_locations( Object );

	dg_inwords Cursor = (dg_inwords)FObject->Header.MemoryLength - ( PayloadWords + FreshWords );
	for ( dg_u8 Idx = 0; Idx < SurvivorCount; ++Idx) {
		dg_u8 OrderIndex = Survivors[Idx];
		dg_inwords Words = dg_rsm_object_lieu_slot_physical_words( Object, OrderIndex );
		FreshCerts[Idx] = OldCerts[OrderIndex];
		FreshOffsets[Idx].IsPointer = OldOffsets[OrderIndex].IsPointer;
		FreshOffsets[Idx].Value = Cursor;
		for ( dg_inwords Xe = 0; Xe < Words; ++Xe ) {
			FObject->Memory[Cursor + Xe] = Object->Memory[OldOffsets[OrderIndex].Value + Xe];
		}
		dg_rsm_object_lieu_hash_insert( FreshHash, FObject->Header.HashTableMask, OldCerts[OrderIndex], Idx );
		Cursor += Words;
	}
	if ( Appending ) {
		FreshCerts[SurvivorCount] = FreshCert;
		FreshOffsets[SurvivorCount].IsPointer = FreshField.IsPointer;
		FreshOffsets[SurvivorCount].Value = Cursor;
		dg_rsm_object_lieu_hash_insert( FreshHash, FObject->Header.HashTableMask, FreshCert, SurvivorCount );
		//Slot bytes are already zero from alloc_zeroed.
	}
	//FAKE tail entry [FreshCount]: pinned to MemoryLength so physical width is branchless.
	FreshOffsets[FreshCount] = (dg_inwords32_ptrbool){ .Value = FObject->Header.MemoryLength };

	// Removal ledger: repack pays it off ( Proto is all 0xFF ); append carries it.
	if ( !DropRemoved ) {
		FObject->Header.RemovedCertsIndexes = Object->Header.RemovedCertsIndexes;
	}
	*dg_rsm_object_suro_ptr_earlyqualfier( FObject ) = *dg_rsm_object_suro_ptr_earlyqualfier( Object );

	*OutFreshObject = FObject;
	return DGE_RSM_NONE;
}

// Structural mutation, the selfish-runner rule: a CLEAN object takes the cheap
// append ( survivors replicate verbatim ); an object with pending removals was
// going to owe a repack anyway — the moment load shows up, maintenance is expected,
// so the mutation pays the full repack and the ledger resets. A cert already
// physically present degrades to a readd ( *OutFreshObject = the same object ).
// IMPLICIT: the appended cert's guild bit rises with the fresh blob — same index,
// object identity survives the rebuild.
[[nodiscard]] static inline dg_errcode dg_rsm_object_proc_mutate( dg_allocator* Alloc, dg_rsm_registrar* Registrar, dg_rsm_object* Object, dg_canon FreshCert, bool FreshIsPointer, dg_rsm_object** OutFreshObject ) {
	if ( DG_UNLIKELY( Registrar == nullptr ) ) {
		return DGE_RSM_REGISTRAR_NULL;
	}
	if ( DG_UNLIKELY( Object == nullptr || OutFreshObject == nullptr ) ) {
		return DGE_RSM_OBJECT_NULL;
	}
	if ( DG_UNLIKELY( FreshCert == 0u ) ) {
		return DGE_RSM_NOT_VALID;
	}
	if ( dg_rsm_object_lieu_retrieve_cert_index( Object, FreshCert ) >= 0 ) {
		*OutFreshObject = Object;
		return dg_rsm_object_proc_readd_item( Registrar, Object, FreshCert );
	}
	const dg_rsm_entry* Entry = dg_rsm_registry_suro_lookup_via_cert( Registrar, FreshCert );
	if ( Entry == nullptr ) {
		return DGE_RSM_CERT_IS_NOT_CURRENTLY_CONTEXTED;
	}
	dg_inwords32_ptrbool Field = Entry->Width;
	Field.IsPointer = FreshIsPointer;
	if ( Field.IsPointer ) {
		Field.Value = 1u;
	}

	bool Loaded = Object->Header.RemovedCertsIndexes.byte[0] != DG_RSM_EMPTY_INDEX;
	for ( dg_errcode E = dg_rsm_object_lieu_rebuild( Alloc, Object, FreshCert, Field, Loaded, OutFreshObject ); !dg_error_ok(E); ) {
		return E;
	}
	if ( Registrar->ShadowMaxObjects != 0u && (*OutFreshObject)->Header.ShadowIndex != DG_RSM_SHADOW_NO_INDEX ) {
		return dg_rsm_shadow_proc_submit( Registrar, FreshCert, (*OutFreshObject)->Header.ShadowIndex );
	}
	return DGE_RSM_NONE;
}

//Manual compaction: physically drop tombstoned slots, add nothing. Same swap contract.
[[nodiscard]] static inline dg_errcode dg_rsm_object_suro_repack( dg_allocator* Alloc, dg_rsm_object* Object, dg_rsm_object** OutFreshObject ) {
	return dg_rsm_object_lieu_rebuild( Alloc, Object, 0u, (dg_inwords32_ptrbool){ .Raw = 0u }, true, OutFreshObject );
}

// THE HANDLE. Blobs are structurally immutable once published — a rebuild's multi
// word transition ( count + hash + offsets + payload ) can never be atomic, so the
// atomicity lives in the ONE word that names the object. Readers acquire-load and
// get a fully-built old or new blob, never a torn middle; word-atomics inside a blob
// ( mutate_to_pointer / pointer_exchange ) stay legal because they move no layout.
// The rotated-out blob is freed at a SAFE POINT — a reader may still walk it this frame.
typedef _Atomic(dg_rsm_object*) dg_rsm_atomic_object_handle;

[[nodiscard]] static inline dg_rsm_object* dg_rsm_object_lieu_handle_read( dg_rsm_atomic_object_handle* Handle ) {
	return atomic_load_explicit( Handle, memory_order_acquire );
}

//Unconditional rotate: publish Fresh, hand back the previous blob for deferred free.
[[nodiscard]] static inline dg_rsm_object* dg_rsm_object_proc_handle_rotate( dg_rsm_atomic_object_handle* Handle, dg_rsm_object* Fresh ) {
	return atomic_exchange_explicit( Handle, Fresh, memory_order_acq_rel );
}

// Guarded rotate: publishes ONLY if the handle still names Expected — the blob this
// rebuild was computed FROM. A lost race means another mutation won: free YOUR fresh
// blob and rebuild from the winner. This is the cheap enforcement of the
// single-structural-writer contract when ownership is ambiguous.
[[nodiscard]] static inline bool dg_rsm_object_proc_handle_rotate_expected( dg_rsm_atomic_object_handle* Handle, dg_rsm_object* Expected, dg_rsm_object* Fresh ) {
	return atomic_compare_exchange_strong_explicit( Handle, &Expected, Fresh, memory_order_acq_rel, memory_order_acquire );
}

// Atomically point a slot at external memory: store the pointer ( release ), then
// CAS-flip IsPointer without corrupting the packed Value offset. Readers racing this
// see either the old inline value or the published pointer — never a torn offset.
[[nodiscard]] static inline dg_errcode dg_rsm_object_proc_mutate_to_pointer( dg_rsm_object* Object, dg_canon Cert, void* ExternalPtr ) {
	if ( DG_UNLIKELY( Object == nullptr ) ) {
		return DGE_RSM_OBJECT_NULL;
	}
	dg_i16 Index = dg_rsm_object_lieu_retrieve_cert_index( Object, Cert );
	if ( Index < 0 ) {
		return DGE_RSM_CERT_IS_NOT_CURRENTLY_CONTEXTED;
	}
	// Zero-width tag: it has no slot of its own — a store here would land on its
	// neighbor ( or past the blob when trailing ).
	if ( dg_rsm_object_lieu_slot_physical_words( Object, (dg_u8)Index ) == 0u ) {
		return DGE_RSM_NOT_A_POINTER_SLOT;
	}

	dg_inwords32_ptrbool* OffsetPtr = &dg_rsm_object_suro_ptr_payload_locations( Object )[Index];
	_Atomic(dg_u32)* AtomicRaw = (_Atomic(dg_u32)*)&OffsetPtr->Raw;

	dg_inwords32_ptrbool Current;
	Current.Raw = atomic_load_explicit( AtomicRaw, memory_order_acquire );

	_Atomic(dg_ptr)* Slot = (_Atomic(dg_ptr)*)( Object->Memory + Current.Value );
	atomic_store_explicit( Slot, (dg_ptr)ExternalPtr, memory_order_release );

	if ( !Current.IsPointer ) {
		dg_inwords32_ptrbool Fresh = Current;
		Fresh.IsPointer = true;
		while ( !atomic_compare_exchange_weak_explicit( AtomicRaw, &Current.Raw, Fresh.Raw, memory_order_release, memory_order_relaxed ) ) {
			if ( Current.IsPointer ) {
				break; //Another thread already flipped it.
			}
			Fresh.Value = Current.Value;
		}
	}
	return DGE_RSM_NONE;
}

// Atomic swap of an already-pointer slot's target. The old pointer comes back to the
// caller ( who owns it — BORROWED model ), so held memory can rotate with no lock.
[[nodiscard]] static inline dg_errcode dg_rsm_object_proc_pointer_exchange( dg_rsm_object* Object, dg_canon Cert, void* FreshPtr, void** OutPreviousPtr ) {
	if ( DG_UNLIKELY( Object == nullptr ) ) {
		return DGE_RSM_OBJECT_NULL;
	}
	dg_i16 Index = dg_rsm_object_lieu_retrieve_cert_index( Object, Cert );
	if ( Index < 0 ) {
		return DGE_RSM_CERT_IS_NOT_CURRENTLY_CONTEXTED;
	}
	dg_inwords32_ptrbool Locale = dg_rsm_object_suro_ptr_payload_locations( Object )[Index];
	if ( !Locale.IsPointer ) {
		return DGE_RSM_NOT_A_POINTER_SLOT;
	}
	_Atomic(dg_ptr)* Slot = (_Atomic(dg_ptr)*)( Object->Memory + Locale.Value );
	dg_ptr Previous = atomic_exchange_explicit( Slot, (dg_ptr)FreshPtr, memory_order_acq_rel );
	if ( OutPreviousPtr != nullptr ) {
		*OutPreviousPtr = (void*)Previous;
	}
	return DGE_RSM_NONE;
}

// Invalidate in place ( BORROWED: the object owns no pointer targets, frees nothing ).
// ItemCount = 0 makes every lookup miss; MemoryLength survives so the owner can still
// size the free of the blob itself. IMPLICIT: cert bits fall and the dense index
// recycles BEFORE the header is wiped ( nullptr Registrar degrades to wipe-only ).
[[nodiscard]] static inline dg_errcode dg_rsm_object_suro_cleanfn( dg_rsm_registrar* Registrar, dg_rsm_object* Object ) {
	if ( DG_UNLIKELY( Object == nullptr ) ) {
		return DGE_RSM_OBJECT_NULL;
	}
	if ( Registrar != nullptr ) {
		dg_rsm_shadow_lieu_clear_object( Registrar, Object );
		dg_rsm_shadow_lieu_index_free( Registrar, Object->Header.ShadowIndex );
	}
	Object->Header.ShadowIndex = DG_RSM_SHADOW_NO_INDEX;
	Object->Header.ItemCount = 0u;
	Object->Header.SelfCert = 0u;
	return DGE_RSM_NONE;
}

//
//
// DungeonQualifer
//
// The OBJECT-SIDE early qualifier: qualification baked into the blob at create time.
// A declared qualifier is a component combination assigned a bit ( 0..63 ) in the
// hidden qualify word. Evaluate once, then qualify is one shift+AND with zero lookups
// — the cheap has-check the game loop actually runs.

// Bake the object's qualifier bitmask from its own cert list. Call at create and
// after any remove/readd ( removals honestly change what the object qualifies as ).
static inline void dg_rsm_object_suro_compute_qualifiers( dg_rsm_object* Object, const dg_rsm_qualifier_set* Set ) {
	dg_u64 Mask = 0u;
	if ( Set != nullptr ) {
		for ( dg_u16 Idx = 0; Idx < Set->Count; ++Idx ) {
			const dg_rsm_qualifier* Qualifier = &Set->Items[Idx];
			bool CarriesAll = true;
			for ( dg_u16 Xe = 0; Xe < Qualifier->MemberCount; ++Xe) {
				if ( !dg_rsm_object_lieu_carries_cert( Object, Qualifier->Members[Xe] ) ) {
					CarriesAll = false;
					break;
				}
			}
			if ( CarriesAll ) {
				Mask |= (dg_u64)1u << Qualifier->Bit;
			}
		}
	}
	*dg_rsm_object_suro_ptr_earlyqualfier( Object ) = Mask;
}

//THE fast path: one shift + AND. No registrar, no hash, no pointer chase.
[[nodiscard]] static inline bool dg_rsm_object_lieu_qualify( dg_rsm_object* Object, dg_u8 Bit ) {
	return ( ( *dg_rsm_object_suro_ptr_earlyqualfier( Object ) >> Bit ) & 1u ) != 0u;
}
