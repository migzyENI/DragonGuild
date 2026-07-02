//dg_typedef
// (C) MigzyENT 2026

#ifndef DG_TYPEDEF_H
#define DG_TYPEDEF_H
#pragma once

#if !defined(__cplusplus) && __STDC_VERSION__ < 202311L
	#error "This file requires C23 or later. (dg_typedef)"
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>


#if defined(__x86_64__)
	#include <immintrin.h>
#elif defined(__aarch64__)
	#include <arm_neon.h>
#endif

//
//
// Platform detection

#if defined(__unix__) || defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	#define DG_UNIX
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(__REACTOS__)
	#define DG_WINDOWS
#endif

#if defined (DG_UNIX)
#if defined (DG_WINDOWS)
#error
#endif
#endif



//
//
// Branch prediction hints

#define DG_LIKELY(x)   __builtin_expect(!!(x), 1)
#define DG_UNLIKELY(x) __builtin_expect(!!(x), 0)

//
//
// Type declarations

_Static_assert(sizeof(void*) == 8u && alignof(void*) == 8u, "64-bit only");

typedef uint32_t dg_u32;
typedef int32_t  dg_i32;
typedef float    dg_f32;
typedef uint64_t dg_u64;
typedef int64_t  dg_i64;
typedef double   dg_d64;

typedef _Atomic(uint64_t) dg_atomic_u64;
typedef _Atomic(uint32_t) dg_atomic_u32;

typedef uint8_t  dg_u8;  //not preferred, use for special math otherwise use bytes for data.
typedef uint16_t dg_u16; //not preferred, use for efficeny
typedef int16_t dg_i16;

typedef uint8_t dg_byte; // This semantically is more strict.
typedef dg_u64  dg_inbytes;
typedef dg_u32  dg_inbytes32;
typedef dg_u16  dg_inwords16; // From RSM
typedef dg_u32 dg_inwords32;
typedef dg_inwords32  dg_inwords;

typedef uintptr_t dg_ptr;

// Double Word ( 32-bit )
typedef union dg_dword {
	dg_u32 u32;
	dg_i32 i32;
	dg_f32 f32;
	dg_u16 u16[2];
	dg_i16 i16[2];
	dg_byte byte[4];
	dg_u8 u8[4];
} dg_dword;


// Quad Word (64-bit)
typedef union dg_qword {
	dg_u64  u64;
	dg_i64  i64;
	dg_d64  d64;
	void*   Ptr;
	dg_u32  u32[2];
	dg_i32  i32[2];
	dg_f32  f32[2];
	dg_u16 u16[4];
	dg_i16 i16[4];
	dg_byte byte[8];
	dg_u8 u8[8];
} __attribute__((aligned(8))) dg_qword;

// Ultra Quad Word (128-bit)
typedef union dg_uqword {
	#if defined(__x86_64__)
		__m128i i128;
		__m128  f128;
		__m128d d128;
	#elif defined(__aarch64__)
		int32x4_t   i128;
		float32x4_t f128;
	#endif

		dg_u64  u64[2];
		dg_i64  i64[2];
		dg_d64  d64[2];
		void*   Ptr[2];
		dg_u32  u32[4];
		dg_i32  i32[4];
		dg_f32  f32[4];
		dg_u16 u16[8];
		dg_i16 i16[8];
		dg_byte byte[16];
		dg_u8 u8[16];
} __attribute__((aligned(16))) dg_uqword;

// Extra Word (256-bit)
typedef union dg_xword {
	#if defined(__AVX2__)
		__m256i i256;
		__m256  f256;
		__m256d d256;
	#elif defined(__x86_64__)
		__m128i i128[2];
		__m128  f128[2];
		__m128d d128[2];
	#endif
	dg_u64  u64[4];
	dg_i64  i64[4];
	dg_d64  d64[4];
	void*   Ptr[4];
	dg_u32  u32[8];
	dg_i32  i32[8];
	dg_f32  f32[8];
	dg_u16 u16[16];
	dg_i16 i16[16];
	dg_byte byte[32];
	dg_u8 u8[32];
} __attribute__((aligned(32))) dg_xword;

// Ultra Extra Word (512-bit)
typedef union dg_uqxword {
	#if defined(__AVX512F__)
		__m512i i512;
		__m512  f512;
		__m512d d512;
	#elif defined(__AVX2__)
		__m256i i256[2];
		__m256  f256[2];
		__m256d d256[2];
	#elif defined(__x86_64__)
		__m128i i128[4];
		__m128  f128[4];
		__m128d d128[4];
	#endif
	dg_u64  u64[8];
	dg_i64  i64[8];
	dg_d64  d64[8];
	void*   Ptr[8]; //Doesn't work on 128bits fix later at 0.500.250
	dg_u32  u32[16];
	dg_i32  i32[16];
	dg_f32  f32[16];
	dg_u16 u16[32];
	dg_i16 i16[32];
	dg_byte byte[64];
	dg_u8 u8[64];
} __attribute__((aligned(64))) dg_uqxword;

