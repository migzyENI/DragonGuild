//dg_rsm.h
// (C) MigzyENT 2026

#ifndef DG_RSM_H
#define DG_RSM_H

#include "dg_error.h"
#include <stdatomic.h>
#pragma once

#if !defined(__cplusplus) && __STDC_VERSION__ < 202311L
#error "This file requires C23 or later."
#endif

#include <dg_typedef.h>
#include <dg_errcode.h>
#include <dg_arena_pool.h>
#include <dg_allocator.h>
#include <dg_hash.h>

constexpr dg_u32 DG_RSM_DWORD_NAME = 0x52534D20; // 'RSM '

DG_ERRCODE_TABLE_GENERATE(
	DGE_RSM, _IDX, _COUNT, DG_RSM_DWORD_NAME,
	NOT_VALID,
	ALLOCATOR_NULL_PTR,
	REGISTRAR_NULL_PTR,
	OBJECT_NULL_PTR,
	CERTS_NULL_PTR,
	CANON_ARRAY_NULL_PTR,
	SERIAL_ARRAY_NULL_PTR,
	MISMATCH_ATOMIC,
	SELF_CERT_WAS_OMFG_SENTINEL,
	SELF_CERT_WAS_OMFG_NOT_YOU,
	SIZE_IS_ZERO,
	SIZE_PROVIDED_HAS_31b_OVERFLOW,
	SERIAL_ALREADY_REGISTERED,
	REGISTRAR_SUPPOSED_FULL,
	CERT_IS_NOT_CURRENTLY_CONTEXTED,
	ITEM_COUNT_EXCEEDS_255
)

typedef dg_u64 dg_serial;
typedef dg_u64 dg_canon;

//Requires serialization implementation for endianess.
// For (Object) context on reading back struct
// For (Registrar) context, provide total size of 31bits.
typedef struct dg_special_inwords {
	union {
		struct {
		bool IsPointer : 1;
		dg_inwords Value : 31;
		};
		dg_u32 Raw;
	};
} dg_special_inwords;

typedef struct dg_subsequor {
	dg_u16 ItemCount;
	dg_inwords16 PayloadOffset; // Word offset to the start of component data
	dg_inwords16 CertsArrayOffset; // Word offset to dg_canonc array
	dg_inwords16 AheadOffsetsArrayOffset; // Word offset to dg_special_inwords array, it provides more offsets.
	dg_u16 HashTableCapacity; //For debug, include check that ensures this is 2^X.
	dg_inwords16 HashTableOffset; // Word offset to the embedded byte hash table (8bit size)
	//dg_inwords16 ValidPtrTableaOffset;
	dg_u16 PADDING;
	dg_canon Cert;
} dg_subsequor;

typedef struct dg_object {
	dg_subsequor Header;
	dg_u64 Memory[];
} dg_object;


// The reason the struct is atomic is that individually both are covered by it instead of a single one requiring 1 less lock
typedef struct dg_certpairs {
	dg_u16 Secret;
	dg_u16 ShortAlias; // Simple two byte shorthand, print this as ASCII.
	dg_special_inwords StructWidth;
	dg_serial Serial;
	dg_canon Cert;
} dg_certpairs;

typedef _Atomic(dg_certpairs*) dg_atomic_certpair_ptr;
typedef _Atomic(dg_serial)* dg_atomic_serial_ptr;
typedef _Atomic(dg_canon)* dg_atomic_canon_ptr;

typedef struct dg_registrar {

	dg_allocator Alloc;

	dg_pool_fixed CertPool;
	dg_pool_slab BaseSlab;

	// The Lock-Free Lookup Tables (Cap. Must be a Power of 2)
	dg_u32 TableCapacity;
	dg_u32 TableMask;

	dg_atomic_certpair_ptr* MapBySerial;
	dg_atomic_certpair_ptr* MapByCert;
} dg_registrar;

