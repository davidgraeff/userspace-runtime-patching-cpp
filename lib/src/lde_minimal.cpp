#include "lde_minimal.h"

enum flags {
    MODRM = 1,
    PLUS_R = 1 << 1,
    REG_OPCODE = 1 << 2,
    IMM8 = 1 << 3,
    IMM16 = 1 << 4,
    IMM32 = 1 << 5,
    RELOC = 1 << 6
};

const uint8_t prefixes[] = {
        0xF0, 0xF2, 0xF3, 0x2E, 0x36, 0x3E, 0x26, 0x64, 0x65,
        0x66, /* operand override */
        0x67  /* address override */
};

struct OpCode {
    uint8_t opcode;
    uint8_t reg_opcode;
    unsigned int flags;
};

/*
     * https://www-ssl.intel.com/content/www/us/en/processors/architectures-software-developer-manuals.html
     * Intel Developer Manual volumes 2a and 2b
     */
const struct OpCode opcodes[] = {
        /* ADD AL, imm8      */ {0x04, 0, IMM8},
        /* ADD EAX, imm32    */
                                {0x05, 0, IMM32},
        /* ADD r/m8, imm8    */
                                {0x80, 0, MODRM | REG_OPCODE | IMM8},
        /* ADD r/m32, imm32  */
                                {0x81, 0, MODRM | REG_OPCODE | IMM32},
        /* ADD r/m32, imm8   */
                                {0x83, 0, MODRM | REG_OPCODE | IMM8},
        /* ADD r/m8, r8      */
                                {0x00, 0, MODRM},
        /* ADD r/m32, r32    */
                                {0x01, 0, MODRM},
        /* ADD r8, r/m8      */
                                {0x02, 0, MODRM},
        /* ADD r32, r/m32    */
                                {0x03, 0, MODRM},
        /* AND AL, imm8      */
                                {0x24, 0, IMM8},
        /* AND EAX, imm32    */
                                {0x25, 0, IMM32},
        /* AND r/m8, imm8    */
                                {0x80, 4, MODRM | REG_OPCODE | IMM8},
        /* AND r/m32, imm32  */
                                {0x81, 4, MODRM | REG_OPCODE | IMM32},
        /* AND r/m32, imm8   */
                                {0x83, 4, MODRM | REG_OPCODE | IMM8},
        /* AND r/m8, r8      */
                                {0x20, 0, MODRM},
        /* AND r/m32, r32    */
                                {0x21, 0, MODRM},
        /* AND r8, r/m8      */
                                {0x22, 0, MODRM},
        /* AND r32, r/m32    */
                                {0x23, 0, MODRM},
        /* CALL rel32        */
                                {0xE8, 0, IMM32 | RELOC},
        /* CALL r/m32        */
                                {0xFF, 2, MODRM | REG_OPCODE},
        /* CMP r/m16/32, imm8*/
                                {0x83, 7, MODRM | REG_OPCODE | IMM8},
        /* DEC r/m16/32      */
                                {0xFF, 1, MODRM | REG_OPCODE},
        /* ENTER imm16, imm8 */
                                {0xC8, 0, IMM16 | IMM8},
        /* INT 3             */
                                {0xCC, 0, 0},
        /* JMP rel32         */
                                {0xE9, 0, IMM32 | RELOC},
        /* JMP r/m32         */
                                {0xFF, 4, MODRM | REG_OPCODE},
        /* LEA r32,m         */
                                {0x8D, 0, MODRM},
        /* LEAVE             */
                                {0xC9, 0, 0},
        /* MOV r/m8,r8       */
                                {0x88, 0, MODRM},
        /* MOV r/m32,r32     */
                                {0x89, 0, MODRM},
        /* MOV r8,r/m8       */
                                {0x8A, 0, MODRM},
        /* MOV r32,r/m32     */
                                {0x8B, 0, MODRM},
        /* MOV r/m16,Sreg    */
                                {0x8C, 0, MODRM},
        /* MOV Sreg,r/m16    */
                                {0x8E, 0, MODRM},
        /* MOV AL,moffs8     */
                                {0xA0, 0, IMM8},
        /* MOV EAX,moffs32   */
                                {0xA1, 0, IMM32},
        /* MOV moffs8,AL     */
                                {0xA2, 0, IMM8},
        /* MOV moffs32,EAX   */
                                {0xA3, 0, IMM32},
        /* MOV r8, imm8      */
                                {0xB0, 0, PLUS_R | IMM8},
        /* MOV r32, imm32    */
                                {0xB8, 0, PLUS_R | IMM32},
        /* MOV r/m8, imm8    */
                                {0xC6, 0, MODRM | REG_OPCODE | IMM8},
        /* MOV r/m32, imm32  */
                                {0xC7, 0, MODRM | REG_OPCODE | IMM32},
        /* NOP               */
                                {0x90, 0, 0},
        /* OR AL, imm8       */
                                {0x0C, 0, IMM8},
        /* OR EAX, imm32     */
                                {0x0D, 0, IMM32},
        /* OR r/m8, imm8     */
                                {0x80, 1, MODRM | REG_OPCODE | IMM8},
        /* OR r/m32, imm32   */
                                {0x81, 1, MODRM | REG_OPCODE | IMM32},
        /* OR r/m32, imm8    */
                                {0x83, 1, MODRM | REG_OPCODE | IMM8},
        /* OR r/m8, r8       */
                                {0x08, 0, MODRM},
        /* OR r/m32, r32     */
                                {0x09, 0, MODRM},
        /* OR r8, r/m8       */
                                {0x0A, 0, MODRM},
        /* OR r32, r/m32     */
                                {0x0B, 0, MODRM},
        /* POP r/m32         */
                                {0x8F, 0, MODRM | REG_OPCODE},
        /* POP r32           */
                                {0x58, 0, PLUS_R},
        /* PUSH r/m32        */
                                {0xFF, 6, MODRM | REG_OPCODE},
        /* PUSH r32          */
                                {0x50, 0, PLUS_R},
        /* PUSH imm8         */
                                {0x6A, 0, IMM8},
        /* PUSH imm32        */
                                {0x68, 0, IMM32},
        /* RET               */
                                {0xC3, 0, 0},
        /* RET imm16         */
                                {0xC2, 0, IMM16},
        /* SUB AL, imm8      */
                                {0x2C, 0, IMM8},
        /* SUB EAX, imm32    */
                                {0x2D, 0, IMM32},
        /* SUB r/m8, imm8    */
                                {0x80, 5, MODRM | REG_OPCODE | IMM8},
        /* SUB r/m32, imm32  */
                                {0x81, 5, MODRM | REG_OPCODE | IMM32},
        /* SUB r/m32, imm8   */
                                {0x83, 5, MODRM | REG_OPCODE | IMM8},
        /* SUB r/m8, r8      */
                                {0x28, 0, MODRM},
        /* SUB r/m32, r32    */
                                {0x29, 0, MODRM},
        /* SUB r8, r/m8      */
                                {0x2A, 0, MODRM},
        /* SUB r32, r/m32    */
                                {0x2B, 0, MODRM},
        /* TEST AL, imm8     */
                                {0xA8, 0, IMM8},
        /* TEST EAX, imm32   */
                                {0xA9, 0, IMM32},
        /* TEST r/m8, imm8   */
                                {0xF6, 0, MODRM | REG_OPCODE | IMM8},
        /* TEST r/m32, imm32 */
                                {0xF7, 0, MODRM | REG_OPCODE | IMM32},
        /* TEST r/m8, r8     */
                                {0x84, 0, MODRM},
        /* TEST r/m32, r32   */
                                {0x85, 0, MODRM},
        /* XOR AL, imm8      */
                                {0x34, 0, IMM8},
        /* XOR EAX, imm32    */
                                {0x35, 0, IMM32},
        /* XOR r/m8, imm8    */
                                {0x80, 6, MODRM | REG_OPCODE | IMM8},
        /* XOR r/m32, imm32  */
                                {0x81, 6, MODRM | REG_OPCODE | IMM32},
        /* XOR r/m32, imm8   */
                                {0x83, 6, MODRM | REG_OPCODE | IMM8},
        /* XOR r/m8, r8      */
                                {0x30, 0, MODRM},
        /* XOR r/m32, r32    */
                                {0x31, 0, MODRM},
        /* XOR r8, r/m8      */
                                {0x32, 0, MODRM},
        /* XOR r32, r/m32    */
                                {0x33, 0, MODRM}
};


