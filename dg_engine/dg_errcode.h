// dg_errcode.h
// (C) MigzyENT 2026
//
// Minimal error code type — depends only on <stdint.h>.
// Safe to include from C99, C11, C23, and C++.
//
// Semantic layer:
//   dg_errcode     — struct tag, always the concrete type (C99+)
//   dg_errcode_64  — canonical engine typedef alias (C99+)
//
// C99: no anonymous union, DWord only — Char[] access not available.
// C11+: anonymous union exposes Char[4] alongside DWord.

#ifndef DG_ERRCODE_H
#define DG_ERRCODE_H

#include <stdint.h>

#if defined(__cplusplus) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L)

// C11 / C23 / C++ — anonymous union available.
typedef struct dg_errcode {
    union {
        uint8_t  Char[4];   // byte access for debug printing
        uint32_t DWord;     // 4-char domain tag  e.g. 'ALOC', 'OBJT'
    };
    uint32_t Err;           // domain-local error index; 0 = success
} dg_errcode;

#if defined(__cplusplus) || __STDC_VERSION__ >= 202311L

#define DG_CAT(a, b)  DG_CAT_(a, b)
#define DG_CAT_(a, b) a##b

#define DG_NARGS(...) DG_NARGS_(__VA_ARGS__, \
128,127,126,125,124,123,122,121,120,119,118,117,116,115,114,113,112,111,110,109,108,107,106,105,104,103,102,101,100,99,98,97,96,95,94,93,92,91,90,89,88,87,86,85,84,83,82,81,80,79,78,77,76,75,74,73,72,71,70,69,68,67,66,65,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1, 0)
#define DG_NARGS_( \
_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,_65,_66,_67,_68,_69,_70,_71,_72,_73,_74,_75,_76,_77,_78,_79,_80,_81,_82,_83,_84,_85,_86,_87,_88,_89,_90,_91,_92,_93,_94,_95,_96,_97,_98,_99,_100,_101,_102,_103,_104,_105,_106,_107,_108,_109,_110,_111,_112,_113,_114,_115,_116,_117,_118,_119,_120,_121,_122,_123,_124,_125,_126,_127,_128, N, ...) N

#define DG_ENUM_ENTRY(PREFIX, IDX, NAME, VAL) \
PREFIX##IDX##_##NAME = VAL##u,

#define DG_CONST_ENTRY(PREFIX, IDX, DWORD, NAME) \
constexpr dg_errcode_64 PREFIX##_##NAME = { .DWord = DWORD, .Err = PREFIX##IDX##_##NAME };

