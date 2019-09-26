#pragma once

#include <inttypes.h>
#include <cstddef>
#include <cmath>

#pragma pack(push, 1)
struct JumpInsn {
    uint8_t opcode;
    int32_t offset;
};

static_assert(sizeof(JumpInsn) == 5);

/* AMD64 doesn't support 64-bit direct jumps.
 * Push the address onto the stack, then call RET.
 */
struct Jmp64Insn {
    uint8_t push_opcode;
    uint32_t push_addr; /* lower 32-bits of the address to jump to */
    uint8_t mov_opcode;
    uint8_t mov_modrm;
    uint8_t mov_sib;
    uint8_t mov_offset;
    uint32_t mov_addr;  /* upper 32-bits of the address to jump to */
    uint8_t ret_opcode;
};
#pragma pack(pop)

#define PUSH_OPCODE 0x68
#define MOV_OPCODE  0xC7
#define RET_OPCODE  0xC3
#define JMP_OPCODE  0xE9

#define JMP64_MOV_MODRM  0x44 /* write to address + 1 byte displacement */
#define JMP64_MOV_SIB    0x24 /* write to [rsp] */
#define JMP64_MOV_OFFSET 0x04

/// Write valid x86 jump code to the given target address
void make_jmp(void *src, void *dst);

inline size_t get_jmp_size(void *src, void *dst) {
    auto src_addr = (intptr_t) src;
    auto dst_addr = (intptr_t) dst;
    int64_t distance = std::abs(src_addr - dst_addr);
    return (distance < INT32_MIN || distance > INT32_MAX) ? sizeof(struct Jmp64Insn) : sizeof(struct JumpInsn);
}

/// If the target method has been patched with an unconditional jump, this method will
/// return the jump address or null otherwise.
inline void* read_jmp_destination(void *src) {
    auto maybe_jmp32 = (JumpInsn *) src;
    auto maybe_jmp64 = (Jmp64Insn *) src;

    if (maybe_jmp32->opcode == JMP_OPCODE) {
        return (void *) (maybe_jmp32->offset + (uintptr_t) src + sizeof(JumpInsn));
    } else if (maybe_jmp64->push_opcode == PUSH_OPCODE
               && maybe_jmp64->mov_opcode == MOV_OPCODE
               && maybe_jmp64->mov_modrm == JMP64_MOV_MODRM
               && maybe_jmp64->mov_sib == JMP64_MOV_SIB
               && maybe_jmp64->mov_offset == JMP64_MOV_OFFSET
               && maybe_jmp64->ret_opcode == RET_OPCODE) {
        return (void *) (maybe_jmp64->push_addr & ((uintptr_t) maybe_jmp64->mov_addr << 32));
    }
    return nullptr;
}