typedef struct dg_string {
	dg_u64 Length;
	char*  Char;
} dg_string;

typedef struct dg_defined_memory {
	dg_u32 Length;
	bool IsPtr;
	dg_ptr* Memory;
} dg_defined_memory;

//
//
// Function Primitives


typedef void (*dg_fn_ptr)(void);
#define DG_FN(ReturnType, FunctionTypeName, ...) typedef ReturnType (*FunctionTypeName)(__VA_ARGS__)
	// DG_FN(void, MyFn, int X) -> typedef void (*MyFn)(int X)
#define DG_FN_ADDRESS(FunctionName) (&(FunctionName))
	// explicit address-of a function
#define DG_FN_STORE(FunctionName) ((dg_fn_ptr)(FunctionName))
	// cast concrete fn ptr -> opaque slot
#define DG_FN_LOAD(FunctionPtrTypeHandle, OpaqueSlot) ((FunctionPtrTypeHandle)(OpaqueSlot))
	// cast opaque slot -> concrete fn ptr, then call
#define DG_FN_VALID(FunctionPtrOrSlot) ((FunctionPtrOrSlot) != nullptr)
	// null check

typedef struct dg_virtual {
	const dg_fn_ptr* Slots;
	dg_u32           Count;
} dg_virtual;

#define DG_VIRTUAL_BIND(StaticArray, SlotCount) ((dg_virtual){ .Slots = (StaticArray), .Count = (SlotCount) })

#define DG_VIRTUAL_VALID(VTablePtr, SlotIndex) \
((VTablePtr) != nullptr && (VTablePtr)->Slots != nullptr && (SlotIndex) < (VTablePtr)->Count && DG_FN_VALID((VTablePtr)->Slots[(SlotIndex)]))

#define DG_VIRTUAL_CALL(FunctionPtrTypeHandle, VTablePtr, SlotIndex, ...) \
DG_FN_LOAD(FunctionPtrTypeHandle, (VTablePtr)->Slots[(SlotIndex)])(__VA_ARGS__)


#define DG_LOAD_AS(Type, ptr) __extension__ ({ \
	Type _v; \
	__builtin_memcpy(&_v, (ptr), sizeof(Type)); \
	_v; \
})


//
//
// UB prevention in LTO

#define DG_ALIAS_SIDESTEP(ParentVar, AdjacentAliasType, ResultName) \
	AdjacentAliasType ResultName; \
	__builtin_memcpy(&ResultName, &(ParentVar) , sizeof((ParentVar))) \

#define DG_AS_WORDS( VarBytesCnt ) \
	( ((VarBytesCnt) + 7u) / 8u )

//Might need generic for having casts from unique or standard types.
#define DG_AS_BYTES( VarWordsCnt ) \
	( ((VarWordsCnt) * 8u) )


#define DG_PO2_RETURN_SIZET( Capacitiy , Type_64bit) \
	Capacitiy <= 1 ? 1 : (Type_64bit)1 << (64 - __builtin_clzll((Type_64bit)Capacitiy - 1));

// Rounds up to the next power of two. No trailing ';' (usable in expression position).
// Guards the shift: Capacity > 2^31 has no representable po2 in 32 bits, so it clamps
// to 2^31 instead of doing UB ( 1u << 32 ). 0 and 1 both map to 1.
#define DG_PO2_RETURN_32b( Capacitiy, Type_32bit ) \
	( (Capacitiy) <= 1u \
	? (Type_32bit)1u \
	: ( (dg_u32)(Capacitiy) > 0x80000000u \
		? (Type_32bit)0x80000000u \
		: (Type_32bit)( (dg_u32)1u << ( 32u - (dg_u32)__builtin_clz( (dg_u32)(Capacitiy) - 1u ) ) ) ) )
/*
#if defined(__clang__) && defined(__has_feature)
	#if __has_feature(nullability)
	#define DG_NONNULL   _Nonnull
	#define DG_NULLABLE  _Nullable
	#endif
#endif
#ifndef DG_NONNULL
	#define DG_NONNULL
	#define DG_NULLABLE
#endif

#if defined(__GNUC__) && !defined(__clang__)
	#define DG_NONNULL_ARGS(...)  __attribute__((nonnull(__VA_ARGS__)))
#else
	#define DG_NONNULL_ARGS(...)
#endif
*/

//
//
#endif
