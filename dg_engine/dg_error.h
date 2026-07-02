//dg_error.h
// (C) MigzyENT 2026

#ifndef DG_ERROR_H
#define DG_ERROR_H
#pragma once


/*
 * WARNING: Currently this code and actually the rest of this file do not handle Big-Endian Systems in these Error systems.
 * Specifically the DWord field of dg_errcode_64 will be reversed thus making it hard to read the name.
 * Future support for Big-Endian Systems for 1.250.000dg
 *
 */

#if !defined(__cplusplus) && __STDC_VERSION__ < 202311L
#error "This file requires C23 or later. (dg_error)"
#endif

#include <dg_typedef.h>

#include <stddef.h>
	//This is for wchar_t on windows platforms.
#include <stdio.h>

constexpr dg_u32 DGE_NONE = 0;
constexpr dg_u32 DGE_ERROR_ANY = 1;
constexpr dg_u32 DGE_ERROR_MAX = 0x00FFFFFF;
//Upper range left incase a error system is for reactive function calling.

//
//
// You attribute Dword via a 4 Char assignment like so: 'DRAG', this will automatically insert it in bytes.
// Its used via the runtime for extra error handler which you may or may not want to be required.
// Err variable inside of errcode is where you place your errors, they can be dynamic or static just define it as you need it and if you care: include a function to handle all returns.
#include <dg_errcode.h> //This now provides table generation. See dg_raii.h for a examplar example.

[[nodiscard]] static inline bool dg_error_ok( dg_errcode_64 Err ) {
	return Err.Err == DGE_NONE;
}

[[nodiscard]] static inline bool dg_error_proto( dg_errcode_64 Err ) {
	return Err.Err > DGE_ERROR_MAX;
}

[[nodiscard]] static inline bool dg_error_acopy_no_domain( dg_errcode_64 Err, dg_errcode_64 ErrFax) {
	return Err.Err == ErrFax.Err;
}

//
//
// For test suites and general purpose for throwaway.
#define DGE_ERROR_IGNORE(Expr) ((void)(Expr))

//
//
// Debug error print — stderr only, not available in release builds.

#if defined(DG_UNIX)
	#define DG_STDERR_CHARWIDTH_TYPE  char
	#define DG_STDERR_PRINT(fmt, ...) fprintf(stderr,  fmt, ##__VA_ARGS__)
	#define DG_STDERR_FMT(x)          x
#elif defined(DG_WINDOWS)
	#define DG_STDERR_CHARWIDTH_TYPE  wchar_t
	#define DG_STDERR_PRINT(fmt, ...) fwprintf(stderr, fmt, ##__VA_ARGS__)
	#define DG_STDERR_FMT(x)          L##x
#endif

#ifndef NDEBUG
static inline void dg_error_print( dg_errcode_64 E ) {
	DG_STDERR_PRINT(
		DG_STDERR_FMT("[DGE LOG] DWord: %c%c%c%c | Err: %u | WordSize: %zu |\n"),
		(char)E.Char[3], (char)E.Char[2], (char)E.Char[1], (char)E.Char[0],
		E.Err, sizeof(void*)
	);
}

#else
	// Release build — dg_error_print calls are a compile error by design.
	// Use DGE_ERROR_IGNORE or handle error codes directly.
	#define dg_error_print(E) _Static_assert(0, "dg_error_print is not available in release builds.")
#endif

#endif //DG_ERROR_H
