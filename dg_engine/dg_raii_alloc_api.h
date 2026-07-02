//dg_raii_alloc_api.h
// (C) MigzyENT 2026

#ifndef DG_RAII_ALLOC_API_H
#define DG_RAII_ALLOC_API_H
#include "dg_errcode.h"
#pragma once

#if !defined(__cplusplus) && __STDC_VERSION__ < 202311L
#error "This file requires C23 or later. (dg_raii_alloc_api)"
#endif

#include <dg_typedef.h>
#include <dg_error.h>
#include <dg_allocator.h>
#include <dg_raii.h>

#include <string.h>

//
//
// RAII → ALOC error translator
//
// Adapters implement the dg_allocator interface, so callers expect DGE_ALOC_NONE_* errors.
// Arena internals return DGE_RAII_* — this maps them across domains.

static inline dg_errcode_64 dg_raii_to_aloc_( dg_errcode_64 E ) {
	switch ( (DGE_RAII_IDX_e)E.Err ) {
		case DGE_RAII_IDX_NONE:             return DGE_ALOC_NONE;
		case DGE_RAII_IDX_OUT_OF_MEMORY:    return DGE_ALOC_OUT_OF_MEMORY;
		case DGE_RAII_IDX_NULL_ARENA_PTR:   return DGE_ALOC_NULLCTX;
		case DGE_RAII_IDX_NULL_MEMORY_PTR:  return DGE_ALOC_NULLCTX;
		case DGE_RAII_IDX_NULL_OUT_PTR:     return DGE_ALOC_NULL_OUT_PTR;
		case DGE_RAII_IDX_ALIGNMENT_FAULT:  return DGE_ALOC_ALIGNMENT_FAULT;
		case DGE_RAII_IDX_SIZE_OVERFLOW:    return DGE_ALOC_SIZE_OVERFLOW;
		default:                            return DGE_ALOC_ERROR;
	}
}

//
//
// Arena → dg_allocator adapters
//
// The arena backend always aligns to DG_WORD_SIZE (8 bytes).
// The Align parameter is accepted to satisfy the allocator interface but is not honoured;
// callers requiring alignment stricter than 8 bytes must use a different backend.

static dg_errcode_64 dg_arena_alloc_adapter_( void* Ctx, dg_inbytes Bytes, dg_u32 Align, void** OutPtr ) {
	if ( Align != 8u ) {
		return DGE_ALOC_ALIGNMENT_FAULT;
	}

	(void)Align;
	return dg_raii_to_aloc_( dg_arena_alloc( (dg_arena*)Ctx, Bytes, OutPtr ) );
}

static dg_errcode_64 dg_arena_alloc_zeroed_adapter_( void* Ctx, dg_inbytes Bytes, dg_u32 Align, void** OutPtr ) {
	(void)Align;
	return dg_raii_to_aloc_( dg_arena_alloc_zero( (dg_arena*)Ctx, Bytes, OutPtr ) );
}

// Fast path: OldPtr is at the arena top — roll back OldWords, re-bump from the
// same position (same pointer back), zero any growth tail.
// Slow path: alloc new block, copy old data, old block becomes dead space.
static dg_errcode_64 dg_arena_realloc_adapter_( void* Ctx, dg_inbytes BytesOld, void* OldPtr, dg_inbytes BytesNew, void** OutPtr ) {
	dg_arena* Arena = (dg_arena*)Ctx;

	if ( OldPtr != nullptr && BytesOld > 0u ) {
		dg_inwords OldWords = (dg_inwords)(( BytesOld + DG_WORD_ALIGN_MASK ) / DG_WORD_SIZE);
		if ( Arena->Offset >= OldWords &&
			(dg_qword*)OldPtr == &Arena->Memory[ Arena->Offset - OldWords ] ) {
			dg_inwords SavedOffset = Arena->Offset;
			Arena->Offset           = Arena->Offset - OldWords;
			dg_errcode_64 Err        = dg_raii_to_aloc_( dg_arena_alloc( Arena, BytesNew, OutPtr ) );
			if ( DG_UNLIKELY( !dg_error_ok( Err ) ) ) {
				Arena->Offset = SavedOffset;
				return Err;
			}
			if ( BytesNew > BytesOld )
				memset( (dg_byte*)*OutPtr + BytesOld, 0, BytesNew - BytesOld );
			return DGE_ALOC_NONE;
		}
	}

	dg_errcode_64 Err = dg_raii_to_aloc_( dg_arena_alloc( Arena, BytesNew, OutPtr ) );
	if ( DG_UNLIKELY( !dg_error_ok( Err ) ) ) return Err;
	if ( OldPtr != nullptr && BytesOld > 0u )
		memcpy( *OutPtr, OldPtr, BytesOld < BytesNew ? BytesOld : BytesNew );
	if ( BytesNew > BytesOld )
		memset( (dg_byte*)*OutPtr + BytesOld, 0, BytesNew - BytesOld );
	return DGE_ALOC_NONE;
}

// Arena cannot free individual blocks — structural no-op, always succeeds.
static dg_errcode_64 dg_arena_free_noop_( void* Ctx, dg_inbytes Bytes, void* MemPtr ) {
	(void)Ctx; (void)Bytes; (void)MemPtr;
	return DGE_ALOC_NONE;
}

static dg_errcode_64 dg_arena_reset_adapter_( void* Ctx ) {
	dg_arena_reset( (dg_arena*)Ctx );
	return DGE_ALOC_NONE;
}

/**
 * @brief Construct a dg_allocator backed by an arena.
 *
 * The arena must outlive all allocations made through the returned allocator.
 * Free is a no-op on this backend; Reset resets the full arena.
 *
 * @param Arena  Backing arena. Must not be null.
 * @return       dg_allocator wired to the arena.
 *
 * @see dg_arena_init
 * @see dg_allocator
 */
[[nodiscard]] static inline dg_allocator dg_arena_make_allocator( dg_arena* Arena ) {
	return dg_allocator_generate(
		Arena,
		dg_arena_alloc_adapter_,
		dg_arena_alloc_zeroed_adapter_,
		dg_arena_realloc_adapter_,
		dg_arena_free_noop_,
		dg_arena_reset_adapter_
	);
}

#endif //DG_RAII_ALLOC_API_H
