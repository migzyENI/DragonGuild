//dg_raii.h
// (C) MigzyENT 2026

#ifndef DG_RAII_H
#define DG_RAII_H

#include <stdint.h>
#pragma once

#if !defined(__cplusplus) && __STDC_VERSION__ < 202311L
	#error "This file requires C23 or later. (dg_raii)"
#endif

#include <dg_typedef.h>
#include <dg_error.h>
#include <dg_allocator.h>

#include <string.h>


//
//
// Error Codes

constexpr dg_u32 DG_RAII_DWORD_NAME = 0x52414949 ; // 'RAII'

/**
 * @brief Error index values for the RAII module, placed in the .Err field of dg_errcode_64.
 */
/*
typedef enum dge_raii_idx : dg_u32 {
	DGE_RAII_IDX_NONE                = 0u,
	DGE_RAII_IDX_OUT_OF_MEMORY       = 1u,
	DGE_RAII_IDX_NULL_ARENA_PTR      = 2u,
	DGE_RAII_IDX_NULL_MEMORY_PTR     = 3u,
	DGE_RAII_IDX_NULL_POOL_PTR       = 4u, //Used in dg_arena_pool
	DGE_RAII_IDX_NULL_SLAB_PTR       = 5u,
	DGE_RAII_IDX_NULL_OUT_PTR        = 6u,
	DGE_RAII_IDX_ALIGNMENT_FAULT     = 7u,
	DGE_RAII_IDX_SIZE_OVERFLOW       = 8u,
	DGE_RAII_IDX_OUT_OF_BOUNDS       = 9u,
	DGE_RAII_IDX_SIZE_UNINITIALIZED  = 10u,
	DGE_RAII_IDX_NULL_PAIR_PTR       = 11u,
	DGE_RAII_IDX_DOUBLE_FREE         = 12u,
	DGE_RAII_IDX_NULL_POOL_FIXED_PTR = 13u,
	DGE_RAII_IDX_CAP_MAXIMUM         = 14u,
	DGE_RAII_IDX_COUNT               = 15u,
} dge_raii_idx;

constexpr dg_errcode_64 DGE_RAII_NONE                = { .DWord = DG_RAII_DWORD_NAME, .Err = DGE_RAII_IDX_NONE              };
constexpr dg_errcode_64 DGE_RAII_OUT_OF_MEMORY       = { .DWord = DG_RAII_DWORD_NAME, .Err = DGE_RAII_IDX_OUT_OF_MEMORY     };
constexpr dg_errcode_64 DGE_RAII_NULL_ARENA_PTR      = { .DWord = DG_RAII_DWORD_NAME, .Err = DGE_RAII_IDX_NULL_ARENA_PTR    };
constexpr dg_errcode_64 DGE_RAII_NULL_MEMORY_PTR     = { .DWord = DG_RAII_DWORD_NAME, .Err = DGE_RAII_IDX_NULL_MEMORY_PTR   };
constexpr dg_errcode_64 DGE_RAII_NULL_POOL_PTR       = { .DWord = DG_RAII_DWORD_NAME, .Err = DGE_RAII_IDX_NULL_POOL_PTR     };
constexpr dg_errcode_64 DGE_RAII_NULL_SLAB_PTR       = { .DWord = DG_RAII_DWORD_NAME, .Err = DGE_RAII_IDX_NULL_SLAB_PTR     };
constexpr dg_errcode_64 DGE_RAII_NULL_OUT_PTR        = { .DWord = DG_RAII_DWORD_NAME, .Err = DGE_RAII_IDX_NULL_OUT_PTR      };
constexpr dg_errcode_64 DGE_RAII_ALIGNMENT_FAULT     = { .DWord = DG_RAII_DWORD_NAME, .Err = DGE_RAII_IDX_ALIGNMENT_FAULT   };
constexpr dg_errcode_64 DGE_RAII_SIZE_OVERFLOW       = { .DWord = DG_RAII_DWORD_NAME, .Err = DGE_RAII_IDX_SIZE_OVERFLOW     };
constexpr dg_errcode_64 DGE_RAII_OUT_OF_BOUNDS       = { .DWord = DG_RAII_DWORD_NAME, .Err = DGE_RAII_IDX_OUT_OF_BOUNDS     };
constexpr dg_errcode_64 DGE_RAII_SIZE_UNINITIALIZED  = { .DWord = DG_RAII_DWORD_NAME, .Err = DGE_RAII_IDX_SIZE_UNINITIALIZED};
constexpr dg_errcode_64 DGE_RAII_NULL_PAIR_PTR       = { .DWord = DG_RAII_DWORD_NAME, .Err = DGE_RAII_IDX_NULL_PAIR_PTR     };
constexpr dg_errcode_64 DGE_RAII_DOUBLE_FREE         = { .DWord = DG_RAII_DWORD_NAME, .Err = DGE_RAII_IDX_DOUBLE_FREE       };
constexpr dg_errcode_64 DGE_RAII_NULL_POOL_FIXED_PTR = { .DWord = DG_RAII_DWORD_NAME, .Err = DGE_RAII_IDX_NULL_POOL_FIXED_PTR};
constexpr dg_errcode_64 DGE_RAII_CAP_MAXIMUM         = { .DWord = DG_RAII_DWORD_NAME, .Err = DGE_RAII_IDX_CAP_MAXIMUM};
                             */