#define DG_ENUM_LIST_1(P,I,V, a)     DG_ENUM_ENTRY(P,I,a,V)
#define DG_ENUM_LIST_2(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_1(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_3(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_2(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_4(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_3(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_5(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_4(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_6(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_5(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_7(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_6(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_8(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_7(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_9(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_8(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_10(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_9(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_11(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_10(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_12(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_11(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_13(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_12(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_14(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_13(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_15(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_14(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_16(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_15(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_17(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_16(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_18(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_17(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_19(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_18(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_20(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_19(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_21(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_20(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_22(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_21(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_23(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_22(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_24(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_23(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_25(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_24(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_26(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_25(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_27(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_26(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_28(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_27(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_29(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_28(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_30(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_29(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_31(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_30(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_32(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_31(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_33(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_32(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_34(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_33(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_35(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_34(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_36(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_35(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_37(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_36(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_38(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_37(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_39(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_38(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_40(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_39(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_41(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_40(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_42(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_41(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_43(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_42(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_44(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_43(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_45(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_44(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_46(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_45(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_47(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_46(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_48(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_47(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_49(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_48(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_50(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_49(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_51(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_50(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_52(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_51(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_53(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_52(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_54(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_53(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_55(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_54(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_56(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_55(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_57(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_56(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_58(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_57(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_59(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_58(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_60(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_59(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_61(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_60(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_62(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_61(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_63(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_62(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_64(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_63(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_65(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_64(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_66(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_65(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_67(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_66(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_68(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_67(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_69(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_68(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_70(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_69(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_71(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_70(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_72(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_71(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_73(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_72(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_74(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_73(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_75(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_74(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_76(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_75(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_77(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_76(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_78(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_77(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_79(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_78(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_80(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_79(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_81(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_80(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_82(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_81(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_83(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_82(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_84(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_83(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_85(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_84(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_86(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_85(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_87(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_86(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_88(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_87(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_89(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_88(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_90(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_89(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_91(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_90(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_92(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_91(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_93(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_92(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_94(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_93(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_95(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_94(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_96(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_95(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_97(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_96(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_98(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_97(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_99(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_98(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_100(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_99(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_101(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_100(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_102(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_101(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_103(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_102(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_104(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_103(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_105(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_104(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_106(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_105(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_107(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_106(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_108(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_107(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_109(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_108(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_110(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_109(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_111(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_110(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_112(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_111(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_113(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_112(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_114(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_113(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_115(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_114(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_116(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_115(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_117(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_116(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_118(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_117(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_119(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_118(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_120(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_119(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_121(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_120(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_122(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_121(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_123(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_122(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_124(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_123(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_125(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_124(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_126(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_125(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_127(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_126(P,I,V+1,__VA_ARGS__)
#define DG_ENUM_LIST_128(P,I,V, a,...) DG_ENUM_ENTRY(P,I,a,V) DG_ENUM_LIST_127(P,I,V+1,__VA_ARGS__)

/* ── Const list expanders (1..128) ────────────────────────────────────── */
#define DG_CONST_LIST_1(P,I,D, a)     DG_CONST_ENTRY(P,I,D,a)
#define DG_CONST_LIST_2(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_1(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_3(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_2(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_4(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_3(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_5(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_4(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_6(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_5(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_7(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_6(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_8(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_7(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_9(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_8(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_10(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_9(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_11(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_10(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_12(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_11(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_13(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_12(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_14(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_13(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_15(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_14(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_16(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_15(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_17(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_16(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_18(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_17(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_19(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_18(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_20(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_19(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_21(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_20(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_22(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_21(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_23(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_22(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_24(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_23(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_25(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_24(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_26(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_25(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_27(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_26(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_28(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_27(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_29(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_28(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_30(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_29(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_31(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_30(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_32(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_31(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_33(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_32(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_34(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_33(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_35(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_34(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_36(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_35(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_37(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_36(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_38(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_37(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_39(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_38(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_40(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_39(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_41(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_40(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_42(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_41(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_43(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_42(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_44(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_43(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_45(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_44(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_46(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_45(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_47(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_46(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_48(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_47(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_49(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_48(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_50(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_49(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_51(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_50(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_52(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_51(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_53(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_52(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_54(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_53(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_55(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_54(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_56(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_55(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_57(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_56(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_58(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_57(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_59(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_58(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_60(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_59(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_61(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_60(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_62(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_61(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_63(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_62(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_64(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_63(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_65(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_64(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_66(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_65(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_67(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_66(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_68(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_67(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_69(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_68(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_70(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_69(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_71(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_70(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_72(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_71(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_73(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_72(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_74(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_73(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_75(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_74(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_76(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_75(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_77(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_76(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_78(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_77(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_79(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_78(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_80(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_79(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_81(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_80(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_82(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_81(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_83(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_82(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_84(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_83(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_85(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_84(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_86(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_85(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_87(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_86(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_88(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_87(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_89(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_88(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_90(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_89(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_91(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_90(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_92(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_91(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_93(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_92(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_94(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_93(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_95(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_94(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_96(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_95(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_97(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_96(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_98(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_97(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_99(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_98(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_100(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_99(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_101(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_100(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_102(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_101(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_103(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_102(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_104(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_103(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_105(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_104(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_106(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_105(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_107(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_106(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_108(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_107(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_109(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_108(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_110(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_109(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_111(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_110(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_112(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_111(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_113(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_112(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_114(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_113(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_115(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_114(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_116(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_115(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_117(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_116(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_118(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_117(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_119(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_118(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_120(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_119(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_121(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_120(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_122(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_121(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_123(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_122(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_124(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_123(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_125(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_124(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_126(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_125(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_127(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_126(P,I,D,__VA_ARGS__)
#define DG_CONST_LIST_128(P,I,D, a,...) DG_CONST_ENTRY(P,I,D,a) DG_CONST_LIST_127(P,I,D,__VA_ARGS__)

