/*
 * file: xt_flag.h
 * desc: defines all necessary flags of XTaint log
 */

#ifndef XT_FLAG_H
#define XT_FLAG_H

#define XT_INSN_ADDR "32"
#define XT_TCG_DEPOSIT "4a"
#define XT_SIZE_BEGIN "20"
#define XT_SIZE_END "24"
#define XT_CALL_INSN "14"
#define XT_CALL_INSN_FF2 "4e"
 #define XT_CALL_INSN_2nd "4b"
#define XT_RET_INSN "18"
#define XT_RET_INSN_2nd "4c"

#define TCG_QEMU_LD "34"
#define TCG_QEMU_ST "35"
#define TCG_ADD "3b"
#define TCG_XOR "40"
#endif