DG_ERRCODE_TABLE_GENERATE(
	DGE_RAII, _IDX, _COUNT, DG_RAII_DWORD_NAME,
	OUT_OF_MEMORY,
	NULL_ARENA_PTR,
	NULL_MEMORY_PTR,
	NULL_POOL_PTR,
	NULL_SLAB_PTR,
	NULL_OUT_PTR,
	ALIGNMENT_FAULT,
	SIZE_OVERFLOW,
	OUT_OF_BOUNDS,
	SIZE_UNINITIALIZED,
	NULL_PAIR_PTR,
	DOUBLE_FREE,
	NULL_POOL_FIXED_PTR,
	CAP_MAXIMUM,
)

//
//
// Definitions


/* Function Structure:
 d g*_arena_init x
 dg_arena_alloc x
 dg_arena_alloc_zero x
 dg_arena_remaining x
 dg_arena_reset x
 dg_arena_sub x
 ─────────────────
 dg_arena_scratch_begin
 dg_arena_scratch_release
 ─────────────────
 dg_scope_guard
 dg_scope_guard_release
 ─────────────────
 dg_frame_arena_pair + flip
 dg_arena_set + buckets
 ─────────────────
 pool (dg_pool_slab, dg_pool, alloc, free, macros)
 ─────────────────
 prefetch
 */


///////////////////////////////////////
//
// declarations

/**
 * @brief Cleanup function signature for RAII resource release.
 *
 * Used with __attribute__((cleanup)) or dg_scope_guard to automatically
 * release a resource when it goes out of scope. The implementor is
 * responsible for nulling the pointer after release.
 *
 * @param MemPtr Pointer to the resource pointer to release.
 *
 * @see dg_scope_guard
 * @see DG_SCOPED
 */
DG_FN( void , dg_cleanup_sig, void** MemPtr);

/**
 * @brief The Arena Struct of DG_RAII.
 * @see dg_arena_init
 * @see dg_arena_alloc
 */
typedef struct dg_arena {
	dg_qword*  Memory;
	dg_inwords Offset;
	dg_inwords Cap;
} dg_arena;

/**
 * @brief Save point on an arena's bump offset. Releasing restores the arena to the saved position.
 * @see dg_arena_scratch_begin
 * @see dg_arena_scratch_release
 */
typedef struct dg_arena_scratch {
	dg_errcode_64 Code;
	dg_arena* Arena;
	dg_u64 OldOffset;
} dg_arena_scratch;

/**
 * @brief RAII guard holding a resource pointer and its cleanup function.
 * @see dg_raii_init
 * @see dg_raii_guard_release
 * @see DG_SCOPED
 */
