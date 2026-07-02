// dg_hash.h
// (C) MigzyENT 2026
//
// FNV-1a hash implementation — 64 / 128 / 256 / 512 bit.
// citations:
// https://github.com/lcn2/fnv
// http://www.isthe.com/chongo/tech/comp/fnv/index.html

#ifndef DG_HASH_H
#define DG_HASH_H
#include "dg_allocator.h"
#pragma once

#if !defined(__cplusplus) && __STDC_VERSION__ < 202311L
#error "This file requires C23 or later. (dg_hash)"
#endif

#include <dg_typedef.h>
#include <dg_error.h>

typedef __uint128_t dg_u128_arith; // not preferred; only for internal carry arithmetic

//
//
// Error Codes


//TODO This is using old error code style, update to use macro.

constexpr dg_u32 DGE_HASH_DWORD_NAME = 'HASH';

DG_ERRCODE_TABLE_GENERATE(
	DGE_HASH, _IDX, _COUNT, DGE_HASH_DWORD_NAME,
	NULL_DATA,
	NULL_OUT
)

//
//
// FNV-1a Constants

constexpr dg_qword DG_FNV1_PRIME_64BIT         = { .u64 = 0x100000001b3ULL };
constexpr dg_qword DG_FNV1_OFFSET_BASIS_64BIT  = { .u64 = 0xcbf29ce484222325ULL };

// Scalar approximations of the wide primes (lower 64 bits of each prime).
// Used in the multi-word implementations — full wide primes are not needed.
constexpr dg_u64 DG_FNV1_PRIME_128BIT_SCALAR   = 315u;   // 2^8 + 0x3b
constexpr dg_u64 DG_FNV1_PRIME_256BIT_SCALAR   = 355u;   // 2^8 + 0x63
constexpr dg_u64 DG_FNV1_PRIME_512BIT_SCALAR   = 343u;   // 2^8 + 0x57

constexpr dg_uqword DG_FNV1_OFFSET_BASIS_128BIT = {
	.u64 = {
		0x62B821756295C58DULL,   // word[0]
		0x6C62272E07BB0142ULL,   // word[1]
	}
};

constexpr dg_xword DG_FNV1_OFFSET_BASIS_256BIT = {
	.u64 = {
		0x1023B4C8CAEE0535ULL,   // word[0]
		0xC8B1536847B6BBB3ULL,   // word[1]
		0x2D98C384C4E576CCULL,   // word[2]
		0xDD268DBCAAC55036ULL,   // word[3]
	}
};

constexpr dg_uqxword DG_FNV1_OFFSET_BASIS_512BIT = {
	.u64 = {
		0x441F8A7568C3EE4EULL,   // word[0]
		0x104B90E1EB9277B4ULL,   // word[1]
		0xBA29DCD14598E518ULL,   // word[2]
		0x44581F3DFC00A56CULL,   // word[3]
		0xC6F6C2C0DE7E5E88ULL,   // word[4]
		0x33F09BA6D37B6DEAULL,   // word[5]
		0x3E51A1A1F377A354ULL,   // word[6]
		0x83BB5300708A6D8DULL,   // word[7]
	}
};


//
//
// Hash Functions

[[nodiscard]] static inline dg_errcode_64 dg_hash_fnv1a_64 ( const dg_byte* Data, dg_u64 Length, dg_qword* OutHash ) {
	if ( DG_UNLIKELY( Data    == nullptr ) ) return DGE_HASH_NULL_DATA;
	if ( DG_UNLIKELY( OutHash == nullptr ) ) return DGE_HASH_NULL_OUT;
	dg_u64 Hash = DG_FNV1_OFFSET_BASIS_64BIT.u64;
	for ( dg_u64 Xe = 0u; Xe < Length; ++Xe ) {
		Hash = ( Hash ^ (dg_u64)Data[ Xe ] ) * DG_FNV1_PRIME_64BIT.u64;
	}
	OutHash->u64 = Hash;
	return DGE_HASH_NONE;
}