typedef struct dg_registry_accessor {
	const dg_certpairs* Pair;
	dg_serial ExpectedSerial;
	dg_canon ExpectedCert;
} dg_registry_accessor;

#define RESOLVE_PTR( InSpecialWordsArticle , DataPtr ) \
	((InSpecialWordsArticle).IsPointer ? *(void **)(DataPtr) : (void *)(DataPtr))

[[nodiscard]] static inline bool dg_registry_accessor_validity(const dg_registry_accessor* Accessor) {
	if ( DG_UNLIKELY(Accessor == nullptr || Accessor->Pair == nullptr )) return false;

	return (
		( atomic_load_explicit( (dg_atomic_serial_ptr) &Accessor->Pair->Serial, memory_order_acquire) == Accessor->ExpectedSerial ) &&
		( atomic_load_explicit( (dg_atomic_canon_ptr) &Accessor->Pair->Cert, memory_order_acquire) == Accessor->ExpectedCert )
	);
}

#define DG_RSM_OFFSET_ACCESS( type , varobj, subsequorfield ) \
	( type )( varobj->Memory + varobj->Header.subsequorfield)

static inline const dg_canon* dg_rsm_certs_c(const dg_object* Object) {
	// return (const dg_canon*)(Object->Memory + Object->Header.CertsArrayOffset);
	return DG_RSM_OFFSET_ACCESS( const dg_canon* , Object, CertsArrayOffset);
}

static inline const dg_special_inwords* dg_rsm_offsets_c(const dg_object* Object) {
	// return (const dg_special_inwords*)(Object->Memory + Object->Header.AheadOffsetsArrayOffset);
	return DG_RSM_OFFSET_ACCESS( const dg_special_inwords* , Object , AheadOffsetsArrayOffset );
}

static inline const dg_byte* dg_rsm_hashtable_c(const dg_object* Object) {
	// 8-bit slots: each holds a cert index (0..ItemCount-1), 0xFF = empty. Caps ItemCount at 255.
	return DG_RSM_OFFSET_ACCESS( const dg_byte* , Object , HashTableOffset );
}

static inline dg_byte* dg_rsm_payload(dg_object* Object) {
	// return (dg_byte*)(Object->Memory + Object->Header.PayloadOffset);
	return DG_RSM_OFFSET_ACCESS( dg_byte* , Object , PayloadOffset);
}

#undef DG_RSM_OFFSET_ACCESS

constexpr dg_byte DG_RSM_EMPTY_BUCKET = 0xFFu;

static inline dg_i32 dg_rsm_cert_index(const dg_object* Obj, dg_canon Cert) {
	// Capacity is the stored POT bucket count (see dg_subsequor.HashTableCapacity).
	dg_u32 Cap  = (dg_u32)Obj->Header.HashTableCapacity;

#ifndef NDEBUG
	// 8-bit hash slots cap ItemCount at 255 (0xFF is the empty sentinel), so the
	// largest POT bucket count is 256 and the Mask is at most 255.
	// The linear probe also EXPECTS a power-of-two capacity (MASK = CAP - 1).
#define DG_RSM_CAP_IS_VALID ( Cap != 0u && ( Cap & ( Cap - 1u ) ) == 0u && Cap <= 256u )
	if ( !( DG_RSM_CAP_IS_VALID ) ) {
		__builtin_trap();
	}
#undef DG_RSM_CAP_IS_VALID
#endif

	dg_u32 Mask = Cap - 1u;

	const dg_u8* HashTable  = dg_rsm_hashtable_c(Obj);
	const dg_canon* CertsArray = dg_rsm_certs_c(Obj);

	DG_HASH_MM3_LINEAR_PROBE(Cert, Cap, Mask, SlotIdx) {
		dg_u8 Slot = HashTable[SlotIdx];

		if (Slot == DG_RSM_EMPTY_BUCKET) {
			return -1;
		}
		if (CertsArray[Slot] == Cert) {
			return (dg_i32)Slot;
		}
	}
	return -1;
}

