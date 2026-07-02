//dg_allocator.h
// (C) MigzyENT 2026

#ifndef DG_ALLOCATOR_H
#define DG_ALLOCATOR_H
#pragma once

#if !defined(__cplusplus) && __STDC_VERSION__ < 202311L
#error "This file requires C23 or later. (dg_allocator)"
#endif

//
//
// Notice:
// This file has NO NULL or ERROR checking, the compilance of generation is on the library to implement said features and handle error codes by it self. If you need reference of how to do it please refer to <dg_raii.h> for a arena implementation. - MigzyENT

#include <dg_typedef.h>
#include <dg_error.h>

constexpr dg_u32 DG_ALOC_DWORD_NAME = 0x414C4F43; // 'ALOC'

DG_ERRCODE_TABLE_GENERATE(
	DGE_ALOC, _IDX, _COUNT, DG_ALOC_DWORD_NAME,
	ERROR,
	FAILURE,
	NULLCTX,
	NO_TARGET_SATISFIES,
	NULL_OUT_PTR,
	OUT_OF_MEMORY,
	SIZE_OVERFLOW,
	NULL_OLD_PTR,
	ALIGNMENT_FAULT
)

DG_FN(dg_errcode_64, dg_allocator_alloc_fn,      void* Ctx, dg_inbytes Bytes, dg_u32 Align, void** OutPtr);
DG_FN(dg_errcode_64, dg_allocator_alloczeroed_fn,void* Ctx, dg_inbytes Bytes, dg_u32 Align, void** OutPtr);
DG_FN(dg_errcode_64, dg_allocator_realloc_fn,    void* Ctx, dg_inbytes BytesOld, void* OldPtr, dg_inbytes BytesNew, void** OutPtr);
DG_FN(dg_errcode_64, dg_allocator_free_fn,       void* Ctx, dg_inbytes Bytes, void* MemPtr);
DG_FN(dg_errcode_64, dg_allocator_reset_fn,      void* Ctx);

typedef struct dg_allocator {
	void* Ctx;
	dg_allocator_alloc_fn
		Alloc;
	dg_allocator_alloczeroed_fn
		AllocZero;
	dg_allocator_realloc_fn
		Realloc;
	dg_allocator_free_fn
		Free;
	dg_allocator_reset_fn
		Reset;
} dg_allocator;

[[nodiscard]] static inline dg_errcode_64 dg_alloc( dg_allocator* A, dg_inbytes Bytes, dg_u32 Align, void** OutPtr ) {
	return A->Alloc( A->Ctx, Bytes, Align, OutPtr );
}

[[nodiscard]] static inline dg_errcode_64 dg_alloc_zeroed( dg_allocator* A, dg_inbytes Bytes, dg_u32 Align, void** OutPtr ) {
	return A->AllocZero( A->Ctx, Bytes, Align, OutPtr );
}

// null OldPtr is a fresh allocation (realloc(NULL, ...) semantics).
[[nodiscard]] static inline dg_errcode_64 dg_realloc( dg_allocator* A, dg_inbytes BytesOld, void* OldPtr, dg_inbytes BytesNew, void** OutPtr ) {
	return A->Realloc( A->Ctx, BytesOld, OldPtr, BytesNew, OutPtr );
}

// freeing null is a valid no-op on most backends.
[[nodiscard]] static inline dg_errcode_64 dg_free( dg_allocator* A, dg_inbytes Bytes, void* MemPtr ) {
	return A->Free( A->Ctx, Bytes, MemPtr );
}

[[nodiscard]] static inline dg_errcode_64 dg_reset( dg_allocator* A ) {
	return A->Reset( A->Ctx );
}

// Calls Free then Reset regardless of backend. Always safe — one will be a no-op.
// MemPtr may be null. Errors from both calls are discarded by design.
static inline void dg_release( dg_allocator* A, dg_inbytes Bytes, void* MemPtr ) {
	DGE_ERROR_IGNORE(dg_free( A, Bytes, MemPtr ));
	DGE_ERROR_IGNORE(dg_reset( A ));
}