[[nodiscard]] static inline dg_errcode_64 dg_hash_fnv1a_128 ( const dg_byte* Data, dg_u64 Length, dg_uqword* OutHash ) {
	if ( DG_UNLIKELY( Data    == nullptr ) ) return DGE_HASH_NULL_DATA;
	if ( DG_UNLIKELY( OutHash == nullptr ) ) return DGE_HASH_NULL_OUT;
	dg_u64 Part0th = DG_FNV1_OFFSET_BASIS_128BIT.u64[0];
	dg_u64 Part1th = DG_FNV1_OFFSET_BASIS_128BIT.u64[1];
	for ( dg_u64 Xe = 0u; Xe < Length; ++Xe ) {
		Part0th = Part0th ^ (dg_u64)Data[Xe];
		dg_u64 Part1th_Carry        = Part0th << 24u;
		dg_u128_arith Product0      = (dg_u128_arith)Part0th * DG_FNV1_PRIME_128BIT_SCALAR;
		Part0th = (dg_u64)Product0;
		Part1th = ( (dg_u64)(Product0 >> 64u) + ( Part1th * DG_FNV1_PRIME_128BIT_SCALAR ) ) + Part1th_Carry;
	}
	OutHash->u64[0] = Part0th;
	OutHash->u64[1] = Part1th;
	return DGE_HASH_NONE;
}

[[nodiscard]] static inline dg_errcode_64 dg_hash_fnv1a_256 ( const dg_byte* Data, dg_u64 Length, dg_xword* OutHash ) {
	if ( DG_UNLIKELY( Data    == nullptr ) ) return DGE_HASH_NULL_DATA;
	if ( DG_UNLIKELY( OutHash == nullptr ) ) return DGE_HASH_NULL_OUT;
	dg_u64 Part0th = DG_FNV1_OFFSET_BASIS_256BIT.u64[0];
	dg_u64 Part1th = DG_FNV1_OFFSET_BASIS_256BIT.u64[1];
	dg_u64 Part2th = DG_FNV1_OFFSET_BASIS_256BIT.u64[2];
	dg_u64 Part3th = DG_FNV1_OFFSET_BASIS_256BIT.u64[3];
	for ( dg_u64 Xe = 0u; Xe < Length; ++Xe ) {
		Part0th = Part0th ^ (dg_u64)Data[Xe];
		dg_u64 Part2th_Carry   = Part0th << 40u;
		dg_u64 Part3th_Carry   = Part0th >> 24u;
		dg_u128_arith Product0 = (dg_u128_arith)Part0th * DG_FNV1_PRIME_256BIT_SCALAR;
		dg_u128_arith Product1 = (dg_u128_arith)Part1th * DG_FNV1_PRIME_256BIT_SCALAR + (dg_u64)( Product0 >> 64u );
		dg_u128_arith Product2 = (dg_u128_arith)Part2th * DG_FNV1_PRIME_256BIT_SCALAR + (dg_u64)( Product1 >> 64u );
		Part0th = (dg_u64)Product0;
		Part1th = (dg_u64)Product1;
		Part2th = (dg_u64)Product2  + Part2th_Carry;
		Part3th = ( Part3th * DG_FNV1_PRIME_256BIT_SCALAR ) + (dg_u64)( Product2 >> 64u ) + Part3th_Carry;
	}
	OutHash->u64[0] = Part0th;
	OutHash->u64[1] = Part1th;
	OutHash->u64[2] = Part2th;
	OutHash->u64[3] = Part3th;
	return DGE_HASH_NONE;
}

