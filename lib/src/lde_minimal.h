///! A simple length disassmebler engine (LDE) that works only with most common prologue instructions like push, mov, call, etc.
#pragma once
#include <tuple>

/// maximum length of x86 instruction
#define MAX_INSN_LEN 15

/// Disassembles the instruction at a given address to compute the variable-sized x86 instruction length
///
/// \param src The target function address.
/// \return Returns a tuple with (instruction_len, reloc_op_offset). reloc_op_offset helps for calls and jmps if
///         an operand is a relative address. This is not yet used however.
auto disasm(void *src) -> std::tuple<int, int>;

/// Disassembles instructions at a given address to determine how many instructions need be erased (replaced by nop)
/// after a jump has been inserted.
int disasm_until(void* src, int min_len);