typedef struct dg_scope_guard {
	dg_errcode_64 Code;
	void* Resource;
	dg_cleanup_sig CleanupSig;
} dg_scope_guard;

///////////////////////////////////////
//
// dg_arena

#ifdef DG_RAII_WORD_SIZE_TYPE
_Static_assert(sizeof(DG_RAII_WORD_SIZE_TYPE) > (dg_inbytes)4u, "Words must be minimum 32bit.");
constexpr dg_inbytes DG_WORD_SIZE = sizeof(DG_RAII_WORD_SIZE_TYPE);
#else
constexpr dg_inbytes DG_WORD_SIZE = sizeof(dg_qword);
#endif
constexpr dg_u64 DG_WORD_ALIGN_MASK = DG_WORD_SIZE - 1u;

constexpr dg_u64 DG_ARENA_MAX_SAFE_SIZE = (dg_u64)SIZE_MAX - DG_WORD_ALIGN_MASK;



/**
 * @brief Initializes an arena over a pre-allocated memory block.
 *
 * Cap is stored internally in words (divided by DG_WORD_SIZE).
 * The memory block must outlive the arena.
 *
 * @param Arena  Arena to initialize. Must not be null.
 * @param Memory Pointer to backing memory. Must be 8-byte aligned.
 * @param Cap    Size of backing memory in bytes.
 *
 * @retval DGE_RAII_NONE              Success.
 * @retval DGE_RAII_NULL_ARENA_PTR    Arena pointer was null.
 * @retval DGE_RAII_NULL_MEMORY_PTR   Memory pointer was null.
 * @retval DGE_RAII_ALIGNMENT_FAULT   Memory pointer is not 8-byte aligned.
 *
 * @see dg_arena_alloc
 * @see dg_arena_reset
 */
[[nodiscard]] static inline dg_errcode_64 dg_arena_init( dg_arena* Arena, dg_qword* Memory, dg_inbytes Cap ) {
	if ( DG_UNLIKELY( Arena == nullptr ) ) {
		return DGE_RAII_NULL_ARENA_PTR;
	}

	if ( Memory == nullptr ) {
		return DGE_RAII_NULL_MEMORY_PTR;
	}

	if ( (dg_u64)Memory & DG_WORD_ALIGN_MASK ) {
		return DGE_RAII_ALIGNMENT_FAULT;
	}

	if ( (Cap > (dg_u64)UINT32_MAX * DG_WORD_SIZE)) {
		return DGE_RAII_CAP_MAXIMUM;
	}
	// Here we map the pointers on to the arena and ensure the offset is 0,
	//the cap is obvious how many bytes it takes.
	Arena->Memory = Memory;
	Arena->Offset = 0u;
	Arena->Cap    = Cap / DG_WORD_SIZE;
	return DGE_RAII_NONE;
}



/**
 * @brief Allocates a block of memory from the arena via bump pointer.
 *
 * Size is rounded up to the nearest 8-byte word boundary internally.
 * The returned pointer is valid until the arena is reset or freed.
 *
 * @param Arena  Arena to allocate from. Must not be null.
 * @param Size   Number of bytes to allocate.
 * @param OutPtr Receives the pointer to the allocated block. Must not be null.
 *
 * @retval DGE_RAII_NONE            Success.
 * @retval DGE_RAII_NULL_ARENA_PTR  Arena was null.
 * @retval DGE_RAII_NULL_OUT_PTR    OutPtr was null.
 * @retval DGE_RAII_SIZE_OVERFLOW   Size exceeds safe allocation limit.
 * @retval DGE_RAII_OUT_OF_MEMORY   Arena has insufficient remaining capacity.
 *
 * @see dg_arena_init
 * @see dg_arena_remaining
 * @see dg_arena_reset
 */