static inline void* dg_rsm_get(dg_object* Obj, dg_canon Cert) {
	dg_i32 Slot = dg_rsm_cert_index(Obj, Cert);
	if (DG_UNLIKELY(Slot < 0)) {
		return nullptr;
	}
	dg_special_inwords ItemOffset = dg_rsm_offsets_c(Obj)[Slot];
	// O(1) memory jump: Base Payload Address + Offset for this specific slot
	void* DataPtr = dg_rsm_payload(Obj) + ((dg_u64)ItemOffset.Value * 8u);
	if (ItemOffset.IsPointer) {
		return *(void**)DataPtr;
	} else {
		return DataPtr;
	}
}

#define DG_RSM_GET(Obj, Cert, Type) ((Type*)dg_rsm_get((Obj), (Cert)))

[[nodiscard]] static inline dg_errcode dg_rsm_mutate_to_pointer(dg_object* Obj, dg_canon Cert, void* ExternalPtr) {
	dg_i32 Slot = dg_rsm_cert_index(Obj, Cert);
	if (DG_UNLIKELY(Slot < 0)) {
		return DGE_RSM_CERT_IS_NOT_CURRENTLY_CONTEXTED;
	}

	dg_special_inwords* OffsetPtr = (dg_special_inwords*)&dg_rsm_offsets_c(Obj)[Slot];
	_Atomic(dg_u32)* AtomicRaw = (_Atomic(dg_u32)*)&OffsetPtr->Raw;

	dg_special_inwords CurrentOffset;
	CurrentOffset.Raw = atomic_load_explicit(
		AtomicRaw,
		memory_order_acquire
	);

	// Retrieve actual payload space and atomically store the pointer
	void* DataArea = dg_rsm_payload(Obj) + ((dg_u64)CurrentOffset.Value * 8u);
	atomic_store_explicit(
		(_Atomic(void*)*)DataArea,
		ExternalPtr,
		memory_order_release
	);

	// If it was raw data, atomically flip the IsPointer bit without corrupting the Value offset
	if (!CurrentOffset.IsPointer) {
		dg_special_inwords NewOffset = CurrentOffset;
		NewOffset.IsPointer = true;

		while (!atomic_compare_exchange_weak_explicit(
			AtomicRaw,
			&CurrentOffset.Raw,
			NewOffset.Raw,
			memory_order_release,
			memory_order_relaxed
		)) {
			// If CAS fails, CurrentOffset.Raw is updated. Check if another thread already flipped it.
			if (CurrentOffset.IsPointer) {
				break;
			}
			NewOffset.Value = CurrentOffset.Value;
		}
	}

	return DGE_RSM_NONE;
}

[[nodiscard]] static inline dg_errcode dg_registrar_init(dg_registrar* OutPtr, dg_allocator* Alloc, dg_u32 Exponent) {
	if (DG_UNLIKELY(OutPtr == nullptr)) {
		return DGE_RSM_REGISTRAR_NULL_PTR;
	}
	if (DG_UNLIKELY(Alloc == nullptr)) {
		return DGE_RSM_ALLOCATOR_NULL_PTR;
	}
	if ( Exponent == 0 ) {
		return DGE_RSM_SIZE_IS_ZERO;
	}
	if ( Exponent > 31u) {
		return DGE_RSM_SIZE_PROVIDED_HAS_31b_OVERFLOW;
	}
	dg_u32 Capacity = 1u << Exponent;
	dg_u64 MapBytes  = (dg_u64)Capacity * sizeof(dg_atomic_certpair_ptr);
	dg_u64 PoolBytes = (dg_u64)Capacity * sizeof(dg_certpairs);

	dg_byte* CertPoolMemory = nullptr;
	dg_errcode E = dg_alloc_zeroed(Alloc, PoolBytes, alignof(dg_certpairs), (void**)&CertPoolMemory);
	if ( !dg_error_ok(E) ) {
		return E;
	}
	dg_atomic_certpair_ptr* SerialMapMemory = nullptr;
	E = dg_alloc_zeroed(Alloc, MapBytes, alignof(dg_atomic_certpair_ptr), (void**)&SerialMapMemory);
	if ( !dg_error_ok(E) ) {
		return E;
	}
	dg_atomic_certpair_ptr* CertMapMemory = nullptr;
	E = dg_alloc_zeroed(Alloc, MapBytes, alignof(dg_atomic_certpair_ptr), (void**)&CertMapMemory);
	if ( !dg_error_ok(E) ) {
		return E;
	}
	*OutPtr = (dg_registrar){
		.Alloc         = *Alloc,
		.TableCapacity = Capacity,
		.TableMask     = Capacity - 1u,
		.MapBySerial   = SerialMapMemory,
		.MapByCert     = CertMapMemory
	};

	E = dg_pool_fixed_init(&OutPtr->CertPool, sizeof(dg_certpairs), &OutPtr->BaseSlab, Capacity, CertPoolMemory);
	if ( !dg_error_ok(E) ) return E;

	return DGE_RSM_NONE;
}