#define DG_ERRCODE_TABLE_GENERATE_ENUM(PREFIX, IDX_SUFFIX, COUNT_SUFFIX, ...)     \
typedef enum DG_CAT(DG_CAT(PREFIX, IDX_SUFFIX), _e) : dg_u32 {               \
    PREFIX##IDX_SUFFIX##_NONE = 0u,                                            \
    DG_CAT(DG_ENUM_LIST_, DG_NARGS(__VA_ARGS__))(PREFIX, IDX_SUFFIX, 1, __VA_ARGS__) \
    PREFIX##IDX_SUFFIX##COUNT_SUFFIX,                                          \
} DG_CAT(DG_CAT(PREFIX, IDX_SUFFIX), _e);

#define DG_ERRCODE_TABLE_GENERATE_CONSTS(PREFIX, IDX_SUFFIX, DWORD_VAR, ...)      \
constexpr dg_errcode_64 PREFIX##_NONE = { .DWord = DWORD_VAR, .Err = PREFIX##IDX_SUFFIX##_NONE }; \
DG_CAT(DG_CONST_LIST_, DG_NARGS(__VA_ARGS__))(PREFIX, IDX_SUFFIX, DWORD_VAR, __VA_ARGS__)


// █████   █████ ██████████ ███████████   ██████████ ███ ███ ███ ███ ███ ███ ███ ███ ███ ███             ██████████
//░░███   ░░███ ░░███░░░░░█░░███░░░░░███ ░░███░░░░░█░███░███░███░███░███░███░███░███░███░███            ░███░░░░░░█
// ░███    ░███  ░███  █ ░  ░███    ░███  ░███  █ ░ ░███░███░███░███░███░███░███░███░███░███ █████ █████░███     ░
// ░███████████  ░██████    ░██████████   ░██████   ░███░███░███░███░███░███░███░███░███░███░░███ ░░███ ░█████████
// ░███░░░░░███  ░███░░█    ░███░░░░░███  ░███░░█   ░███░███░███░███░███░███░███░███░███░███ ░░░█████░  ░░░░░░░░███
// ░███    ░███  ░███ ░   █ ░███    ░███  ░███ ░   █░░░ ░░░ ░░░ ░░░ ░░░ ░░░ ░░░ ░░░ ░░░ ░░░   ███░░░███  ███   ░███
// █████   █████ ██████████ █████   █████ ██████████ ███ ███ ███ ███ ███ ███ ███ ███ ███ ███ █████ █████░░████████
// CC. HERE!!!!!!!!!x5
// If you want to see an example please check out dg_raii.h (DG_RAII_H)
//
#define DG_ERRCODE_TABLE_GENERATE(PREFIX, IDX_SUFFIX, COUNT_SUFFIX, DWORD_VAR, ...) \
DG_ERRCODE_TABLE_GENERATE_ENUM(PREFIX, IDX_SUFFIX, COUNT_SUFFIX, __VA_ARGS__)   \
DG_ERRCODE_TABLE_GENERATE_CONSTS(PREFIX, IDX_SUFFIX, DWORD_VAR, __VA_ARGS__)

#endif

#else

#error C99 not supported (dg_errcode.h)
// C99 — no anonymous union, Char[] not accessible.
// DWord and Err carry the same data; domain tag is opaque to C99 consumers.
typedef struct dg_errcode {
    uint32_t DWord;
    uint32_t Err;
} dg_errcode;

#endif

// dg_errcode_64 — canonical engine name.
typedef dg_errcode dg_errcode_64;

#endif // DG_ERRCODE_H