[[nodiscard]] static inline dg_errcode_64 dg_arena_alloc( dg_arena* Arena, dg_inbytes Size, void** OutPtr ) {
	if ( DG_UNLIKELY( Arena == nullptr ) ) {
		return DGE_RAII_NULL_ARENA_PTR;
	}
	if ( OutPtr == nullptr ) {
		return DGE_RAII_NULL_OUT_PTR;
	}

	if ( DG_UNLIKELY( Size > DG_ARENA_MAX_SAFE_SIZE ) ) {
		return DGE_RAII_SIZE_OVERFLOW;
	}

	dg_inwords WordCount = (dg_inwords)(( Size + DG_WORD_ALIGN_MASK ) / DG_WORD_SIZE);

	if ( ( Arena->Offset + WordCount ) > Arena->Cap ) {
		return DGE_RAII_OUT_OF_MEMORY;
	}
	// Here we map the end of the Arena onto the offset saved index onto the OutPtr for all the new memory.
	//We update the offset and carriage return.
	*OutPtr = &Arena->Memory[ Arena->Offset ];
	Arena->Offset = Arena->Offset + WordCount;
	return DGE_RAII_NONE;
}

/**
 * @brief Allocates a block of memory from the arena and zero it.
 *
 * Size is rounded up to the nearest 8-byte word boundary internally.
 * The returned pointer is valid until the arena is reset or freed.
 * Only the allocated block is zeroed, not the entire arena.
 *
 * @param Arena  Arena to allocate from. Must not be null.
 * @param Size   Number of bytes to allocate.
 * @param OutPtr Receives the pointer to the allocated block. Must not be null.
 *
 * @retval DGE_RAII_NONE            Success.
 * @retval DGE_RAII_NULL_ARENA_PTR  Arena was null.
 * @retval DGE_RAII_NULL_OUT_PTR    OutPtr was null.
 * @retval DGE_RAII_SIZE_OVERFLOW   Size exceeds safe allocation limit.
 * @retval DGE_RAII_OUT_OF_MEMORY   Arena has insufficient remaining capacity.
 *
 * @see dg_arena_init
 * @see dg_arena_alloc
 */
[[nodiscard]] static inline dg_errcode_64 dg_arena_alloc_zero( dg_arena* Arena, dg_inbytes Size, void** OutPtr ) {
	dg_errcode_64 Code = dg_arena_alloc( Arena, Size, OutPtr );
	if ( dg_error_ok( Code ) ) {
		memset( *OutPtr, 0, Size );
	}
	return Code;
}

/**
 * @brief Returns the number of bytes remaining in the arena.
 *
 * Returns 0 as a sentinel if Arena is null or the offset has overrun Cap.
 * Result is in bytes, not words.
 *
 * @param Arena Arena to query. May be null — returns 0 safely.
 *
 * @return Remaining bytes available for allocation, or 0 if null or exhausted.
 *
 * @see dg_arena_alloc
 * @see dg_arena_reset
 */
static inline dg_inbytes dg_arena_remaining( dg_arena* Arena ) {
	if ( DG_UNLIKELY( Arena == nullptr ) || DG_UNLIKELY( Arena->Offset > Arena->Cap ) ) {
		return 0u;
	}
	return ( Arena->Cap - Arena->Offset ) * DG_WORD_SIZE; //Turns into bytes here.
}


/**
 * @brief Resets the arena offset to zero, making all memory available again.
 *
 * Does not clear or zero any data — existing memory contents remain.
 * Do not read past the offset after reset as that memory is considered free.
 * Null-safe: does nothing if Arena is null.
 *
 * @param Arena Arena to reset. May be null.
 *
 * @see dg_arena_alloc
 * @see dg_arena_remaining
 */
static inline void dg_arena_reset( dg_arena* Arena ) {
	if ( DG_LIKELY( Arena != nullptr ) ) {
		Arena->Offset = 0u;
	}
}