auto disasm(void *src) -> std::tuple<int, int> {
    auto *code = static_cast<uint8_t *>(src);

    // Return values
    int len = 0;
    int reloc_op_offset = 0;

    int operand_size = 4;

    for (unsigned char prefix : prefixes) {
        if (code[len] == prefix) {
            len++;
            if (prefix == 0x66) {
                operand_size = 2;
            }
        }
    }

    /* This is a REX prefix (40H - 4FH). REX prefixes are valid only in 64-bit mode. */
    if ((code[len] & 0xF0) == 0x40) {
        uint8_t rex = code[len++];

        if (rex & 8) {
            /* REX.W changes size of immediate operand to 64 bits. */
            operand_size = 8;
        }
    }

    OpCode opcode;
    int found_opcode = false;
    for (auto &i : opcodes) {
        if (code[len] == i.opcode) {
            if (i.flags & REG_OPCODE) {
                found_opcode = ((code[len + 1] >> 3) & 7) == i.reg_opcode;
            } else {
                found_opcode = true;
            }
        }

        if ((i.flags & PLUS_R)
            && (code[len] & 0xF8) == i.opcode) {
            found_opcode = true;
        }

        if (found_opcode) {
            len++;
            opcode = i;
            break;
        }
    }

    if (!found_opcode) {
        return {0, 0};
    }

    if (opcode.flags & RELOC) {
        reloc_op_offset = len; /* relative call or jump */
    }

    if (opcode.flags & MODRM) {
        uint8_t modrm = code[len++]; /* +1 for Mod/RM byte */
        uint8_t mod = modrm >> 6;
        uint8_t rm = modrm & 7;

        if (mod != 3 && rm == 4) {
            uint8_t sib = code[len++]; /* +1 for SIB byte */
            uint8_t base = sib & 7;

            if (base == 5) {
                /* The SIB is followed by a disp32 with no base if the MOD is 00B.
                 * Otherwise, disp8 or disp32 + [EBP].
                 */
                if (mod == 1) {
                    len += 1; /* for disp8 */
                } else {
                    len += 4; /* for disp32 */
                }
            }
        }

        if (rm == 5) {
            reloc_op_offset = (int32_t) len; /* RIP-relative addressing */
        }

        if (mod == 1) {
            len += 1; /* for disp8 */
        }
        if (mod == 2 || (mod == 0 && rm == 5)) {
            len += 4; /* for disp32 */
        }
    }

    if (opcode.flags & IMM8) {
        len += 1;
    }
    if (opcode.flags & IMM16) {
        len += 2;
    }
    if (opcode.flags & IMM32) {
        len += operand_size;
    }

    return {len, reloc_op_offset};
}

int disasm_until(void *src, int min_len) {
    auto src_addr = (intptr_t) src;
    size_t insn_len;
    size_t orig_size = 0;

    while (orig_size < min_len) {
        int reloc_op_offset = 0;
        std::tie(insn_len, reloc_op_offset) = disasm((void *) (src_addr + orig_size));

        if (insn_len == 0) {
            return 0;
        }

        orig_size += insn_len;
    }
}