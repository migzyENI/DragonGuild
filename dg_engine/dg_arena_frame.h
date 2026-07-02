//dg_arena_frame.h
// (C) MigzyENT

#ifndef DG_ARENA_FRAME_H
#define DG_ARENA_FRAME_H

#pragma once

#if !defined(__cplusplus) && __STDC_VERSION__ < 202311L
#error "This file requires C23 or later."
#endif

#include <dg_typedef.h>
#include <dg_raii.h>

/**
 * @brief Double-buffered arena pair. One arena is written to while the other holds the previous frame's data.
 * @see dg_frame_write_arena
 * @see dg_frame_read_arena
 * @see dg_frame_flip
 */
typedef struct dg_frame_arena_pair {
	dg_atomic_u32 WriteIdx; // 0 | 1
	dg_arena      Arenas[2];
} dg_frame_arena_pair;

/**
 * @brief Returns the current frame's write arena.
 * @retval DGE_RAII_NULL_ARENA_PTR  Pair was null.
 * @retval DGE_RAII_NULL_OUT_PTR    OutPtrRef was null.
 * @retval DGE_RAII_NONE            Success.
 */
[[nodiscard]] static inline dg_errcode_64 dg_frame_write_arena( dg_frame_arena_pair* Pair, dg_arena** OutPtrRef ) {
	if ( DG_UNLIKELY( Pair == nullptr ) ) {
		return DGE_RAII_NULL_ARENA_PTR;
	}
	if ( DG_UNLIKELY( OutPtrRef == nullptr ) ) {
		return DGE_RAII_NULL_OUT_PTR;
	}
	*OutPtrRef = &Pair->Arenas[ atomic_load_explicit( &Pair->WriteIdx, memory_order_acquire ) ];
	return DGE_RAII_NONE;
}

/**
 * @brief Returns the previous frame's read arena.
 * @retval DGE_RAII_NULL_ARENA_PTR  Pair was null.
 * @retval DGE_RAII_NULL_OUT_PTR    OutPtrRef was null.
 * @retval DGE_RAII_NONE            Success.
 */
[[nodiscard]] static inline dg_errcode_64 dg_frame_read_arena( dg_frame_arena_pair* Pair, dg_arena** OutPtrRef ) {
	if ( DG_UNLIKELY( Pair == nullptr ) ) {
		return DGE_RAII_NULL_ARENA_PTR;
	}
	if ( DG_UNLIKELY( OutPtrRef == nullptr ) ) {
		return DGE_RAII_NULL_OUT_PTR;
	}
	*OutPtrRef = &Pair->Arenas[ atomic_load_explicit( &Pair->WriteIdx, memory_order_acquire ) ^ 1u ];
	return DGE_RAII_NONE;
}

/**
 * @brief Flips the double buffer, resetting the incoming write arena. Must be called by a single thread only.
 * @retval DGE_RAII_NULL_ARENA_PTR  Pair was null.
 * @retval DGE_RAII_NONE            Success.
 */
[[nodiscard]] static inline dg_errcode_64 dg_frame_flip( dg_frame_arena_pair* Pair ) {
	#define NewWriteIdx OldWriteIdx ^ 1u

	if ( DG_UNLIKELY( Pair == nullptr ) ) {
		return DGE_RAII_NULL_ARENA_PTR;
	}
	dg_u32 OldWriteIdx = atomic_fetch_xor_explicit( &Pair->WriteIdx, 1u, memory_order_acq_rel );

	dg_arena_reset( &Pair->Arenas[ NewWriteIdx ] );

	return DGE_RAII_NONE;

	#undef  NewWriteIdx
}

/**
 * @brief Initializes a double-buffered arena pair over two equally-sized backing buffers.
 *
 * @param Pair    Arena pair to initialize. Must not be null.
 * @param MemoryA Backing memory for the first arena.
 * @param MemoryB Backing memory for the second arena.
 * @param CapT    Capacity in bytes of each backing buffer. Both arenas share this size.
 *
 * @retval DGE_RAII_NULL_PAIR_PTR  Pair was null.
 * @retval ...           Inherits all error codes from dg_arena_init.
 *
 * @see dg_arena_init
 * @see dg_frame_flip
 * @see dg_frame_write_arena
 * @see dg_frame_read_arena
 */
[[nodiscard]] static inline dg_errcode_64 dg_frame_arena_pair_init( dg_frame_arena_pair* Pair, dg_qword* MemoryA, dg_qword* MemoryB, dg_inbytes CapT ) {
	if ( DG_UNLIKELY( Pair == nullptr ) ) {
		return DGE_RAII_NULL_PAIR_PTR;
	}
	atomic_store_explicit( &Pair->WriteIdx, 0u, memory_order_relaxed );
	for (dg_errcode_64 E = dg_arena_init( &Pair->Arenas[0], MemoryA, CapT );  DG_UNLIKELY( !dg_error_ok(E) );) {
		return E;
	}
	return dg_arena_init( &Pair->Arenas[1], MemoryB, CapT );
}

#endif //DG_ARENA_FRAME_H