[[nodiscard]] static inline dg_errcode_64 dg_hash_fnv1a_512 ( const dg_byte* Data, dg_u64 Length, dg_uqxword* OutHash ) {
	if ( DG_UNLIKELY( Data    == nullptr ) ) return DGE_HASH_NULL_DATA;
	if ( DG_UNLIKELY( OutHash == nullptr ) ) return DGE_HASH_NULL_OUT;
	dg_u64 Part0th = DG_FNV1_OFFSET_BASIS_512BIT.u64[0];
	dg_u64 Part1th = DG_FNV1_OFFSET_BASIS_512BIT.u64[1];
	dg_u64 Part2th = DG_FNV1_OFFSET_BASIS_512BIT.u64[2];
	dg_u64 Part3th = DG_FNV1_OFFSET_BASIS_512BIT.u64[3];
	dg_u64 Part4th = DG_FNV1_OFFSET_BASIS_512BIT.u64[4];
	dg_u64 Part5th = DG_FNV1_OFFSET_BASIS_512BIT.u64[5];
	dg_u64 Part6th = DG_FNV1_OFFSET_BASIS_512BIT.u64[6];
	dg_u64 Part7th = DG_FNV1_OFFSET_BASIS_512BIT.u64[7];
	for ( dg_u64 Xe = 0u; Xe < Length; ++Xe ) {
		Part0th = Part0th ^ (dg_u64)Data[Xe];
		dg_u64 Part5th_Carry   = Part0th << 24u;
		dg_u64 Part6th_Carry   = Part0th >> 40u;
		dg_u128_arith Product0 = (dg_u128_arith)Part0th * DG_FNV1_PRIME_512BIT_SCALAR;
		dg_u128_arith Product1 = (dg_u128_arith)Part1th * DG_FNV1_PRIME_512BIT_SCALAR + (dg_u64)( Product0 >> 64u );
		dg_u128_arith Product2 = (dg_u128_arith)Part2th * DG_FNV1_PRIME_512BIT_SCALAR + (dg_u64)( Product1 >> 64u );
		dg_u128_arith Product3 = (dg_u128_arith)Part3th * DG_FNV1_PRIME_512BIT_SCALAR + (dg_u64)( Product2 >> 64u );
		dg_u128_arith Product4 = (dg_u128_arith)Part4th * DG_FNV1_PRIME_512BIT_SCALAR + (dg_u64)( Product3 >> 64u );
		dg_u128_arith Product5 = (dg_u128_arith)Part5th * DG_FNV1_PRIME_512BIT_SCALAR + (dg_u64)( Product4 >> 64u );
		dg_u128_arith Product6 = (dg_u128_arith)Part6th * DG_FNV1_PRIME_512BIT_SCALAR + (dg_u64)( Product5 >> 64u );
		Part0th = (dg_u64)Product0;
		Part1th = (dg_u64)Product1;
		Part2th = (dg_u64)Product2;
		Part3th = (dg_u64)Product3;
		Part4th = (dg_u64)Product4;
		Part5th = (dg_u64)Product5 + Part5th_Carry;
		Part6th = (dg_u64)Product6 + Part6th_Carry;
		Part7th = ( Part7th * DG_FNV1_PRIME_512BIT_SCALAR ) + (dg_u64)( Product6 >> 64u );
	}
	OutHash->u64[0] = Part0th;
	OutHash->u64[1] = Part1th;
	OutHash->u64[2] = Part2th;
	OutHash->u64[3] = Part3th;
	OutHash->u64[4] = Part4th;
	OutHash->u64[5] = Part5th;
	OutHash->u64[6] = Part6th;
	OutHash->u64[7] = Part7th;
	return DGE_HASH_NONE;
}


//
//
// Generic Dispatch
// dg_hash_fnv1a( Data, Length, OutHash ) — dispatches on OutHash type.
// Always returns dg_errcode_64.

#if defined(__x86_64__)
#define DG_HASH_GENERIC_SIMD128( Data, Length, OutHash ) \
, __m128i*: dg_hash_fnv1a_128( (Data), (Length), (dg_uqword*)(OutHash) )
#elif defined(__aarch64__)
#define DG_HASH_GENERIC_SIMD128( Data, Length, OutHash ) \
, int32x4_t*: dg_hash_fnv1a_128( (Data), (Length), (dg_uqword*)(OutHash) )
#else
#define DG_HASH_GENERIC_SIMD128( Data, Length, OutHash )
#endif

// Arm 256-bit is part of SVE — hardware runtime check required, not handled here.