// Words must be provided as High bit set zero.
[[nodiscard]] static inline dg_errcode dg_registrar_enroll( dg_registrar* Reg, dg_u16 Secret, dg_serial Value, dg_inwords Words, dg_registry_accessor* OutAccessor) {
	if (DG_UNLIKELY(Reg == nullptr)) {
		return DGE_RSM_REGISTRAR_NULL_PTR;
	}
	if (DG_UNLIKELY(Value == 0)) {
		return DGE_RSM_NOT_VALID;
	}
	if (DG_UNLIKELY(Words > 0x7FFFFFFFu)) {
		return DGE_RSM_SIZE_PROVIDED_HAS_31b_OVERFLOW;
	}

	dg_u64 SaltedValue = Value ^ ((dg_u64) Secret << 32u ) ^ ((dg_u64) Secret);

	dg_canon FCert = (dg_canon) dg_hash_mm3_fmix64( SaltedValue );

	dg_certpairs* RegisteredEntry = nullptr;

	DG_HASH_MM3_LINEAR_PROBE(Value, Reg->TableCapacity, Reg->TableMask, SlotIdx) {
		dg_certpairs* RealEntry = atomic_load_explicit(
			&Reg->MapBySerial[SlotIdx],
			memory_order_acquire
		);

#define ENTRY_IS_NEW RealEntry == nullptr
#define ENTRY_IS_COPY RealEntry->Serial == Value

		if ( ENTRY_IS_NEW ) {
			dg_certpairs* New = nullptr;

			//
			//Alloc
			for (dg_errcode E = dg_pool_fixed_alloc(&Reg->CertPool, (void**)&New); !dg_error_ok(E); ) {
				if ( !dg_error_acopy_no_domain( E, DGE_RAII_OUT_OF_MEMORY ) ) {
					return E;
				}

				//ALOC is not used here since Pool is RAII Specefic.


				dg_u64 BackingBytes = (dg_u64)Reg->TableCapacity * sizeof(dg_certpairs);
				dg_u64 GrowBytes    = sizeof(dg_pool_slab) + BackingBytes;
				dg_byte* Block      = nullptr;

				dg_errcode_64 GrowErr = dg_alloc_zeroed(&Reg->Alloc, GrowBytes, alignof(dg_pool_slab), (void**)&Block);
				if ( !dg_error_ok(GrowErr) ) {
					return GrowErr; // Bubbles up DGE_ALOC_OUT_OF_MEMORY, DGE_ALOC_NULLCTX, etc.
				}

				dg_errcode_64 ChainErr = dg_pool_fixed_chain(&Reg->CertPool, (dg_pool_slab*)Block, Reg->TableCapacity, Block + sizeof(dg_pool_slab));
				if ( !dg_error_ok(ChainErr) ) {
					// We allocated the block, but chaining failed. Free the orphaned block to prevent a memory leak!
					dg_release(&Reg->Alloc, GrowBytes, Block);
					return ChainErr;
				}

				// The pool successfully expanded. Retry the allocation.
				dg_errcode_64 RetryErr = dg_pool_fixed_alloc(&Reg->CertPool, (void**)&New);
				if ( !dg_error_ok(RetryErr) ) {
					return RetryErr;
				}

				E = DGE_ALOC_NONE;
			}

			//
			//Alloc End

			*New = ((dg_certpairs){
				.Serial = Value,
				.Cert = FCert,
				.Secret = Secret,
				.StructWidth = { .Value = Words }

			});

			dg_certpairs* ExpectedNull = nullptr;

			bool ClaimedSlot = atomic_compare_exchange_strong_explicit(
				&Reg->MapBySerial[SlotIdx],
				&ExpectedNull,
				New,
				memory_order_release,
				memory_order_relaxed
			);

			if ( ClaimedSlot ) {
				RegisteredEntry = New;
				break; // Move on and leave loop.
			} else {
				// Failed CAS. Free NewEntry and let the loop evaluate ExpectedNull.
				// dg_pool_free(&Registrar->CertPool, NewEntry);
				for (dg_errcode E = dg_pool_fixed_free_ptr(&Reg->CertPool, New); !dg_error_ok(E);) {
					return E;
				}

				RealEntry = ExpectedNull;
				_Attempt_SlotIdx = _Attempt_SlotIdx - 1;
				continue;
			}

		}
		else if ( ENTRY_IS_COPY ) {
			if( RealEntry->Secret == Secret) {
				RegisteredEntry = RealEntry;
				break;
			}
			return DGE_RSM_SERIAL_ALREADY_REGISTERED;
		}
	}//for- hash

	if ( RegisteredEntry == nullptr ) {
		return DGE_RSM_REGISTRAR_SUPPOSED_FULL;
	}

	#undef ENTRY_IS_NEW
	#undef ENTRY_IS_COPY
	#define ENTRY_IS_NEW RealCertEntry == nullptr
	#define ENTRY_IS_COPY RealCertEntry->Cert == FCert


	DG_HASH_MM3_LINEAR_PROBE(FCert, Reg->TableCapacity, Reg->TableMask, SlotIdx) {

		dg_certpairs* RealCertEntry = atomic_load_explicit(
			&Reg->MapByCert[SlotIdx],
			memory_order_acquire
		);

		if ( ENTRY_IS_NEW ) {
			dg_certpairs* ExpectedNull = nullptr;

			bool ClaimedSlot = atomic_compare_exchange_strong_explicit(
				&Reg->MapByCert[SlotIdx],
				&ExpectedNull,
				RegisteredEntry,
				memory_order_release,
				memory_order_relaxed
			);

			if ( ClaimedSlot ) {
				break; //out of loop
			} else {
				RealCertEntry =  ExpectedNull;
				_Attempt_SlotIdx = _Attempt_SlotIdx - 1;
				continue;
			}

		}
		else if ( ENTRY_IS_COPY ) {
			break; // Done already
		}
	}

	if (OutAccessor != nullptr) {
		OutAccessor->Pair           = RegisteredEntry;
		OutAccessor->ExpectedSerial = Value;
		OutAccessor->ExpectedCert   = FCert;
	}

	return DGE_RSM_NONE;

	#undef ENTRY_IS_NEW
	#undef ENTRY_IS_COPY
}