/**
 * @brief Carves a child arena out of a parent arena's backing memory.
 *
 * Allocates a block from Parent and initializes OutChild over it.
 * The child's lifetime is bounded by the parent — resetting the parent
 * invalidates the child's memory. Prefer this for scoped sub-allocations
 * that need their own reset boundary without a separate backing buffer.
 *
 * Inherits all error codes from dg_arena_alloc and dg_arena_init.
 *
 * @param Parent   Source arena to carve from. Must not be null.
 * @param Size     Size in bytes of the child arena's backing block.
 * @param OutChild Output arena to initialize. Must not be null.
 *
 * @retval DGE_RAII_NONE            Success.
 * @retval DGE_RAII_NULL_OUT_PTR    OutChild was null.
 * @retval DGE_RAII_NULL_ARENA_PTR  Parent was null (propagated from dg_arena_alloc).
 * @retval DGE_RAII_OUT_OF_MEMORY   Parent has insufficient remaining capacity.
 * @retval DGE_RAII_SIZE_OVERFLOW   Size exceeds safe allocation limit.
 * @retval DGE_RAII_ALIGNMENT_FAULT Carved block was not 8-byte aligned (should not occur).
 *
 * @see dg_arena_alloc
 * @see dg_arena_init
 * @see dg_arena_reset
 */
[[nodiscard]] static inline dg_errcode_64 dg_arena_sub( dg_arena* Parent, dg_inbytes Size, dg_arena* OutChild ) {
	if ( DG_UNLIKELY( OutChild == nullptr ) ) {
		return DGE_RAII_NULL_OUT_PTR;
	}
	void* Block = nullptr;
	dg_errcode_64 E = dg_arena_alloc( Parent, Size, &Block );
	if ( dg_error_ok(E) ) {
		return dg_arena_init( OutChild, (dg_qword*)Block, Size );
	}
	return E;
}

///////////////////////////////////////
//
// dg_arena_scratch
//
// Scratches are akin to a sub arena but instead of making a new arena they create a
// save point on the offset. Releasing the scratch restores the arena to that point.

/**
 * @brief Creates a scratch boundary on the arena, saving the current offset.
 *
 * Typically used when a cycle of a program needs temporary allocations
 * without permanently advancing the main arena offset.
 *
 * @param Arena Pointer to Arena. May be null — returns an error scratch safely.
 *
 * @retval DGE_RAII_NONE            Success. Scratch holds saved offset and Arena pointer.
 * @retval DGE_RAII_NULL_ARENA_PTR  Arena was null. Returned scratch has null Arena and zero offset.
 *
 * @see dg_arena_scratch
 * @see dg_arena_scratch_release
 */
[[nodiscard]]
static inline dg_arena_scratch dg_arena_scratch_begin( dg_arena* Arena ) {
	if ( DG_UNLIKELY( Arena == nullptr ) ) {
		return (dg_arena_scratch){
			.Code        = DGE_RAII_NULL_ARENA_PTR,
			.Arena       = nullptr,
			.OldOffset = 0u,
		};
	} else {
		return (dg_arena_scratch){
			.Code        = DGE_RAII_NONE,
			.Arena       = Arena,
			.OldOffset = Arena->Offset,
		};
	}
}


/**
 * @brief Releases a scratch, restoring the arena offset to its saved position.
 *
 * Moves the Arena's offset back to the Scratch's saved old-offset, then nulls
 * the Scratch's Arena reference to prevent double release.
 * Null-safe — if Scratch or Scratch->Arena is null this is a no-op.
 *
 * @param Scratch The scratch to release. May be null.
 *
 * @see dg_arena_scratch_begin
 * @see dg_arena_scratch
 */
static inline void dg_arena_scratch_release( dg_arena_scratch* Scratch ) {
	if ( DG_LIKELY( Scratch != nullptr ) ) {
		if ( Scratch->Arena != nullptr ) {
			Scratch->Arena->Offset = Scratch->OldOffset;
			Scratch->Arena = nullptr;
		}
	}
}

///////////////////////////////////////
//
// dg_raii

/**
 * @brief Initialize a guard struct with preset error.
 *
 * @return dg_scope_guard filled out.
 *
 */
static inline dg_scope_guard dg_raii_init_error_fielded( dg_errcode_64 Code, void * Resrc, dg_cleanup_sig Destructor) {
	return (dg_scope_guard) {
		.Code = Code,
		.Resource = Resrc,
		.CleanupSig = Destructor,
	};
}