#define dg_hash_fnv1a( Data, Length, OutHash ) _Generic( (OutHash),         \
dg_qword*:   dg_hash_fnv1a_64(  (Data), (Length), (dg_qword*)(OutHash) ),    \
dg_uqword*:  dg_hash_fnv1a_128( (Data), (Length), (dg_uqword*)(OutHash) ),   \
dg_xword*:   dg_hash_fnv1a_256( (Data), (Length), (dg_xword*)(OutHash) ),    \
dg_uqxword*: dg_hash_fnv1a_512( (Data), (Length), (dg_uqxword*)(OutHash) )   \
DG_HASH_GENERIC_SIMD128( Data, Length, OutHash )                             \
)


//
//
// Integer Mixers
// Finalizer constants (fmix64) from MurmurHash3 by Austin Appleby (Public Domain).
// C port inspirations adapted from Peter Scott (https://github.com/PeterScott/murmur3).
// Note: _to_32 variant uses a single-pass truncation mix for speed.

constexpr dg_qword MURMUR3_MIX_CONSTANT_1 = { .u64 = 0xFF51AFD7ED558CCDULL };
constexpr dg_qword MURMUR3_MIX_CONSTANT_2 = { .u64 = 0xC4CEB9FE1A85EC53ULL };
constexpr dg_qword MURMUR3_UNMIX_CONSTANT_1 = { .u64 = 0x4f74430c22a54005ULL };
constexpr dg_qword MURMUR3_UNMIX_CONSTANT_2 = { .u64 = 0x9cb4b2f8129337dbULL };

[[nodiscard]] static inline dg_u64 dg_hash_mm3_fmix64( dg_u64 Value ) {
	(Value) = (Value) ^ ((Value) >> 33U);
	(Value) = (Value)*MURMUR3_MIX_CONSTANT_1.u64;
	(Value) = (Value) ^ ((Value) >> 33U);
	Value = Value * MURMUR3_MIX_CONSTANT_2.u64;
	Value = Value ^ (Value >> 33U);
	return Value;
}

[[nodiscard]] static inline dg_u32 dg_hash_mm3_fmix64_to_32( dg_u64 Value ) {
	(Value) = (Value) ^ ((Value) >> 33U);
	(Value) = (Value)*MURMUR3_MIX_CONSTANT_1.u64;
	(Value) = (Value) ^ ((Value) >> 33U);
	return (dg_u32)Value;
}

[[nodiscard]] static inline dg_u64 dg_hash_mm3_fmix64_inverse( dg_u64 Value ) {
	// Forward is SPLIT_1 then SPLIT_2; invert in reverse order.
	Value = Value ^ ( Value >> 33u );
	Value = Value * MURMUR3_UNMIX_CONSTANT_2.u64;
	Value = Value ^ ( Value >> 33u );
	Value = Value * MURMUR3_UNMIX_CONSTANT_1.u64;
	Value = Value ^ ( Value >> 33u );
	return Value;
}

// EXPECTS: CAPACITY is Power of 2, MASK is (CAPACITY - 1).
#define DG_HASH_MM3_LINEAR_PROBE_KEY(HASH_VAL, MASK, SLOT_VAR, ATTEMPT_VAR) \
for (dg_u32 ATTEMPT_VAR##SLOT_VAR = 0, \
	_Base_##SLOT_VAR = dg_hash_mm3_fmix64_to_32((dg_u64)(HASH_VAL)), \
	SLOT_VAR = _Base_##SLOT_VAR & (MASK); \
	ATTEMPT_VAR##SLOT_VAR < ((MASK)+1); \
	++ATTEMPT_VAR##SLOT_VAR, SLOT_VAR = (_Base_##SLOT_VAR + ATTEMPT_VAR##SLOT_VAR) & (MASK))

#define DG_HASH_MM3_LINEAR_PROBE_HASH(HASH_VAL, MASK, SLOT_VAR, ATTEMPT_VAR) \
	for (dg_u32 ATTEMPT_VAR##SLOT_VAR = 0, \
		_Base_##SLOT_VAR = (HASH_VAL), \
		SLOT_VAR = _Base_##SLOT_VAR & (MASK); \
		ATTEMPT_VAR##SLOT_VAR < ((MASK)+1); \
		++ATTEMPT_VAR##SLOT_VAR, SLOT_VAR = (_Base_##SLOT_VAR + ATTEMPT_VAR##SLOT_VAR) & (MASK))


#endif // DG_HASH_H
