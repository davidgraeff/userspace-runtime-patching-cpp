#include "make_jmp.h"

#define ABS(x) ((x) >= 0 ? (x) : -(x))

void make_jmp32(intptr_t src_addr, intptr_t dst_addr) {
    auto jmp = (JumpInsn *) src_addr;
    jmp->opcode = JMP_OPCODE;
    jmp->offset = (int32_t) (dst_addr - (src_addr + sizeof(*jmp)));
}

void make_jmp64(uintptr_t src, uintptr_t dst) {
    auto jmp = (Jmp64Insn *) src;
    jmp->push_opcode = PUSH_OPCODE;
    jmp->push_addr = (uint32_t) dst; /* truncate */
    jmp->mov_opcode = MOV_OPCODE;
    jmp->mov_modrm = JMP64_MOV_MODRM;
    jmp->mov_sib = JMP64_MOV_SIB;
    jmp->mov_offset = JMP64_MOV_OFFSET;
    jmp->mov_addr = (uint32_t) ((dst) >> 32);
    jmp->ret_opcode = RET_OPCODE;
}

void make_jmp(void *src, void *dst) {
    auto src_addr = (intptr_t) src;
    auto dst_addr = (intptr_t) dst;
    int64_t distance = std::abs(src_addr - dst_addr);
    return (distance < INT32_MIN || distance > INT32_MAX) ? make_jmp64(src_addr, dst_addr) : make_jmp32(src_addr,
                                                                                                        dst_addr);
}