/**
 * @brief Initialize a guard struct.
 *
 * A Destructor primitive, use in conjuction with DG_SCOPED for in scoping destruction.
 * Otherwise use the following code:
 *  __attribute__(( cleanup ( dg_raii_guard_release ))) dg_scope_guard Var = dg_raii_init( Resrc, Destructor)
 *
 * @return dg_scope_guard filled out.
 *
 * @see DG_SCRATCH_SCOPED
 * @see DG_SCOPED
 */
static inline dg_scope_guard dg_raii_init( void * Resrc, dg_cleanup_sig Destructor) {
	return dg_raii_init_error_fielded(DGE_RAII_NONE, Resrc, Destructor);
}

/**
 * @brief Releases a scope guard, invoking its cleanup function if the guard is valid and holds a resource.
 * @see dg_raii_init
 * @see DG_SCOPED
 */
static inline void dg_raii_guard_release( dg_scope_guard* Guard ) {
	if ( Guard == nullptr ) {
		return;
	}
	if ( !dg_error_ok( Guard->Code ) ) {
		return;
	}
	if (
		Guard->Resource   != nullptr &&
		Guard->CleanupSig != nullptr
	) {
		Guard->CleanupSig( &Guard->Resource );
		Guard->Resource   = nullptr;
		Guard->CleanupSig = nullptr;
	}
}


/**
 * @brief Releases a scope guard then resets the arena offset to zero.
 * @see dg_raii_guard_release
 * @see dg_arena_reset
 */
static inline void dg_arena_reset_guarded( dg_arena* Arena, dg_scope_guard* Guard ) {
	dg_raii_guard_release( Guard );
	if ( DG_LIKELY( Arena != nullptr ) ) {
		Arena->Offset = 0u;
	}
}

/**
 * @brief Declares a scope-guarded variable that automatically calls Destructor when it goes out of scope.
 * @see dg_raii_init
 * @see dg_raii_guard_release
 */
#define DG_SCOPED( VarName, Resrc, Destructor ) \
	__attribute__(( cleanup( dg_raii_guard_release ) )) \
	dg_scope_guard VarName = dg_raii_init( ( Resrc ), ( Destructor ) )

/**
 * @brief Declares an arena scratch that automatically releases when it goes out of scope.
 * @see dg_arena_scratch_begin
 * @see dg_arena_scratch_release
 */
#define DG_SCRATCH_SCOPED( VarName, ArenaPtr ) \
	__attribute__(( cleanup( dg_arena_scratch_release ) )) \
	dg_arena_scratch VarName = dg_arena_scratch_begin( ArenaPtr )

/**
 * @brief Carves a child arena from Parent and declares it with automatic reset on scope exit.
 * @see dg_arena_sub
 * @see dg_arena_reset
 */
#define DG_ARENA_SUB_SCOPED( Parent, Name, Size, ErrVar ) \
	dg_arena (Name) \
	__attribute__(( cleanup( dg_arena_reset ) )) = {0}; \
	dg_errcode_64 ErrVar = dg_arena_sub( (Parent), (Size), &(Name) )

/**
 * @brief Declares a static backing array and initializes an arena set bucket over it.
 * @see dg_arena_init
 */
#define DG_ARENA_INIT_BUCKET_STATIC( ArenaSet, Bucket, Name, WordCount, Err ) \
	dg_qword Name##Backing[ (WordCount) ]; \
	dg_errcode_64 (Err) = dg_arena_init( \
		&(ArenaSet).Buckets[ (Bucket) ], \
		Name##Backing, \
		sizeof( Name##Backing ) \
	)

/**
 * @brief Initializes an arena directly over a named array, using sizeof to derive capacity.
 * @see dg_arena_init
 */
#define DG_ARENA_INIT_ARRAYT_PURE( Arena, Name, Err ) \
	dg_errcode_64 (Err) = dg_arena_init( \
		&Arena, \
		(Name), \
		sizeof( Name ) \
	)

#endif //DG_RAII_H