[[nodiscard]] static inline const dg_certpairs* dg_registrar_lookup_by_cert( const dg_registrar* Registrar, dg_canon TargetCert ) {
	if (DG_UNLIKELY(Registrar == nullptr)) return nullptr;

	DG_HASH_MM3_LINEAR_PROBE(TargetCert, Registrar->TableCapacity, Registrar->TableMask, ProbeIndex) {

		dg_certpairs* FoundEntry = atomic_load_explicit(
			(dg_atomic_certpair_ptr*)&Registrar->MapByCert[ProbeIndex],
			memory_order_acquire
		);

		// If we hit a null bucket, the Cert was never registered.
		if (FoundEntry == nullptr) {
			return nullptr;
		}

		// Because of hash-bucket collisions, we must verify the actual 64-bit value
		if (FoundEntry->Cert == TargetCert) {
			return FoundEntry;
		}
	}

	return nullptr;
}

//
//
// Object

[[nodiscard]] static inline dg_u16 dg_rsm_get_item_count(const dg_object* Obj) {
	if ( Obj == nullptr ) {
		return 0u;
	} else {
		return Obj->Header.ItemCount;
	}
}

[[nodiscard]] static inline dg_canon dg_rsm_get_cert_at(const dg_object* Obj, dg_u16 Index) {
	if (
		DG_UNLIKELY(
			Obj == nullptr ||
			Index >= Obj->Header.ItemCount
		)
	) {
		return 0;
	} else {
		return dg_rsm_certs_c(Obj)[Index];
	}
}