[[nodiscard]] static inline dg_allocator dg_allocator_generate(
	void*                       Ctx,
	dg_allocator_alloc_fn       Alloc,
	dg_allocator_alloczeroed_fn AllocZero,
	dg_allocator_realloc_fn     Realloc,
	dg_allocator_free_fn        Free,
	dg_allocator_reset_fn       Reset
) {
	return (dg_allocator){
		.Ctx       = Ctx,
		.Alloc     = Alloc,
		.AllocZero = AllocZero,
		.Realloc   = Realloc,
		.Free      = Free,
		.Reset     = Reset,
	};
}

//
//
// Library-agnostic wrappers — return long long (0 = ok, non-zero encodes domain + error).
// Use these when the caller doesn't know about dg_errcode_64.

[[nodiscard]] static inline long long dg_alloc_ll( dg_allocator* A, dg_inbytes Bytes, dg_u32 Align, void** OutPtr ) {
	dg_errcode_64 E = dg_alloc( A, Bytes, Align, OutPtr );
	DG_ALIAS_SIDESTEP(E, long long, Ell);
	return Ell;
}

[[nodiscard]] static inline long long dg_alloc_zeroed_ll( dg_allocator* A, dg_inbytes Bytes, dg_u32 Align, void** OutPtr ) {
	dg_errcode_64 E = dg_alloc_zeroed( A, Bytes, Align, OutPtr );
	DG_ALIAS_SIDESTEP(E, long long, Ell);
	return Ell;
}

[[nodiscard]] static inline long long dg_realloc_ll( dg_allocator* A, dg_inbytes BytesOld, void* OldPtr, dg_inbytes BytesNew, void** OutPtr ) {
	dg_errcode_64 E = dg_realloc( A, BytesOld, OldPtr, BytesNew, OutPtr );
	DG_ALIAS_SIDESTEP(E, long long, Ell);
	return Ell;
}

[[nodiscard]] static inline long long dg_free_ll( dg_allocator* A, dg_inbytes Bytes, void* MemPtr ) {
	dg_errcode_64 E = dg_free( A, Bytes, MemPtr );
	DG_ALIAS_SIDESTEP(E, long long, Ell);
	return Ell;
}

[[nodiscard]] static inline long long dg_reset_ll( dg_allocator* A ) {
	dg_errcode_64 E = dg_reset( A );
	DG_ALIAS_SIDESTEP(E, long long, Ell);
	return Ell;
}

// dg_ctx — flat dual-allocator context.
// Always pass &Ctx.Perm or &Ctx.Temp explicitly to functions taking dg_allocator*.
// e.g: dg_free( &Ctx.Perm, Bytes, Ptr )
//      dg_alloc( &Ctx.Temp, Bytes, Align, &OutPtr )
typedef struct dg_ctx {
	dg_allocator Perm;
	dg_allocator Temp;
} dg_ctx;

//
//
// TODO(0.500.750): Allocator capability bitmask.
//   Gate advanced memory patterns behind a capability field fixed at construction:
//   io_uring-backed IO, mmap / hugepage regions, NUMA pinning, etc.
//   dg_allocator_generate() would take a requested-caps mask and validate it against
//   what the backend actually implements. An allocator not built to spec for a given
//   pattern cannot be handed to code that requires it — the mismatch fails at wire-up,
//   not at point of use. Consumers query caps before opting into a pattern.
//   (Domain-mixed error returns are fine and stay — the DWord tag disambiguates.)
//
// Standardize return errors.
//
// so my guess is create a logical nodes:
//
// logical_alloc_vtable <- bigs starter - normalist and specialist when logicallys challenged into becoming alloc_vtable.
//
// alloc_vtable <- profiencent allocator
//
// a bitmask u64 with first bit being logical-status and rest being common patterns.
//
//isLogical : 1
//VariantIdx : 12 //allows a ENUM to change bunchs
//: ... // rest is flags
//

#endif //DG_ALLOCATOR_H
