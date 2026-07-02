//dg_prefetch.h
// (C) MigzyENT 2026

#ifndef DG_PREFETCH_H
#define DG_PREFETCH_H
#pragma once

#if !defined(__cplusplus) && __STDC_VERSION__ < 202311L
#error "This file requires C23 or later. (dg_raii)"
#endif

#include <dg_typedef.h>

#ifndef DG_PREFETCH_CONFIDENCE_THRESHOLD
#define DG_PREFETCH_CONFIDENCE_THRESHOLD 128u
#endif

#ifndef DG_PREFETCH_CONFIDENCE_DECAY
#define DG_PREFETCH_CONFIDENCE_DECAY 8u
#endif


static inline void dg_prefetch_read( const void* Ptr ) {
	__builtin_prefetch( Ptr, 0, 1 );
}

static inline void dg_prefetch_stream( const void* Ptr ) {
	__builtin_prefetch( Ptr, 0, 0 );
}

typedef struct dg_prefetch_stride {
	dg_ptr  LastAddy;
	dg_ptr  Stride;
	dg_u32  Confidence;   // 0–255
} dg_prefetch_stride;

static inline void dg_prefetch_stride_update( dg_prefetch_stride* PrefStride, const void* Addy, dg_u32 LookAhead ) {
	if ( DG_UNLIKELY( PrefStride == nullptr ) ) {
		return;
	}
	if ( DG_UNLIKELY( Addy == nullptr ) ) {
		return;
	}
	dg_ptr Current  = (dg_ptr)Addy;
	dg_ptr Observed = Current - PrefStride->LastAddy;

	if ( Observed == PrefStride->Stride ) {
		if ( PrefStride->Confidence + 1u < 255u ) {
			PrefStride->Confidence = PrefStride->Confidence + 1u;
		} else {
			PrefStride->Confidence = 255u;
		}
	} else {
		PrefStride->Stride = Observed;
		if ( PrefStride->Confidence > DG_PREFETCH_CONFIDENCE_DECAY ) {
			PrefStride->Confidence = PrefStride->Confidence - DG_PREFETCH_CONFIDENCE_DECAY;
		} else {
			PrefStride->Confidence = 0u;
		}
	}

	if ( PrefStride->Confidence > DG_PREFETCH_CONFIDENCE_THRESHOLD ) {
		dg_prefetch_read( (const dg_byte*)Addy + PrefStride->Stride * (dg_u64)LookAhead );
	}

	PrefStride->LastAddy = (dg_ptr)Addy;
}

#endif