[[nodiscard]] static inline void* dg_rsm_get_payload_at(dg_object* Obj, dg_u16 Index) {
	if (
		DG_UNLIKELY(
			Obj == nullptr ||
			Index >= Obj->Header.ItemCount
		)
	) {
		return nullptr;
	}

	dg_special_inwords ItemOffset = dg_rsm_offsets_c(Obj)[Index];
	void* DataPtr = dg_rsm_payload(Obj) + ((dg_u64)ItemOffset.Value * 8u);
	if (ItemOffset.IsPointer ) {
		return *(void**)DataPtr;
	} else {
		return DataPtr;
	}
}


[[nodiscard]] dg_errcode dg_rsm_object_create( dg_allocator* Alloc, const dg_registrar* Reg, dg_u16 ItemCount, const dg_canon* RequiredCerts, dg_canon SelfCert, dg_object** OutObj, const bool* IsPointerOverrides ) {

	// This is done so if it does short circut is the cold path for sure and doesn't waste cycles having to do a if chain.
	if ( DG_UNLIKELY(Alloc == nullptr || Reg == nullptr || OutObj == nullptr || RequiredCerts == nullptr) ) {
		if (  Alloc == nullptr ) {
			return DGE_RSM_ALLOCATOR_NULL_PTR;
		}
		if ( Reg == nullptr ) {
			return DGE_RSM_REGISTRAR_NULL_PTR;
		}
		if (  OutObj == nullptr ) {
			return DGE_RSM_OBJECT_NULL_PTR;
		}
		if ( RequiredCerts == nullptr ) {
			return DGE_RSM_CERTS_NULL_PTR;
		}
	}
	if( ItemCount == 0 ) {
		return DGE_RSM_SIZE_IS_ZERO;
	}
	// 8-bit hash slots store a cert index; 0xFF is the empty sentinel, so 255 is the ceiling.
	if ( ItemCount > 255u ) {
		return DGE_RSM_ITEM_COUNT_EXCEEDS_255;
	}

	dg_u32 Capacity = 4u;
	while ( Capacity < (dg_u32)ItemCount ) {
		Capacity = Capacity << 1u;
	}
	dg_u32 HashMask = Capacity - 1u;

	//This is 64b because width is in words and is 32b.
	dg_inwords TotalPayloadWords = 0;
	const dg_certpairs* Pairs[ItemCount];

	for ( dg_u16 idx = 0; idx < ItemCount; idx = idx + 1) {
		Pairs[idx] = dg_registrar_lookup_by_cert( Reg , RequiredCerts[idx] );
		if ( Pairs[idx] == nullptr ) {
			return DGE_RSM_CERT_IS_NOT_CURRENTLY_CONTEXTED;
		}
		// Overrides are optional: nullptr falls back to the cert's registered default.
		bool IsPtr;
		if ( IsPointerOverrides ) {
			IsPtr = IsPointerOverrides[idx];
		} else {
			IsPtr = Pairs[idx]->StructWidth.IsPointer;
		}

		dg_inwords WordWidth;
		if ( IsPtr ) {
			WordWidth = DG_AS_WORDS(sizeof(dg_ptr));
		} else {
			WordWidth = Pairs[idx]->StructWidth.Value;
		}

		TotalPayloadWords = TotalPayloadWords + WordWidth;
	}
	dg_inwords16 CertWordsRequired = DG_AS_WORDS(ItemCount * sizeof(dg_canon));
	dg_inwords16 OffsetWordsRequired = DG_AS_WORDS(ItemCount * sizeof(dg_special_inwords));
	dg_inwords16 HashTableCapacityWordsRequired = DG_AS_WORDS(Capacity * sizeof(dg_byte));

	dg_u64 TotalBytes = sizeof(dg_object) + DG_AS_BYTES(CertWordsRequired + OffsetWordsRequired + HashTableCapacityWordsRequired + TotalPayloadWords);

	dg_object* FObj = nullptr;

	for ( dg_errcode E = dg_alloc_zeroed(Alloc ,TotalBytes , alignof(dg_object) , (void**)&FObj); !dg_error_ok(E); ) {
		return E;
	}

	*FObj = (dg_object){
		.Header = (dg_subsequor) {
			.ItemCount = ItemCount,
			.Cert = SelfCert,
			.CertsArrayOffset = 0,
			.AheadOffsetsArrayOffset = CertWordsRequired,
			.HashTableOffset = CertWordsRequired + OffsetWordsRequired,
			.HashTableCapacity = (dg_u16)Capacity,
			.PayloadOffset = CertWordsRequired + OffsetWordsRequired + HashTableCapacityWordsRequired
		}

	};

	dg_canon* OutCerts = (dg_canon*)dg_rsm_certs_c(FObj);
	dg_special_inwords* OutOffsets = (dg_special_inwords*)dg_rsm_offsets_c(FObj);
	dg_byte* OutHash = (dg_byte*)dg_rsm_hashtable_c(FObj);

	for (dg_u32 idx = 0; idx < Capacity; idx++) {
		OutHash[idx] = DG_RSM_EMPTY_BUCKET;
	}

	dg_u32 CurrentPayloadWordOffset = 0;

	for ( dg_u16 idx = 0; idx < ItemCount; idx = idx + 1) {
		// Recompute the same width decision as the sizing loop so offsets stay in lockstep.
		bool IsPtr;
		if ( IsPointerOverrides ) {
			IsPtr = IsPointerOverrides[idx];
		} else {
			IsPtr = Pairs[idx]->StructWidth.IsPointer;
		}

		dg_inwords WordWidth;
		if ( IsPtr ) {
			WordWidth = DG_AS_WORDS(sizeof(dg_ptr));
		} else {
			WordWidth = Pairs[idx]->StructWidth.Value;
		}

		OutCerts[idx] = RequiredCerts[idx];
		OutOffsets[idx].IsPointer = IsPtr;
		OutOffsets[idx].Value = CurrentPayloadWordOffset; // This is raw payload distance.

		CurrentPayloadWordOffset += WordWidth;

		DG_HASH_MM3_LINEAR_PROBE(RequiredCerts[idx], Capacity, HashMask, SlotIdx) {
			if (OutHash[SlotIdx] == DG_RSM_EMPTY_BUCKET) {
				OutHash[SlotIdx] = (dg_byte)idx; // Store the index into the cert/offset arrays
				break;
			}
		}
	}//for-outter

	*OutObj = FObj;
	return DGE_RSM_NONE;
}

[[nodiscard]] dg_errcode dg_rsm_object_cleanfn(dg_object* Object);

dg_errcode dg_rsm_object_mutate();

#endif
