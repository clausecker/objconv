/****************************    opcodes.cpp    *******************************
* Author:        Agner Fog
* Date created:  2007-02-21 
* Last modified: 2007-02-25
* Project:       objconv
* Module:        opcodes.cpp
* Description:
* Definition of opcode maps used by disassembler
*
* (c) 2007 GNU General Public License www.gnu.org/copyleft/gpl.html
*****************************************************************************/

/*************************** Define opcode maps ******************************
Interpretation of an opcode starts at OpcodeMap0 where the first opcode byte 
after any prefixes is used as index into the map. If more than one possible
instruction starts with this byte then there is a link to a secondary map. 
The criterion for indexing into the secondary map is given by TableLink. 
If TableLink is nonzero then the map number for the secondary map is given 
in the InstructionSet field. The secondary map may contain a link to a 
tertiary map, and so on.

The meaning of each field in the map entries is defined in disasm.h.

OpcodeTables[] is an array of pointers to all the maps.

OpcodeTableLength[] indicates the size of each map.

If a map is incomplete, then the last entry should indicate a default for
the missing entries, i.e. how to display the illegal or unknown instruction
codes.

New entries can be added whenever a new extension to the instruction set is
introduced.

*****************************************************************************/

#include "stdafx.h"

// Primary opcode map. This is the start of all opcode lookups
SOpcodeDef OpcodeMap0[256] = {
//  name         instset prefix  format  dest.   source  link    options
   {"add",       0,      0x10,   0x13,   0x201,  1,      0,      0  },    // 00
   {"add",       0,      0x1110, 0x13,   0x209,  9,      0,      0  },    // 01
   {"add",       0,      0,      0x12,   1,      0x201,  0,      0  },    // 02
   {"add",       0,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 03
   {"add",       0,      0,      0x41,   0xa1,   0x1000, 0,      0  },    // 04
   {"add",       0,      0x1100, 0x81,   0xa9,   0x3000, 0,     0x80},    // 05
   {"push es",   0x8000, 2,         1,   0,      0,      0,      0  },    // 06
   {"pop  es",   0x8000, 2,         1,   0,      0,      0,      0  },    // 07
   {"or",        0,      0x10,   0x13,   0x201,  1,      0,      0  },    // 08
   {"or",        0,      0x1110, 0x13,   0x209,  9,      0,      0  },    // 09
   {"or",        0,      0,      0x12,   1,      0x201,  0,      0  },    // 0A
   {"or",        0,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 0B
   {"or",        0,      0,      0x41,   0xa1,   0x5000, 0,      0  },    // 0C
   {"or",        0,      0x1100, 0x81,   0xa9,   0x7000, 0,     0x80},    // 0D
   {"push cs",   0x8000, 2,         1,   0,      0,      0,      0  },    // 0E
   {0,           1,      0,         0,   0,      0,      1,      0  },    // 0F link to OpcodeMap1
//  name         instset prefix  format  dest.   source  link    options
   {"adc",       0,      0x10,   0x13,   0x201,  1,      0,      0  },    // 10
   {"adc",       0,      0x1110, 0x13,   0x209,  9,      0,      0  },    // 11
   {"adc",       0,      0,      0x12,   1,      0x201,  0,      0  },    // 12
   {"adc",       0,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 13
   {"adc",       0,      0,      0x41,   0xa1,   0x1000, 0,      0  },    // 14
   {"adc",       0,      0x1100, 0x81,   0xa9,   0x3000, 0,     0x80},    // 15
   {"push ss",   0x8000, 2,         1,   0,      0,      0,      0  },    // 16
   {"pop  ss",   0x8000, 2,         1,   0,      0,      0,      0  },    // 17
   {"sbb",       0,      0x10,   0x13,   0x201,  1,      0,      0  },    // 18
   {"sbb",       0,      0x1110, 0x13,   0x209,  9,      0,      0  },    // 19
   {"sbb",       0,      0,      0x12,   1,      0x201,  0,      0  },    // 1A
   {"sbb",       0,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 1B
   {"sbb",       0,      0,      0x41,   0xa1,   0x1000, 0,      0  },    // 1C
   {"sbb",       0,      0x1100, 0x81,   0xa9,   0x3000, 0,     0x80},    // 1D
   {"push ds",   0x8000, 2,         1,   0,      0,      0,      0  },    // 1E
   {"pop  ds",   0x8000, 2,         1,   0,      0,      0,      0  },    // 1F
//  name         instset prefix  format  dest.   source  link    options
   {"and",       0,      0x10,   0x13,   0x201,  1,      0,      0  },    // 20
   {"and",       0,      0x1110, 0x13,   0x209,  9,      0,      0  },    // 21
   {"and",       0,      0,      0x12,   1,      0x201,  0,      0  },    // 22
   {"and",       0,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 23
   {"and",       0,      0,      0x41,   0xa1,   0x5000, 0,      0  },    // 24
   {"and",       0,      0x1100, 0x81,   0xa9,   0x7000, 0,     0x80},    // 25
   {"es:",       0,      0,    0x8001,   0,      0,      0,      0  },    // 26
   {"daa",       0x8000, 0,         2,   0,      0,      0,      8  },    // 27
   {"sub",       0,      0x10,   0x13,   0x201,  1,      0,      0  },    // 28
   {"sub",       0,      0x1110, 0x13,   0x209,  9,      0,      0  },    // 29
   {"sub",       0,      0,      0x12,   1,      0x201,  0,      0  },    // 2A
   {"sub",       0,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 2B
   {"sub",       0,      0,      0x41,   0xa1,   0x1000, 0,      0  },    // 2C
   {"sub",       0,      0x1100, 0x81,   0xa9,   0x3000, 0,     0x80},    // 2D
   {"cs:",       0,      0,    0x8001,   0,      0,      0,      0  },    // 2E
   {"das",       0x8000, 0,         2,   0,      0,      0,      8  },    // 2F
//  name         instset prefix  format  dest.   source  link    options
   {"xor",       0,      0x10,   0x13,   0x201,  1,      0,      0  },    // 30
   {"xor",       0,      0x1110, 0x13,   0x209,  9,      0,      0  },    // 31
   {"xor",       0,      0,      0x12,   1,      0x201,  0,      0  },    // 32
   {"xor",       0,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 33
   {"xor",       0,      0,      0x41,   0xa1,   0x5000, 0,      0  },    // 34
   {"xor",       0,      0x1100, 0x81,   0xa9,   0x7000, 0,     0x80},    // 35
   {"ss:",       0,      0,    0x8001,   0,      0,      0,      0  },    // 36
   {"aaa",       0x8000, 0,         2,   0,      0,      0,      8  },    // 37
   {"cmp",       0,      0x00,   0x13,   0x201,  1,      0,      4  },    // 38
   {"cmp",       0,      0x1100, 0x13,   0x209,  9,      0,      4  },    // 39
   {"cmp",       0,      0,      0x12,   1,      0x201,  0,      4  },    // 3A
   {"cmp",       0,      0x1100, 0x12,   9,      0x209,  0,      4  },    // 3B
   {"cmp",       0,      0,      0x41,   0xa1,   0x1000, 0,      4  },    // 3C
   {"cmp",       0,      0x1100, 0x81,   0xa9,   0x3000, 0,     0x84},    // 3D
   {"ds:",       0,      0,    0x8001,   0,      0,      0,      0  },    // 3E
   {"aas",       0x8000, 0,         2,   0,      0,      0,      8  },    // 3F
//  name         instset prefix  format  dest.   source  link    options
   {"inc",       0x8000, 0x100,  3,      8,      0,      0,      0  },    // 40
   {"inc",       0x8000, 0x100,  3,      8,      0,      0,      0  },    // 41
   {"inc",       0x8000, 0x100,  3,      8,      0,      0,      0  },    // 42
   {"inc",       0x8000, 0x100,  3,      8,      0,      0,      0  },    // 43
   {"inc",       0x8000, 0x100,  3,      8,      0,      0,      0  },    // 44
   {"inc",       0x8000, 0x100,  3,      8,      0,      0,      0  },    // 45
   {"inc",       0x8000, 0x100,  3,      8,      0,      0,      0  },    // 46
   {"inc",       0x8000, 0x100,  3,      8,      0,      0,      0  },    // 47
   {"dec",       0x8000, 0x100,  3,      8,      0,      0,      0  },    // 48
   {"dec",       0x8000, 0x100,  3,      8,      0,      0,      0  },    // 49
   {"dec",       0x8000, 0x100,  3,      8,      0,      0,      0  },    // 4A
   {"dec",       0x8000, 0x100,  3,      8,      0,      0,      0  },    // 4B
   {"dec",       0x8000, 0x100,  3,      8,      0,      0,      0  },    // 4C
   {"dec",       0x8000, 0x100,  3,      8,      0,      0,      0  },    // 4D
   {"dec",       0x8000, 0x100,  3,      8,      0,      0,      0  },    // 4E
   {"dec",       0x8000, 0x100,  3,      8,      0,      0,      0  },    // 4F
//  name         instset prefix  format  dest.   source  link    options
   {"push",      0,      0x2102, 3,      0x0A,   0,      0,      4  },    // 50
   {"push",      0,      0x2102, 3,      0x0A,   0,      0,      4  },    // 51
   {"push",      0,      0x2102, 3,      0x0A,   0,      0,      4  },    // 52
   {"push",      0,      0x2102, 3,      0x0A,   0,      0,      4  },    // 53
   {"push",      0,      0x2102, 3,      0x0A,   0,      0,      4  },    // 54
   {"push",      0,      0x2102, 3,      0x0A,   0,      0,      4  },    // 55
   {"push",      0,      0x2102, 3,      0x0A,   0,      0,      4  },    // 56
   {"push",      0,      0x2102, 3,      0x0A,   0,      0,      4  },    // 57
   {"pop",       0,      0x2102, 3,      0x0A,   0,      0,      0  },    // 58
   {"pop",       0,      0x2102, 3,      0x0A,   0,      0,      0  },    // 59
   {"pop",       0,      0x2102, 3,      0x0A,   0,      0,      0  },    // 5A
   {"pop",       0,      0x2102, 3,      0x0A,   0,      0,      0  },    // 5B
   {"pop",       0,      0x2102, 3,      0x0A,   0,      0,      0  },    // 5C
   {"pop",       0,      0x2102, 3,      0x0A,   0,      0,      0  },    // 5D
   {"pop",       0,      0x2102, 3,      0x0A,   0,      0,      0  },    // 5E
   {"pop",       0,      0x2102, 3,      0x0A,   0,      0,      0  },    // 5F
//  name         instset prefix  format  dest.   source  link    options
   {"pusha",     0x8001, 0x102,  2,      0,      0,      0,      5  },    // 60
   {"popa",      0x8001, 0x102,  2,      0,      0,      0,      9  },    // 61
   {"bound",     0x8001, 0x106,  0x12,   8,      0x109,  0,      0  },    // 62
   {"arpl/movsxd", 0x3B, 0,      0,      0,      0,      7,      0  },    // 63 Link to map 0x33
   {"fs:",       0,      0,    0x8001,   0,      0,      0,      0  },    // 64
   {"gs:",       0,      0,    0x8001,   0,      0,      0,      0  },    // 65
   {"operand size:", 0,  0,    0x8000,   0,      0,      0,      0  },    // 66
   {"address size:", 0,  0,    0x8000,   0,      0,      0,      0  },    // 67
   {"push",      0,      0x2102, 0x82,   0,      0x3000, 0,     0x80},    // 68 push imm word
   {"imul",      1,      0x1100, 0x92,   9,      0x2209, 0,     0x80},    // 69 imul r,m,iv
   {"push",      0,      0x2102, 0x42,   0,      0x2000, 0,      0  },    // 6A push imm byte
   {"imul",      1,      0x1100, 0x52,   9,      0x2209, 0,      0  },    // 6B imul r,m,ib
   {"insb",      0,      0x021,  1,      0,      0,      0,      8  },    // 6C insb
   {"ins",       0,      0x121,  1,      0xc2,   0xb2,   0,      8  },    // 6D insw
   {"outsb",     0,      0x021,  1,      0,      0,      0,      8  },    // 6E outsb
   {"outs",      0,      0x121,  1,      0xb2,   0xc2,   0,      8  },    // 6F outs
//  name         instset prefix  format  dest.   source  link    options
   {"jo",        0,      0x88,   0x42,   0x81,   0,      0,      0  },    // 70 conditional short jumps
   {"jno",       0,      0x88,   0x42,   0x81,   0,      0,      0  },    // 71
   {"jc",        0,      0x88,   0x42,   0x81,   0,      0,      0  },    // 72
   {"jnc",       0,      0x88,   0x42,   0x81,   0,      0,      0  },    // 73
   {"jz",        0,      0x88,   0x42,   0x81,   0,      0,      0  },    // 74
   {"jnz",       0,      0x88,   0x42,   0x81,   0,      0,      0  },    // 75
   {"jbe",       0,      0x88,   0x42,   0x81,   0,      0,      0  },    // 76
   {"ja",        0,      0x88,   0x42,   0x81,   0,      0,      0  },    // 77
   {"js",        0,      0x88,   0x42,   0x81,   0,      0,      0  },    // 78
   {"jns",       0,      0x88,   0x42,   0x81,   0,      0,      0  },    // 79
   {"jpe",       0,      0x88,   0x42,   0x81,   0,      0,      0  },    // 7A
   {"jpo",       0,      0x88,   0x42,   0x81,   0,      0,      0  },    // 7B
   {"jl",        0,      0x88,   0x42,   0x81,   0,      0,      0  },    // 7C
   {"jge",       0,      0x88,   0x42,   0x81,   0,      0,      0  },    // 7D
   {"jle",       0,      0x88,   0x42,   0x81,   0,      0,      0  },    // 7E
   {"jg",        0,      0x88,   0x42,   0x81,   0,      0,      0  },    // 7F
//  name         instset prefix  format  dest.   source  link    options
   {"grp1",      0x1A,   0,      0,      0,      0,      2,      0  },    // 80 link to immediate grp 1
   {"grp1",      0x1B,   0,      0,      0,      0,      2,      0  },    // 81 link to immediate grp 1
   {"grp1",      0x1C,   0,      0,      0,      0,      2,      0  },    // 82 link to immediate grp 1
   {"grp1",      0x1D,   0,      0,      0,      0,      2,      0  },    // 83 link to immediate grp 1
   {"test",      0,      0,      0x13,   0x201,  1,      0,      4  },    // 84
   {"test",      0,      0x1100, 0x13,   0x209,  9,      0,      4  },    // 85
   {"xchg",      0,      0x0010, 0x13,   0x201,  1,      0,     0x48},    // 86
   {"xchg",      0,      0x1110, 0x13,   0x209,  9,      0,     0x48},    // 87
   {"mov",       0,      0,      0x13,   0x201,  1,      0,     0x40},    // 88
   {"mov",       0,      0x1100, 0x13,   0x209,  9,      0,     0x40},    // 89
   {"mov",       0,      0,      0x12,   1,      0x201,  0,     0x40},    // 8A
   {"mov",       0,      0x1100, 0x12,   9,      0x209,  0,     0x40},    // 8B
   {"mov",       0,      0x1100, 0x13,   0x209,  0x91,   0,      0  },    // 8C mov r16,segreg
   {"lea",       0,      0x1101, 0x12,   9,      0x109,  0,     0xC0},    // 8D
   {"mov",       0,      0x1100, 0x12,   0x91,   0x209,  0,      0  },    // 8E mov segreg,r16
   {"pop",       0x28,   0,      0,      0,      0,      2,      0  },    // 8F Link to group 1A
//  name         instset prefix  format  dest.   source  link    options
   {"nop",       0x3C,   0,      0,      0,      0,      9,      0  },    // 90 NOP/Pause. Link to map
   {"xchg",      0,      0x1100, 3,      9,      0xa9,   0,      8  },    // 91 xchg cx,ax
   {"xchg",      0,      0x1100, 3,      9,      0xa9,   0,      8  },    // 92 xchg dx,ax
   {"xchg",      0,      0x1100, 3,      9,      0xa9,   0,      8  },    // 93 xchg bx,ax
   {"xchg",      0,      0x1100, 3,      9,      0xa9,   0,      8  },    // 94 xchg sp,ax
   {"xchg",      0,      0x1100, 3,      9,      0xa9,   0,      8  },    // 95 xchg bp,ax
   {"xchg",      0,      0x1100, 3,      9,      0xa9,   0,      8  },    // 96 xchg si,ax
   {"xchg",      0,      0x1100, 3,      9,      0xa9,   0,      8  },    // 97 xchg di,ax
   {"cbw",       0x39,   0x1100, 0,      0,      0,      8,      0  },    // 98 Link to map
   {"cwd",       0x3A,   0x1100, 0,      0,      0,      8,      0  },    // 99 Link to map
   {"call",      0x8000, 0x182,  0x200,  0x85,   0,      0,      8  },    // 9A call far
   {"fwait",     0x100,  0,      2,      0,      0,      0,      0  },    // 9B
   {"pushf",     0x3E,   0x2100, 0,      0,      0,      8,      0  },    // 9C Link to map: pushf/d/q
   {"popf",      0x3F,   0x2100, 0,      0,      0,      8,      0  },    // 9D Link to map: popf/d/q
   {"sahf",      0,      0,      2,      0,      0,      0,      0  },    // 9E
   {"lahf",      0,      0,      2,      0,      0,      0,      8  },    // 9F
//  name         instset prefix  format  dest.   source  link    options
   {"mov",       0,      5,      0x401,  0xa1,   0x101,  0,      0  },    // A0 mov al,mem
   {"mov",       0,      0x1105, 0x401,  0xa9,   0x109,  0,      0  },    // A1 mov ax,mem
   {"mov",       0,      5,      0x401,  0x101,  0xa1,   0,      0  },    // A2 mov mem,al
   {"mov",       0,      0x1105, 0x401,  0x109,  0xa9,   0,      0  },    // A3 mov mem,ax
   {"movs",      0,      0x25,   1,      0xc2,   0xc1,   0,      8  },    // A4 movsb
   {"movs",      0,      0x1125, 1,      0xc2,   0xc1,   0,      8  },    // A5 movsw
   {"cmps",      0,      0x45,   1,      0xc2,   0xc1,   0,      8  },    // A6 cmpsb
   {"cmps",      0,      0x1145, 1,      0xc2,   0xc1,   0,      8  },    // A7 cmpsw
   {"test",      0,      0,      0x41,   0xa1,   0x5000, 0,      4  },    // A8 test al,ib
   {"test",      0,      0x1100, 0x81,   0xa9,   0x7000, 0,      4  },    // A9 test ax,iw
   {"stos",      0,      0x21,   1,      0xc2,   0,      0,      8  },    // AA stosb
   {"stos",      0,      0x1121, 1,      0xc2,   0,      0,      8  },    // AB stosw
   {"lods",      0,      0x25,   1,      0,      0xc1,   0,      8  },    // AC lodsb
   {"lods",      0,      0x1125, 1,      0,      0xc1,   0,      8  },    // AD lodsw
   {"scas",      0,      0x41,   1,      0xc2,   0,      0,      8  },    // AE scasb
   {"scas",      0,      0x1141, 1,      0xc2,   0,      0,      8  },    // AF scasw
//  name         instset prefix  format  dest.   source  link    options
   {"mov",       0,      0,      0x43,   1,      0x1000, 0,      0  },    // B0 mov al,ib
   {"mov",       0,      0,      0x43,   1,      0x1000, 0,      0  },    // B1 mov cl,ib
   {"mov",       0,      0,      0x43,   1,      0x1000, 0,      0  },    // B2 mov dl,ib
   {"mov",       0,      0,      0x43,   1,      0x1000, 0,      0  },    // B3 mov bl,ib
   {"mov",       0,      0,      0x43,   1,      0x1000, 0,      0  },    // B4 mov ah,ib
   {"mov",       0,      0,      0x43,   1,      0x1000, 0,      0  },    // B5 mov ch,ib
   {"mov",       0,      0,      0x43,   1,      0x1000, 0,      0  },    // B6 mov dh,ib
   {"mov",       0,      0,      0x43,   1,      0x1000, 0,      0  },    // B7 mov bh,ib
   {"mov",       0,      0x1100, 0x103,  9,      0x1000, 0,      0  },    // B8 mov ax,iw
   {"mov",       0,      0x1100, 0x103,  9,      0x1000, 0,      0  },    // B9 mov cx,iw
   {"mov",       0,      0x1100, 0x103,  9,      0x1000, 0,      0  },    // BA mov dx,iw
   {"mov",       0,      0x1100, 0x103,  9,      0x1000, 0,      0  },    // BB mov bx,iw
   {"mov",       0,      0x1100, 0x103,  9,      0x1000, 0,      0  },    // BC mov sp,iw
   {"mov",       0,      0x1100, 0x103,  9,      0x1000, 0,      0  },    // BD mov bp,iw
   {"mov",       0,      0x1100, 0x103,  9,      0x1000, 0,      0  },    // BE mov si,iw
   {"mov",       0,      0x1100, 0x103,  9,      0x1000, 0,      0  },    // BF mov di,iw
//  name         instset prefix  format  dest.   source  link    options
   {"grp2",      0x1E,   0,      0,      0,      0,      2,      0  },    // C0 link to grp 2
   {"grp2",      0x1F,   0,      0,      0,      0,      2,      0  },    // C1 link to grp 2
   {"ret",       0,      0x102,  0x22,   0,      0x1000, 0,     0x10},    // C2 retn iw
   {"ret",       0,      0x102,  2,      0,      0,      0,     0x10},    // C3 retn
   {"les",       0x8000, 0x100,  0x812,  9,      0x10c,  0,      0  },    // C4 les
   {"lds",       0x8000, 0x100,  0x812,  9,      0x10c,  0,      0  },    // C5 lds
   {"mov",       0x2F,   0,      0,      0,      0,      2,      0  },    // C6 link to grp 11
   {"mov",       0x30,   0,      0,      0,      0,      2,      0  },    // C7 link to grp 11
   {"enter",     0,      0,      0x62,   0,      0x1000, 0,      8  },    // C8
   {"leave",     0,      0,      2,      0,      0,      0,      8  },    // C9
   {"retf",      0,      2,      0x22,   0,      0x1000, 0,     0x10},    // CA retf iw
   {"retf",      0,      2,      2,      0,      0,      0,     0x10},    // CB retf
   {"int 3;breakpoint or filler",0,0,2,  0,      0,      0,     0x48},    // CC
   {"int",       0,      0,      0x42,   0,      0x1000, 0,      8  },    // CD
   {"into",      0x8000, 0,      2,      0,      0,      0,      0  },    // CE
   {0,           0x19,   0x1100, 0,      0,      0,      8,      0  },    // CF link to IRET
//  name         instset prefix  format  dest.   source  link    options
   {"grp2",      0x20,   0,      0,      0,      0,      2,      0  },    // D0 link to grp 2
   {"grp2",      0x21,   0,      0,      0,      0,      2,      0  },    // D1 link to grp 2
   {"grp2",      0x22,   0,      0,      0,      0,      2,      0  },    // D2 link to grp 2
   {"grp2",      0x23,   0,      0,      0,      0,      2,      0  },    // D3 link to grp 2
   {"aam",       0x8000, 0,   0x42,      0,      0x1000, 0,      8  },    // D4. Don't show immediate operand if = 10 !
   {"aad",       0x8000, 0,   0x42,      0,      0x1000, 0,      8  },    // D5. Don't show immediate operand if = 10 !
   {"salc",      0x8000, 0,   0x4002,    0,      0,      0,      0  },    // D6 salc (undocumented opcode)
   {"xlat",      0,      5,      1,      0,      0xc0,   0,      8  },    // D7
   {"x87 instr", 0x8,    0,      0,      0,      0,      4,      0  },    // D8 link to FP grp
   {"x87 instr", 0x9,    0,      0,      0,      0,      4,      0  },    // D9 link to FP grp
   {"x87 instr", 0xA,    0,      0,      0,      0,      4,      0  },    // DA link to FP grp
   {"x87 instr", 0xB,    0,      0,      0,      0,      4,      0  },    // DB link to FP grp
   {"x87 instr", 0xC,    0,      0,      0,      0,      4,      0  },    // DC link to FP grp
   {"x87 instr", 0xD,    0,      0,      0,      0,      4,      0  },    // DD link to FP grp
   {"x87 instr", 0xE,    0,      0,      0,      0,      4,      0  },    // DE link to FP grp
   {"x87 instr", 0xF,    0,      0,      0,      0,      4,      0  },    // DF link to FP grp
//  name         instset prefix  format  dest.   source  link    options
   {"loopne",    0,      0x80,   0x42,   0x81,   0,      0,      8  },    // E0
   {"loope",     0,      0x80,   0x42,   0x81,   0,      0,      8  },    // E1
   {"loop",      0,      0x80,   0x42,   0x81,   0,      0,      8  },    // E2
   {"j(e/r)cxz", 0x3D,   0,      0,      0,      0,      0xA,    0  },    // E3 link to map
   {"in",        0x800,  0,      0x41,   0xa1,   0x1000, 0,      0  },    // E4 in al,ib
   {"in",        0x800,  0x100,  0x41,   0xa8,   0x1000, 0,      0  },    // E5 in ax,ib
   {"out",       0x800,  0,      0x41,   0x1000, 0xa1,   0,      0  },    // E6 out ib,al
   {"out",       0x800,  0x100,  0x41,   0x1000, 0xa8,   0,      0  },    // E7 out ib,ax
   {"call",      0,      0x82,   0x82,   0x83,   0,      0,      8  },    // E8 call near
   {"jmp",       0,      0x80,   0x82,   0x82,   0,      0,     0x90},    // E9 jmp near
   {"jmp",       0x8000, 0x80,   0x202,  0x84,   0,      0,     0x10},    // EA jmp far
   {"jmp",       0,      0x80,   0x42,   0x81,   0,      0,     0x10},    // EB jmp short
   {"in",        0x800,  0,      1,      0xa1,   0xb2,   0,      0  },    // EC in al,dx
   {"in",        0x800,  0x100,  1,      0xa8,   0xb2,   0,      0  },    // ED in ax,dx
   {"out",       0x800,  0,      1,      0xb2,   0xa1,   0,      0  },    // EE out dx,al
   {"out",       0x800,  0x100,  1,      0xb2,   0xa8,   0,      0  },    // EF out dx,ax
//  name         instset prefix  format  dest.   source  link    options
   {"lock:",     0,      0,      0x8000, 0,      0,      0,      0  },    // F0 lock prefix
   {"icebp",     0x8000, 0,      0x4002, 0,      0,      0,      0  },    // F1 ICE breakpoint, undocumented opcode
   {"repne:",    0,      0,      0x8000, 0,      0,      0,      0  },    // F2 repne prefix
   {"repe:",     0,      0,      0x8000, 0,      0,      0,      0  },    // F3 repe  prefix
   {"hlt",       0,      0,      2,      0,      0,      0,     0x48},    // F4
   {"cmc",       0,      0,      2,      0,      0,      0,      0  },    // F5
   {"grp3",      0x24,   0,      0,      0,      0,      2,      0  },    // F6 link to grp 3
   {"grp3",      0x25,   0,      0,      0,      0,      2,      0  },    // F7 link to grp 3
   {"clc",       0,      0,      2,      0,      0,      0,      0  },    // F8
   {"stc",       0,      0,      2,      0,      0,      0,      0  },    // F9
   {"cli",   0x800,      0,      2,      0,      0,      0,      0  },    // FA
   {"sti",   0x800,      0,      2,      0,      0,      0,      0  },    // FB
   {"cld",       0,      0,      2,      0,      0,      0,      0  },    // FC
   {"std",       0,      0,      2,      0,      0,      0,      0  },    // FD
   {"grp4",      0x26,   0,      0,      0,      0,      2,      0  },    // FE link to grp 4
   {"grp5",      0x27,   0,      0,      0,      0,      2,      0  }     // FF link to grp 5
};

// Secondary opcode map for 2-byte opcode. First byte = 0F
// Indexed by second opcode byte
SOpcodeDef OpcodeMap1[256] = {
//  name         instset prefix  format  dest.   source  link    options
   {"grp6",      0x2A,   0,      0,      0,      0,      2,      0  },    // 0F 00 link to grp 6; sldt etc.
   {"grp7",      0x2B,   0,      0,      0,      0,      4,      0  },    // 0F 01 link to grp 7; sgdt etc.
   {0,           0x5E,   0,      0,      0,      0,      3,      0  },    // 0F 02 link to lar
   {0,           0x5F,   0,      0,      0,      0,      3,      0  },    // 0F 03 link to lsl
   {0,           0,      0,      0,      0,      0,      0,      0  },    // Illegal
   {"syscall",   5,      0,      1,      0,      0,      0,      8  },    // 0F 05
   {"clts",      0x805,  0,      1,      0,      0,      0,      0  },    // 0F 06
   {"sysret",    0x805,  0,      1,      0,      0,      0,     0x10},    // 0F 07
   {"invd",      0x804,  0,      1,      0,      0,      0,      0  },    // 0F 08
   {"wbinvd",    0x804,  0,      1,      0,      0,      0,      0  },    // 0F 09
   {0,           0,      0,      0,      0,      0,      0,      0  },    // Illegal
   {"ud2",       3,      0,      1,      0,      0,      0,     0x10},    // 0F 0B
   {0,           0,      0,      0,      0,      0,      0,      0  },    // Illegal
   {"PREFETCH;AMD only",0x1000,0,0x11,   0,      0x106,  0,      0  },    // 0F 0D. AMD only. Works as NOP on Intel
   {"FEMS",      0x1000, 0,      2,      0,      0,      0,      0  },    // 0F OE. AMD only
   {0,           6,      0,      0x52,   0,      0,      6,      0  },    // 0F 0F. Link to tertiary map for AMD 3DNow instructions
//  name         instset prefix  format  dest.   source  link    options
   {0,           0x40,   0,      0,      0,      0,      9,      0  },    // Link to tertiary map: movups, etc.
   {0,           0x41,   0,      0,      0,      0,      9,      0  },    // Link to tertiary map: movups, etc.
   {0,           0x42,   0,      0,      0,      0,      9,      0  },    // Link to tertiary map: movlps, etc.
   {"movl",      0x11,   0x200,  0x13,   0x110,  0x6F,   0,      1  },    // 0F 13 movlps/pd
   {"unpckl",    0x11,   0x200,  0x12,   0x6F,   0x26F,  0,      1  },    // 0F 14 unpcklps/pd
   {"unpckh",    0x11,   0x200,  0x12,   0x6F,   0x26F,  0,      1  },    // 0F 15 unpckhps/pd
   {0,           0x44,   0,      0,      0,      0,      9,      0  },    // Link to tertiary map: movhps, etc.
   {"movh",      0x11,   0x200,  0x13,   0x110,  0x6F,   0,      1  },    // 0F 17 movhps/pd
   {0,           0x35,   0,      0,      0,      0,      2,      0  },    // Link to tertiary map: group 16
   {"hint",      6,      0,      0x2012, 0,      0x206,  0,      0  },    // 0F 19. Hint instructions reserved for future use
   {"hint",      6,      0,      0x2012, 0,      0x206,  0,      0  },    // 0F 1A. Hint instructions reserved for future use
   {"hint",      6,      0,      0x2012, 0,      0x206,  0,      0  },    // 0F 1B. Hint instructions reserved for future use
   {"hint",      6,      0,      0x2012, 0,      0x206,  0,      0  },    // 0F 1C. Hint instructions reserved for future use
   {"hint",      6,      0,      0x2012, 0,      0x206,  0,      0  },    // 0F 1D. Hint instructions reserved for future use
   {"hint",      6,      0,      0x2012, 0,      0x206,  0,      0  },    // 0F 1E. Hint instructions reserved for future use
   {"nop",       6,      0,      0x11,   0,      0x200,  0,     0x40},    // 0F 1F. Multi-byte nop
//  name         instset prefix  format  dest.   source  link    options
   {"mov",       0x803,  0,      0x13,   0xA,    0x92,   0,      0  },    // 0F 20. mov r32/64,cr
   {"mov",       0x803,  0x1000, 0x13,   9,      0x93,   0,      0  },    // 0F 21. mov r32,dr
   {"mov",       0x803,  0,      0x12,   0x92,   0xA,    0,      0  },    // 0F 22. mov cr,r32/64
   {"mov",       0x803,  0x1000, 0x12,   0x93,   9,      0,      0  },    // 0F 23. mov dr,r32
   {"mov",       0x803,  0,      0x4013, 3,      0x94,   0,      0  },    // 0F 24. mov r32,tr
   {0,           0x803,  0,      0x4013, 3,      3,      0,      0  },    // 0F 25. illegal
   {"mov",       0x803,  0,      0x4012, 0x94,   3,      0,      0  },    // 0F 26. mov tr,r32
   {0,           0x803,  0,      0x4012, 3,      3,      0,      0  },    // 0F 27. illegal
   {"mova",      0x11,   0x200,  0x12,   0x6F,   0x26F,  0,      1  },    // 0F 28. movaps/pd
   {"mova",      0x11,   0x200,  0x13,   0x26F,  0x6F,   0,      1  },    // 0F 29. movaps/pd
   {0,           0x46,   0,      0,      0,      0,      9,      0  },    // 0F 2A. Link to tertiary map: cvtpi2ps, etc.
   {"movnt",     0x11,   0x200,  0x13,   0x16F,  0x6F,   0,      1  },    // 0F 2B. movntps/pd
   {0,           0x47,   0,      0,      0,      0,      9,      0  },    // 0F 2C. Link to tertiary map: cvttps2pi, etc.
   {0,           0x48,   0,      0,      0,      0,      9,      0  },    // 0F 2D. Link to tertiary map: cvtps2pi, etc.
   {0,           0x4B,   0,      0,      0,      0,      9,      0  },    // 0F 2E. Link to tertiary map: ucomiss/sd
   {0,           0x4C,   0,      0,      0,      0,      9,      0  },    // 0F 2F. Link to tertiary map: comiss/sd
//  name         instset prefix  format  dest.   source  link    options
   {"wrmsr",     0x805,  0x1000, 1,      0,      0,      0,      0  },    // 0F 30
   {"rdtsc",     5,      0,      1,      0,      0,      0,      8  },    // 0F 31
   {"rdmsr",     0x805,  0x1000, 1,      0,      0,      0,      8  },    // 0F 32
   {"rdpmc",     5,      0,      1,      0,      0,      0,      8  },    // 0F 33
   {"sysenter",  8,      0,      1,      0,      0,      0,      8  },    // 0F 34
   {"sysexit;Same name with or without 48h prefix",0x808,0x1000,1,0,0,0,0},// 0F 35
   {0,           0,      0,      0,      0,      0,      0,      0  },    // 0F 36 Illegal
   {0,           0,      0,      0,      0,      0,      0,      0  },    // 0F 37 Illegal
   {0,           0x2,    0,      0,      0,      0,      1,      0  },    // 0F 38. Link to tertiary map for 3-byte opcodes
   {0,           0x3,    0,      0,      0,      0,      1,      0  },    // 0F 39. Link to tertiary map for 3-byte opcodes
   {0,           0x4,    0,      0,      0,      0,      1,      0  },    // 0F 3A. Link to tertiary map for 3-byte opcodes
   {0,           0x5,    0,      0,      0,      0,      1,      0  },    // 0F 3B. Link to tertiary map for 3-byte opcodes
   {0,           0,      0,      0,      0,      0,      0,      0  },    // 0F 3C Illegal
   {0,           0,      0,      0,      0,      0,      0,      0  },    // 0F 3D Illegal
   {0,           0,      0,      0,      0,      0,      0,      0  },    // 0F 3E Illegal
   {0,           0,      0,      0,      0,      0,      0,      0  },    // 0F 3F Illegal
//  name         instset prefix  format  dest.   source  link    options
   {"cmovo",     6,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 0F 40. cmov
   {"cmovno",    6,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 0F 41. cmov
   {"cmovc",     6,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 0F 42. cmov
   {"cmovnc",    6,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 0F 43. cmov
   {"cmove",     6,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 0F 44. cmov
   {"cmovne",    6,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 0F 45. cmov
   {"cmovbe",    6,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 0F 46. cmov
   {"cmova",     6,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 0F 47. cmov
   {"cmovs",     6,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 0F 48. cmov
   {"cmovns",    6,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 0F 49. cmov
   {"cmovpe",    6,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 0F 4A. cmov
   {"cmovpo",    6,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 0F 4B. cmov
   {"cmovl",     6,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 0F 4C. cmov
   {"cmovge",    6,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 0F 4D. cmov
   {"cmovle",    6,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 0F 4E  cmov
   {"cmovg",     6,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 0F 4F. cmov
//  name         instset prefix  format  dest.   source  link    options
   {"movmsk",    0x11,   0x1200, 0x12,   9,      0x6F,   0,      1  },    // 0F 50. movmskps/pd
   {"sqrt",      0x11,   0xE00,  0x12,   0x6F,   0x26F,  0,      1  },    // 0F 51. sqrtps/pd/ss/sd
   {"rsqrt",     0x11,   0x400,  0x12,   0x6F,   0x26F,  0,      1  },    // 0F 52. rsqrtps/ss
   {"rcp",       0x11,   0x400,  0x12,   0x6F,   0x26F,  0,      1  },    // 0F 53. rcpps/ss
   {"and",       0x11,   0x200,  0x12,   0x6F,   0x26F,  0,      1  },    // 0F 54. andps/pd
   {"andn",      0x11,   0x200,  0x12,   0x6F,   0x26F,  0,      1  },    // 0F 55. andnps/pd
   {"or",        0x11,   0x200,  0x12,   0x6F,   0x26F,  0,      1  },    // 0F 56. orps/pd
   {"xor",       0x11,   0x200,  0x12,   0x6F,   0x26F,  0,      1  },    // 0F 57. xorps/pd
   {"add",       0x11,   0xE00,  0x12,   0x6F,   0x26F,  0,      1  },    // 0F 58. addps/pd/ss/sd
   {"mul",       0x11,   0xE00,  0x12,   0x6F,   0x26F,  0,      1  },    // 0F 59. mulps/pd/ss/sd
   {0,           0x49,   0,      0,      0,      0,      9,      0  },    // 0F 5A. Link to tertiary map: cvtps2pd, etc.
   {0,           0x4A,   0,      0,      0,      0,      9,      0  },    // 0F 5B. Link to tertiary map: cvtdq2ps, etc.
   {"sub",       0x11,   0xE00,  0x12,   0x6F,   0x26F,  0,      1  },    // 0F 5C. subps/pd/ss/sd
   {"min",       0x11,   0xE00,  0x12,   0x6F,   0x26F,  0,      1  },    // 0F 5D. minps/pd/ss/sd
   {"div",       0x11,   0xE00,  0x12,   0x6F,   0x26F,  0,      1  },    // 0F 5E. divps/pd/ss/sd
   {"max",       0x11,   0xE00,  0x12,   0x6F,   0x26F,  0,      1  },    // 0F 5F. maxps/pd/ss/sd
//  name         instset prefix  format  dest.   source  link    options
   {"punpcklbw", 7,      0x200,  0x12,   0x31,   0x231,  0,      0  },    // 0F 60
   {"punpcklwd", 7,      0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F 61
   {"punpckldq", 7,      0x200,  0x12,   0x33,   0x233,  0,      0  },    // 0F 62
   {"packsswb",  7,      0x200,  0x12,   0x31,   0x232,  0,      0  },    // 0F 63
   {"pcmpgtb",   7,      0x200,  0x12,   0x31,   0x231,  0,      0  },    // 0F 64
   {"pcmpgtw",   7,      0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F 65
   {"pcmpgtd",   7,      0x200,  0x12,   0x33,   0x233,  0,      0  },    // 0F 66
   {"packuswb",  7,      0x200,  0x12,   0x31,   0x232,  0,      0  },    // 0F 67
   {"punpckhbw", 7,      0x200,  0x12,   0x31,   0x231,  0,      0  },    // 0F 68
   {"punpckhwd", 7,      0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F 69
   {"punpckhdq", 7,      0x200,  0x12,   0x33,   0x233,  0,      0  },    // 0F 6A
   {"packssdw",  7,      0x200,  0x12,   0x32,   0x233,  0,      0  },    // 0F 6B
   {"punpcklqdq",0x12,   0x8200, 0x12,   0x24,   0x224,  0,      0  },    // 0F 6C. Not valid without 66 prefix
   {"punpckhqdq",0x12,   0x8200, 0x12,   0x24,   0x224,  0,      0  },    // 0F 6D. Not valid without 66 prefix
   {0,           0x58,   0,      0,      0,      0,      8,      0  },    // 0F 6E. Link to tertiary map: movd/movq
   {0,           0x4D,   0,      0,      0,      0,      9,      0  },    // 0F 6F. Link to tertiary map: movq/movdqa/movdqu
//  name         instset prefix  format  dest.   source  link    options
   {0,           0x4F,   0,      0,      0,      0,      9,      0  },    // 0F 70. Link to tertiary map: pshufw, etc.
   {0,           0x31,   0,      0,      0,      0,      2,      0  },    // 0F 71. Link to tertiary map for group 12
   {0,           0x32,   0,      0,      0,      0,      2,      0  },    // 0F 72. Link to tertiary map for group 13
   {0,           0x33,   0,      0,      0,      0,      2,      0  },    // 0F 73. Link to tertiary map for group 14
   {"pcmpeqb",   7,      0x200,  0x12,   0x31,   0x231,  0,      0  },    // 0F 74
   {"pcmpeqw",   7,      0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F 75
   {"pcmpeqd",   7,      0x200,  0x12,   0x33,   0x233,  0,      0  },    // 0F 76
   {"emms",      7,      0,      2,      0,      0,      0,      0  },    // 0F 77
   {"vmread",    0x813,  0x1000, 0x13,   0x204,  4,      0,      0  },    // 0F 78
   {"vmread",    0x813,  0x1000, 0x12,   4,      0x204,  0,      0  },    // 0F 79
   {0,           0,      0,      0,      0,      0,      0,      0  },    // 0F 7A. Illegal
   {0,           0,      0,      0,      0,      0,      0,      0  },    // 0F 7B. Illegal
   {0,           0x5C,   0xA00,  0,      0,      0,      9,      0  },    // 0F 7C. Link to tertiary map: hadd
   {0,           0x5D,   0xA00,  0,      0,      0,      9,      0  },    // 0F 7D. Link to tertiary map: hsub
   {0,           0x59,   0,      0,      0,      0,      9,      0  },    // 0F 7E. Link to tertiary map: movd/movq
   {0,           0x4E,   0,      0,      0,      0,      9,      0  },    // 0F 7F. Link to tertiary map: movq/movdqa/movdqu
//  name         instset prefix  format  dest.   source  link    options
   {"jo",        3,      8,      0x80,   0x82,   0,      0,     0x80},    // 0F 80
   {"jno",       3,      8,      0x80,   0x82,   0,      0,     0x80},    // 0F 81
   {"jb",        3,      8,      0x80,   0x82,   0,      0,     0x80},    // 0F 82
   {"jae",       3,      8,      0x80,   0x82,   0,      0,     0x80},    // 0F 83
   {"je",        3,      8,      0x80,   0x82,   0,      0,     0x80},    // 0F 84
   {"jne",       3,      8,      0x80,   0x82,   0,      0,     0x80},    // 0F 85
   {"jbe",       3,      8,      0x80,   0x82,   0,      0,     0x80},    // 0F 86
   {"ja",        3,      8,      0x80,   0x82,   0,      0,     0x80},    // 0F 87
   {"js",        3,      8,      0x80,   0x82,   0,      0,     0x80},    // 0F 88
   {"jns",       3,      8,      0x80,   0x82,   0,      0,     0x80},    // 0F 89
   {"jpe",       3,      8,      0x80,   0x82,   0,      0,     0x80},    // 0F 8A
   {"jpo",       3,      8,      0x80,   0x82,   0,      0,     0x80},    // 0F 8B
   {"jl",        3,      8,      0x80,   0x82,   0,      0,     0x80},    // 0F 8C
   {"jge",       3,      8,      0x80,   0x82,   0,      0,     0x80},    // 0F 8D
   {"jle",       3,      8,      0x80,   0x82,   0,      0,     0x80},    // 0F 8E
   {"jg",        3,      8,      0x80,   0x82,   0,      0,     0x80},    // 0F 8F
//  name         instset prefix  format  dest.   source  link    options
   {"seto",      3,      0,      0x11,   0x201,  0,      0,      0  },    // 0F 90
   {"setno",     3,      0,      0x11,   0x201,  0,      0,      0  },    // 0F 91
   {"setb",      3,      0,      0x11,   0x201,  0,      0,      0  },    // 0F 92
   {"setae",     3,      0,      0x11,   0x201,  0,      0,      0  },    // 0F 93
   {"sete",      3,      0,      0x11,   0x201,  0,      0,      0  },    // 0F 94
   {"setne",     3,      0,      0x11,   0x201,  0,      0,      0  },    // 0F 95
   {"setbe",     3,      0,      0x11,   0x201,  0,      0,      0  },    // 0F 96
   {"seta",      3,      0,      0x11,   0x201,  0,      0,      0  },    // 0F 97
   {"sets",      3,      0,      0x11,   0x201,  0,      0,      0  },    // 0F 98
   {"setns",     3,      0,      0x11,   0x201,  0,      0,      0  },    // 0F 99 
   {"setpe",     3,      0,      0x11,   0x201,  0,      0,      0  },    // 0F 9A
   {"setpo",     3,      0,      0x11,   0x201,  0,      0,      0  },    // 0F 9B
   {"setl",      3,      0,      0x11,   0x201,  0,      0,      0  },    // 0F 9C
   {"setge",     3,      0,      0x11,   0x201,  0,      0,      0  },    // 0F 9D
   {"setle",     3,      0,      0x11,   0x201,  0,      0,      0  },    // 0F 9E
   {"setg",      3,      0,      0x11,   0x201,  0,      0,      0  },    // 0F 9F
//  name         instset prefix  format  dest.   source  link    options
   {"push fs",   3,      2,      1,      0,      0,      0,      0  },    // 0F A0
   {"pop  fs",   3,      2,      1,      0,      0,      0,      0  },    // 0F A1
   {"cpuid",     4,      0,      1,      0,      0,      0,      8  },    // 0F A2
   {"bt",        3,      0x1100, 0x13,   0x209,  9,      0,      0  },    // 0F A3
   {"shld",      3,      0x1100, 0x53,   0x209,  0x1009, 0,      0  },    // 0F A4
   {"shld",      3,      0x1100, 0x13,   0x209,  0x0809, 0,      0  },    // 0F A5
   {0,           0,      0,      0,      0,      0,      0,      0  },    // 0F A6. Illegal
   {0,           0,      0,      0,      0,      0,      0,      0  },    // 0F A7. Illegal
   {"push gs",   3,      2,      1,      0,      0,      0,      0  },    // 0F A8
   {"pop  gs",   3,      2,      1,      0,      0,      0,      0  },    // 0F A9
   {"rsm",       0x803,  0,      1,      0,      0,      0,      0  },    // 0F AA
   {"bts",       3,      0x1100, 0x13,   0x209,  9,      0,      0  },    // 0F AB
   {"shrd",      3,      0x1100, 0x53,   0x209,  0x1009, 0,      0  },    // 0F AC
   {"shrd",      3,      0x1100, 0x13,   0x209,  0x0809, 0,      0  },    // 0F AD
   {0,           0x34,   0,      0,      0,      0,      4,      0  },    // 0F AE. Link to tertiary map for group 15
   {"imul",      1,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 0F AF
//  name         instset prefix  format  dest.   source  link    options
   {"cmpxchg",   3,      0x10,   0x13,   0x101,  1,      0,      0  },    // 0F B0
   {"cmpxchg",   3,      0x1110, 0x13,   0x109,  9,      0,      0  },    // 0F B1
   {"lss",       0,      0x1100, 0x812,  9,      0x10D,  0,      0  },    // 0F B2 (valid in 64-bit mode)
   {"btr",       3,      0x1100, 0x13,   0x209,  9,      0,      0  },    // 0F B3
   {"lfs",       0,      0x1100, 0x812,  9,      0x10D,  0,      0  },    // 0F B4
   {"lgs",       0,      0x1100, 0x812,  9,      0x10D,  0,      0  },    // 0F B5
   {"movzx",     3,      0x1100, 0x12,   9,      0x201,  0,      0  },    // 0F B6
   {"movzx",     3,      0x1100, 0x12,   9,      0x202,  0,      0  },    // 0F B7   
   {0,           0x60,   0,      0,      0,      0,      9,      0  },    // 0F B8. Link to tertiary map for popcnt, jmpe
   {0,           0x2E,   0,      0,      0,      0,      2,      0  },    // 0F B9. Link to tertiary map for group 10: ud1
   {0,           0x2C,   0,      0,      0,      0,      2,      0  },    // 0F BA. Link to tertiary map for group 8: bt
   {"btc",       3,      0x1100, 0x13,   0x209,  9,      0,      0  },    // 0F BB
   {"bsf",       3,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 0F BC
   {"bsr",       3,      0x1100, 0x12,   9,      0x209,  0,      0  },    // 0F BD
   {"movsx",     3,      0x1100, 0x12,   9,      0x201,  0,      0  },    // 0F BE
   {"movsx",     3,      0x1100, 0x12,   9,      0x202,  0,      0  },    // 0F BF
//  name         instset prefix  format  dest.   source  link    options
   {"xadd",      4,      0x10,   0x13,   0x201,  1,      0,      0  },    // 0F C0
   {"xadd",      4,      0x1110, 0x13,   0x209,  9,      0,      0  },    // 0F C1
   {0,           0x38,   0,      0x52,   0,      0,      6,      0  },    // 0F C2. Link to tertiary map for cmpps etc.
   {"movnti",    0x11,   0x1000, 0x13,   0x109,  9,      0,      0  },    // 0F C3
   {0,           0x29,   0,      0,      0,      0,      3,      0  },    // 0F C4. Link to pinsrw
   {"pextrw",    7,      0x1200, 0x52,   9,      0x1032, 0,      0  },    // 0F C5
   {"shuf",      0x11,   0x200,  0x52,   0x6F,   0x126F, 0,      1  },    // 0F C6
   {0,           0x50,   0,      0,      0,      0,      2,      0  },    // 0F C7. Link to tertiary map for group 9
   {"bswap",     3,      0x1000, 3,      9,      0,      0,      0  },    // 0F C8. bswap eax
   {"bswap",     3,      0x1000, 3,      9,      0,      0,      0  },    // 0F C9. bswap ecx
   {"bswap",     3,      0x1000, 3,      9,      0,      0,      0  },    // 0F CA. bswap edx
   {"bswap",     3,      0x1000, 3,      9,      0,      0,      0  },    // 0F CB. bswap ebx
   {"bswap",     3,      0x1000, 3,      9,      0,      0,      0  },    // 0F CC. bswap esp
   {"bswap",     3,      0x1000, 3,      9,      0,      0,      0  },    // 0F CD. bswap ebp
   {"bswap",     3,      0x1000, 3,      9,      0,      0,      0  },    // 0F CE. bswap esi
   {"bswap",     3,      0x1000, 3,      9,      0,      0,      0  },    // 0F CF. bswap edi
//  name         instset prefix  format  dest.   source  link    options
   {0,           0x2D,   0xA00,  0,      0,      0,      9,      0  },    // 0F D0. Link to addsubps/pd
   {"psrlw",     7,      0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F D1
   {"psrld",     7,      0x200,  0x12,   0x33,   0x233,  0,      0  },    // 0F D2
   {"psrlq",     7,      0x200,  0x12,   0x34,   0x234,  0,      0  },    // 0F D3
   {"paddq",     0x12,   0x200,  0x12,   0x34,   0x234,  0,      0  },    // 0F D4
   {"pmullw",    7,      0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F D5
   {0,           0x53,   0,      0,      0,      0,      9,      0  },    // 0F D6. Link to tertiary map for movq2dq etc.
   {"pmovmskb",  7,      0x1200, 0x12,   0x9,    0x030,  0,      0  },    // 0F D7
   {"psubusb",   7,      0x200,  0x12,   0x31,   0x231,  0,      0  },    // 0F D8
   {"psubusw",   7,      0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F D9
   {"pminub",    7,      0x200,  0x12,   0x31,   0x231,  0,      0  },    // 0F DA
   {"pand",      7,      0x200,  0x12,   0x30,   0x230,  0,      0  },    // 0F DB
   {"paddusb",   7,      0x200,  0x12,   0x31,   0x231,  0,      0  },    // 0F DC
   {"paddusw",   7,      0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F DD
   {"pmaxub",    7,      0x200,  0x12,   0x31,   0x231,  0,      0  },    // 0F DE
   {"pandn",     7,      0x200,  0x12,   0x30,   0x230,  0,      0  },    // 0F DF
//  name         instset prefix  format  dest.   source  link    options
   {"pavgb",     7,      0x200,  0x12,   0x31,   0x231,  0,      0  },    // 0F E0
   {"psraw",     7,      0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F E1
   {"psrad",     7,      0x200,  0x12,   0x33,   0x233,  0,      0  },    // 0F E2
   {"pavgw",     7,      0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F E3
   {"pmulhuw",   7,      0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F E4
   {"pmulhw",    7,      0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F E5
   {0,           0x54,   0xE00,  0,      0,      0,      9,      0  },    // 0F E6. Link to tertiary map for cvtpd2dq etc.
   {0,           0x55,   0x200,  0,      0,      0,      9,      0  },    // 0F E7. Link to tertiary map for movntq
   {"psubsb",    7,      0x200,  0x12,   0x31,   0x231,  0,      0  },    // 0F E8
   {"psubsw",    7,      0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F E9
   {"pminsw",    7,      0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F EA
   {"por",       7,      0x200,  0x12,   0x30,   0x230,  0,      0  },    // 0F EB
   {"paddsb",    7,      0x200,  0x12,   0x31,   0x231,  0,      0  },    // 0F EC
   {"paddsw",    7,      0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F ED
   {"pmaxsw",    7,      0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F EE
   {"pxor",      7,      0x200,  0x12,   0x30,   0x230,  0,      0  },    // 0F EF
//  name         instset prefix  format  dest.   source  link    options
   {0,           0x56,   0x800,  0,      0,      0,      9,      0  },    // 0F F0. Link to tertiary map for lddqu
   {"psllw",     7,      0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F F1
   {"pslld",     7,      0x200,  0x12,   0x33,   0x233,  0,      0  },    // 0F F2
   {"psllq",     0x12,   0x200,  0x12,   0x34,   0x234,  0,      0  },    // 0F F3
   {"pmuludq",   7,      0x200,  0x12,   0x34,   0x233,  0,      0  },    // 0F F4
   {"pmaddwd",   7,      0x200,  0x12,   0x33,   0x232,  0,      0  },    // 0F F5
   {"psadbw",    7,      0x200,  0x12,   0x32,   0x231,  0,      0  },    // 0F F6
   {0,           0x57,   0x200,  0,      0,      0,      9,      0  },    // 0F F7. Link to tertiary map for maskmovq
   {"psubb",     7,      0x200,  0x12,   0x31,   0x231,  0,      0  },    // 0F F8
   {"psubw",     7,      0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F F9
   {"psubd",     7,      0x200,  0x12,   0x33,   0x233,  0,      0  },    // 0F FA
   {"psubq",     7,      0x200,  0x12,   0x34,   0x234,  0,      0  },    // 0F FB
   {"paddb",     7,      0x200,  0x12,   0x31,   0x231,  0,      0  },    // 0F FC
   {"paddw",     7,      0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F FD
   {"paddd",     7,      0x200,  0x12,   0x33,   0x233,  0,      0  },    // 0F FE 
   {0,           0,      0,      0,      0,      0,      0,      0  }};   // 0F EF


// Tertiary opcode map for 3-byte opcode. First two bytes = 0F 38
// Indexed by third opcode byte
SOpcodeDef OpcodeMap2[] = {
//  name         instset prefix  format  dest.   source  link    options
   {"pshufb",    0x14,   0x200,  0x12,   0x31,   0x231,  0,      0  },    // 0F 38 00
   {"phaddw",    0x14,   0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F 38 01
   {"phaddd",    0x14,   0x200,  0x12,   0x33,   0x233,  0,      0  },    // 0F 38 02
   {"phaddsw",   0x14,   0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F 38 03
   {"pmaddubsw", 0x14,   0x200,  0x12,   0x32,   0x231,  0,      0  },    // 0F 38 04
   {"phsubw",    0x14,   0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F 38 05
   {"phsubd",    0x14,   0x200,  0x12,   0x33,   0x233,  0,      0  },    // 0F 38 06
   {"phsubsw",   0x14,   0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F 38 07
   {"psignb",    0x14,   0x200,  0x12,   0x31,   0x231,  0,      0  },    // 0F 38 08
   {"psignw",    0x14,   0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F 38 09
   {"psignd",    0x14,   0x200,  0x12,   0x33,   0x233,  0,      0  },    // 0F 38 0A
   {"pmulhrsw",  0x14,   0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F 38 0B
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 0C
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 0D
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 0E
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 0F
   {"pblendvb",  0x15,   0x8200, 0x12,   0x21,   0x221,  0,      0  },    // 0F 38 10
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 11
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 12
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 13
   {"blendvps",  0x15,   0x8200, 0x12,   0x6B,   0x26B,  0,      0  },    // 0F 38 14
   {"blendvpd",  0x15,   0x8200, 0x12,   0x6C,   0x26C,  0,      0  },    // 0F 38 15
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 16
   {"ptest",     0x15,   0x8200, 0x12,   0x20,   0x220,  0,      0  },    // 0F 38 17
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 18
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 19
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 1A
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 1B
   {"pabsb",     0x14,   0x200,  0x12,   0x31,   0x231,  0,      0  },    // 0F 38 1C
   {"pabsw",     0x14,   0x200,  0x12,   0x32,   0x232,  0,      0  },    // 0F 38 1D
   {"pabsd",     0x14,   0x200,  0x12,   0x33,   0x233,  0,      0  },    // 0F 38 1E
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 1F
//  name         instset prefix  format  dest.   source  link    options
   {"pmovsxbv",  0x15,   0x8200, 0x12,   0x22,   0x221,  0,      0  },    // 0F 38 20
   {"pmovsxbd",  0x15,   0x8200, 0x12,   0x23,   0x221,  0,      0  },    // 0F 38 21
   {"pmovsxbq",  0x15,   0x8200, 0x12,   0x24,   0x221,  0,      0  },    // 0F 38 22
   {"pmovsxwd",  0x15,   0x8200, 0x12,   0x23,   0x222,  0,      0  },    // 0F 38 23
   {"pmovsxwq",  0x15,   0x8200, 0x12,   0x24,   0x222,  0,      0  },    // 0F 38 24
   {"pmovsxdq",  0x15,   0x8200, 0x12,   0x24,   0x223,  0,      0  },    // 0F 38 25
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 26
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 27
   {"pmuldq",    0x15,   0x8200, 0x12,   0x24,   0x223,  0,      0  },    // 0F 38 28
   {"pcmpeqq",   0x16,   0x8200, 0x12,   0x24,   0x224,  0,      0  },    // 0F 38 29
   {"movntdqa",  0x15,   0x8200, 0x12,   0x20,   0x120,  0,      0  },    // 0F 38 2A
   {"packusdw",  0x15,   0x8200, 0x12,   0x22,   0x223,  0,      0  },    // 0F 38 2B
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 2C
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 2D
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 2E
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 2F 
//  name         instset prefix  format  dest.   source  link    options
   {"pmovzxbv",  0x15,   0x8200, 0x12,   0x22,   0x221,  0,      0  },    // 0F 38 30
   {"pmovzxbd",  0x15,   0x8200, 0x12,   0x23,   0x221,  0,      0  },    // 0F 38 31
   {"pmovzxbq",  0x15,   0x8200, 0x12,   0x24,   0x221,  0,      0  },    // 0F 38 32
   {"pmovzxwd",  0x15,   0x8200, 0x12,   0x23,   0x222,  0,      0  },    // 0F 38 33
   {"pmovzxwq",  0x15,   0x8200, 0x12,   0x24,   0x222,  0,      0  },    // 0F 38 34
   {"pmovzxdq",  0x15,   0x8200, 0x12,   0x24,   0x223,  0,      0  },    // 0F 38 35
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 36
   {"pcmpgtq",   0x16,   0x8200, 0x12,   0x24,   0x224,  0,      0  },    // 0F 38 37
   {"pminsb",    0x15,   0x8200, 0x12,   0x21,   0x221,  0,      0  },    // 0F 38 38
   {"pminsd",    0x15,   0x8200, 0x12,   0x23,   0x223,  0,      0  },    // 0F 38 39
   {"pminuw",    0x15,   0x8200, 0x12,   0x22,   0x222,  0,      0  },    // 0F 38 3A
   {"pminud",    0x15,   0x8200, 0x12,   0x23,   0x223,  0,      0  },    // 0F 38 3B
   {"pmaxsb",    0x15,   0x8200, 0x12,   0x21,   0x221,  0,      0  },    // 0F 38 3C
   {"pmaxsd",    0x15,   0x8200, 0x12,   0x23,   0x223,  0,      0  },    // 0F 38 3D
   {"pmaxuw",    0x15,   0x8200, 0x12,   0x22,   0x222,  0,      0  },    // 0F 38 3E
   {"pmaxud",    0x15,   0x8200, 0x12,   0x23,   0x223,  0,      0  },    // 0F 38 3F
//  name         instset prefix  format  dest.   source  link    options
   {"pmulld",    0x15,   0x8200, 0x12,   0x23,   0x223,  0,      0  },    // 0F 38 40
   {"phminposuw",0x15,   0x8200, 0x12,   0x22,   0x222,  0,      0  },    // 0F 38 41
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 42
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 43
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 44
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 45
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 46
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 47
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 48
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 49
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 4A
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 4B
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 4C
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 4D
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 4E
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 4F
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 50
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 51
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 52
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 53
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 54
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 55
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 56
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 57
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 58
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 59
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 5A
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 5B
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 5C
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 5D
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 5E
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 5F
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 60
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 61
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 62
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 63
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 64
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 65
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 66
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 67
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 68
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 69
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 6A
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 6B
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 6C
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 6D
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 6E
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 6F
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 70
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 71
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 72
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 73
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 74
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 75
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 76
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 77
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 78
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 79
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 7A
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 7B
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 7C
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 7D
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 7E
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 7F
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 80
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 81
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 82
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 83
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 84
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 85
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 86
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 87
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 88
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 89
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 8A
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 8B
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 8C
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 8D
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 8E
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 8F
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 90
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 91
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 92
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 93
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 94
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 95
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 96
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 97
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 98
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 99
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 9A
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 9B
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 9C
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 9D
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 9E
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 9F
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 A0
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 A1
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 A2
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 A3
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 A4
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 A5
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 A6
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 A7
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 A8
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 A9
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 AA
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 AB
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 AC
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 AD
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 AE
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 AF
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 B0
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 B1
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 B2
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 B3
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 B4
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 B5
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 B6
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 B7
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 B8
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 B9
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 BA
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 BB
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 BC
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 BD
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 BE
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 BF
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 C0
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 C1
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 C2
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 C3
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 C4
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 C5
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 C6
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 C7
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 C8
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 C9
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 CA
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 CB
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 CC
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 CD
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 CE
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 CF
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 D0
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 D1
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 D2
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 D3
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 D4
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 D5
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 D6
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 D7
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 D8
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 D9
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 DA
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 DB
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 DC
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 DD
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 DE
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 DF
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 E0
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 E1
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 E2
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 E3
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 E4
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 E5
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 E6
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 E7
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 E8
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 E9
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 EA
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 EB
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 EC
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 ED
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 EE
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 EF
//  name         instset prefix  format  dest.   source  link    options
   {"crc32",     0x16,   0x9900, 0x12,   0x9,    0x201,  0,      0  },    // 0F 38 F0
   {"crc32",     0x65,   0x9900, 0,      0,      0,      8,      0  },    // 0F 38 F1. Link to crc32 16/32/64 bit
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 F2
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 F3
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 F4
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 F5
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 F6
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 F7
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 F8
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 F9
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 FA
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 FB
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 FC
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 FD
   {0,           0,      0,      0x2012, 0,      0,      0,      0  },    // 0F 38 FE
   {0,           0,      0,      0x2012, 0,      0,      0,      0  }};   // 0F 38 FF

// Tertiary opcode map for 3-byte opcode. First two bytes = 0F 39
// Not used but reserved by Intel for future extensions
SOpcodeDef OpcodeMap3[] = {
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x2012, 0,      0,      0,      0  }};   // 0F 39 00

// Tertiary opcode map for 3-byte opcode. First two bytes = 0F 3A
// Indexed by third opcode byte
SOpcodeDef OpcodeMap4[] = {
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 00
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 01
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 02
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 03
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 04
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 05
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 06
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 07
   {"roundps",   0x15,   0x8200, 0x52,   0x6B,   0x526B, 0,      0  },    // 0F 3A 08
   {"roundpd",   0x15,   0x8200, 0x52,   0x6C,   0x526C, 0,      0  },    // 0F 3A 09
   {"roundss",   0x15,   0x8200, 0x52,   0x4B,   0x524B, 0,      0  },    // 0F 3A 0A
   {"roundsd",   0x15,   0x8200, 0x52,   0x4B,   0x524B, 0,      0  },    // 0F 3A 0B
   {"blendps",   0x15,   0x8200, 0x52,   0x6B,   0x526B, 0,      0  },    // 0F 3A 0C
   {"blendpd",   0x15,   0x8200, 0x52,   0x6C,   0x526C, 0,      0  },    // 0F 3A 0D
   {"pblendw",   0x15,   0x8200, 0x52,   0x22,   0x5222, 0,      0  },    // 0F 3A 0E
   {"palignr",   0x14,   0x200,  0x52,   0x30,   0x5230, 0,      0  },    // 0F 3A 0F
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 10
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 11
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 12
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 13   
   {0,           0x61,   0,      0,      0,      0,      3,      0  },    // 0F 3A 14. Link to pextrb
   {0,           0x62,   0,      0,      0,      0,      3,      0  },    // 0F 3A 15. Link to pextrw
   {0,           0x63,   0,      0,      0,      0,      8,      0  },    // 0F 3A 16. Link to pextrd, pextrq
   {"extractps", 0x15,   0x9200, 0x53,   0x209,  0x506B, 0,      0  },    // 0F 3A 17
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 18
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 19
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 1A
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 1B
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 1C
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 1D
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 1E
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 1F
//  name         instset prefix  format  dest.   source  link    options
   {0,           0x64,   0,      0,      0,      0,      3,      0  },    // 0F 3A 20. Link to pinsrb
   {"insertps",  0x15,   0x8200, 0x52,   0x6B,   0x5003, 0,      0  },    // 0F 3A 21
   {"pinsr",     0x15,   0x9200, 0x52,   0x23,   0x5003, 0,      1  },    // 0F 3A 22
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 23
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 24
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 25
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 26
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 27
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 28
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 29
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 2A
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 2B
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 2C
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 2D
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 2E
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 2F
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 30
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 31
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 32
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 33
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 34
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 35
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 36
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 37
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 38
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 39
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 3A
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 3B
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 3C
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 3D
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 3E
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 3F
//  name         instset prefix  format  dest.   source  link    options
   {"dpps",      0x15,   0x8200, 0x52,   0x6B,   0x526B, 0,      0  },    // 0F 3A 40
   {"dppd",      0x15,   0x8200, 0x52,   0x6C,   0x526C, 0,      0  },    // 0F 3A 41
   {"mpsadbw",   0x15,   0x8200, 0x52,   0x22,   0x5221, 0,      0  },    // 0F 3A 42
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 43
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 44
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 45
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 46
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 47
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 48
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 49
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 4A
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 4B
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 4C
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 4D
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 4E
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 4F
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 50
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 51
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 52
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 53
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 54
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 55
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 56
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 57
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 58
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 59
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 5A
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 5B
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 5C
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 5D
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 5E
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 5F
//  name         instset prefix  format  dest.   source  link    options
   {"pcmpestrm", 0x16,   0x8200, 0x52,   0x21,   0x5228, 0,      0  },    // 0F 3A 60
   {"pcmpestri", 0x16,   0x8200, 0x52,   0x21,   0x5228, 0,      0  },    // 0F 3A 61
   {"pcmpistrm", 0x16,   0x8200, 0x52,   0x21,   0x5228, 0,      0  },    // 0F 3A 62
   {"pcmpistri", 0x16,   0x8200, 0x52,   0x21,   0x5228, 0,      0  },    // 0F 3A 63
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 64
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 65
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 66
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 67
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 68
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 69
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 6A
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 6B
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 6C
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 6D
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 6E
   {0,           0,      0,      0x2052, 0,      0,      0,      0  },    // 0F 3A 6F
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x2052, 0,      0,      0,      0  }};   // 0F 3A 70


// Tertiary opcode map for 3-byte opcode. First two bytes = 0F 3B
// Not used but reserved by Intel for future extensions
SOpcodeDef OpcodeMap5[1] = {
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x2000, 0,      0,      0,      0  }};   // 0F 3B 00


// Tertiary opcode map for AMD 3DNow instructions. First two bytes = 0F 0F
// Indexed by immediate byte following operands
SOpcodeDef OpcodeMap6[] = {
//  name         instset prefix  format  dest.   source  link    options
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 00
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 01
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 02
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 03
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 04
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 05
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 06
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 07
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 08
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 09
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 0A
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 0B
   {"PFI2FW",    0x1001, 0,      0x0052, 0x5B,   0x212,  0,      0  },    // 0F 0F op 0C
   {"PI2FD",     0x1000, 0,      0x0052, 0x5B,   0x213,  0,      0  },    // 0F 0F op 0D
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 0E
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 0F
//  name         instset prefix  format  dest.   source  link    options
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 10
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 11
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 12
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 13
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 14
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 15
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 16
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 17
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 18
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 19
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 1A
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 1B
   {"PF2IW",     0x1001, 0,      0x0052, 0x12,   0x25B,  0,      0  },    // 0F 0F op 1C
   {"PF2ID",     0x1000, 0,      0x0052, 0x13,   0x25B,  0,      0  },    // 0F 0F op 1D
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 1E
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 1F
//  name         instset prefix  format  dest.   source  link    options
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 20
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 21
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 22
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 23
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 24
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 25
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 26
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 27
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 28
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 29
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 2A
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 2B
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 2C
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 2D
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 2E
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 2F
//  name         instset prefix  format  dest.   source  link    options
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 30
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 31
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 32
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 33
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 34
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 35
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 36
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 37
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 38
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 39
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 3A
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 3B
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 3C
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 3D
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 3E
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 3F
//  name         instset prefix  format  dest.   source  link    options
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 40
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 41
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 42
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 43
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 44
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 45
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 46
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 47
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 48
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 49
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 4A
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 4B
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 4C
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 4D
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 4E
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 4F
//  name         instset prefix  format  dest.   source  link    options
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 50
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 51
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 52
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 53
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 54
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 55
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 56
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 57
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 58
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 59
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 5A
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 5B
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 5C
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 5D
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 5E
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 5F
//  name         instset prefix  format  dest.   source  link    options
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 60
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 61
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 62
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 63
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 64
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 65
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 66
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 67
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 68
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 69
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 6A
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 6B
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 6C
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 6D
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 6E
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 6F
//  name         instset prefix  format  dest.   source  link    options
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 70
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 71
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 72
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 73
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 74
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 75
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 76
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 77
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 78
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 79
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 7A
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 7B
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 7C
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 7D
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 7E
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 7F
//  name         instset prefix  format  dest.   source  link    options
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 80
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 81
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 82
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 83
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 84
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 85
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 86
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 87
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 88
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 89
   {"PFNACC",    0x1001, 0,      0x0052, 0x5B,   0x25B,  0,      0  },    // 0F 0F op 8A
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 8B
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 8C
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 8D
   {"PFPNACC",   0x1001, 0,      0x0052, 0x5B,   0x25B,  0,      0  },    // 0F 0F op 8E
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 8F
//  name         instset prefix  format  dest.   source  link    options
   {"PFCMPGE",   0x1000, 0,      0x0052, 0x5B,   0x25B,  0,      0  },    // 0F 0F op 90
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 91
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 92
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 93
   {"PFMIN",     0x1000, 0,      0x0052, 0x5B,   0x25B,  0,      0  },    // 0F 0F op 94
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 95
   {"PFRCP",     0x1000, 0,      0x0052, 0x5B,   0x25B,  0,      0  },    // 0F 0F op 96
   {"PFRSQRT",   0x1000, 0,      0x0052, 0x5B,   0x25B,  0,      0  },    // 0F 0F op 97
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 98
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 99
   {"PFSUB",     0x1000, 0,      0x0052, 0x5B,   0x25B,  0,      0  },    // 0F 0F op 9A
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 9B
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 9C
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 9D
   {"PFADD",     0x1000, 0,      0x0052, 0x5B,   0x25B,  0,      0  },    // 0F 0F op 9E
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op 9F
//  name         instset prefix  format  dest.   source  link    options
   {"PFCMPGT",   0x1000, 0,      0x0052, 0x5B,   0x25B,  0,      0  },    // 0F 0F op A0
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op A1
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op A2
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op A3
   {"PFMAX",     0x1000, 0,      0x0052, 0x5B,   0x25B,  0,      0  },    // 0F 0F op A4
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op A5
   {"PFRCPIT1",  0x1000, 0,      0x0052, 0x5B,   0x25B,  0,      0  },    // 0F 0F op A6
   {"PFRSQIT1",  0x1000, 0,      0x0052, 0x5B,   0x25B,  0,      0  },    // 0F 0F op A7
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op A8
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op A9
   {"PFSUBR",    0x1000, 0,      0x0052, 0x5B,   0x25B,  0,      0  },    // 0F 0F op AA
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op AB
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op AC
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op AD
   {"PFACC",     0x1000, 0,      0x0052, 0x5B,   0x25B,  0,      0  },    // 0F 0F op AE
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op AF
//  name         instset prefix  format  dest.   source  link    options
   {"PFCMPEQ",   0x1000, 0,      0x0052, 0x5B,   0x25B,  0,      0  },    // 0F 0F op B0
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op B1
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op B2
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op B3
   {"PFMUL",     0x1000, 0,      0x0052, 0x5B,   0x25B,  0,      0  },    // 0F 0F op B4
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op B5
   {"PFRCPIT2",  0x1000, 0,      0x0052, 0x5B,   0x25B,  0,      0  },    // 0F 0F op B6
   {"PMULHRW",   0x1000, 0,      0x0052, 0x12,   0x212,  0,      0  },    // 0F 0F op B7
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op B8
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op B9
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op BA
   {"PSWAPD",    0x1001, 0,      0x0052, 0x10,   0x210,  0,      0  },    // 0F 0F op BB
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op BC
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op BD
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  },    // 0F 0F op BE
   {"PAVGUSB",   0x1000, 0,      0x0052, 0x11,   0x211,  0,      0  },    // 0F 0F op BF
//  name         instset prefix  format  dest.   source  link    options
   {0,           0x1000, 0,      0x2052, 0x10,   0x210,  0,      0  }};   // 0F 0F op C0


// Secondary opcode map for x87 f.p. instructions. Opcode D8
// Indexed by reg bits and mod == 3
SOpcodeDef OpcodeMap8[16] = {
//  name         instset prefix  format  dest.   source  link    options
   {"fadd",      0x100,  0,      0x11,   0,      0x143,  0,      0  },    // fadd m32
   {"fmul",      0x100,  0,      0x11,   0,      0x143,  0,      0  },    // fmul m32
   {"fcom",      0x100,  0,      0x11,   0,      0x143,  0,      4  },    // fcom m32
   {"fcomp",     0x100,  0,      0x11,   0,      0x143,  0,      4  },    // fcomp m32
   {"fsub",      0x100,  0,      0x11,   0,      0x143,  0,      0  },    // fsub m32
   {"fsubr",     0x100,  0,      0x11,   0,      0x143,  0,      0  },    // fsubr m32
   {"fdiv",      0x100,  0,      0x11,   0,      0x143,  0,      0  },    // fdiv m32
   {"fdivr",     0x100,  0,      0x11,   0,      0x143,  0,      0  },    // fdivr m32
   {"fadd",      0x100,  0,      0x11,   0xaf,   0x040,  0,      0  },    // fadd st,st(i)
   {"fmul",      0x100,  0,      0x11,   0xaf,   0x040,  0,      0  },    // fmul st,st(i)
   {"fcom",      0x100,  0,      0x11,   0,      0x040,  0,      4  },    // fcom st,st(i)
   {"fcomp",     0x100,  0,      0x11,   0,      0x040,  0,      4  },    // fcomp st,st(i)
   {"fsub",      0x100,  0,      0x11,   0xaf,   0x040,  0,      0  },    // fsub st,st(i)
   {"fsubr",     0x100,  0,      0x11,   0xaf,   0x040,  0,      0  },    // fsubr st,st(i)
   {"fdiv",      0x100,  0,      0x11,   0xaf,   0x040,  0,      0  },    // fdiv st,st(i)
   {"fdivr",     0x100,  0,      0x11,   0xaf,   0x040,  0,      0  }};   // fdivr st,st(i)

// Secondary opcode map for x87 f.p. instructions. Opcode D9
// Indexed by reg bits and mod == 3
SOpcodeDef OpcodeMap9[16] = {
//  name         instset prefix  format  dest.   source  link    options
   {"fld",       0x100,  0,      0x11,   0,      0x143,  0,      0  },    // fld m32
   {0,           0,      0,      0x4011, 0,      0x143,  0,      0  },    // illegal
   {"fst",       0x100,  0,      0x11,   0x143,  0,      0,      0  },    // fst m32
   {"fstp",      0x100,  0,      0x11,   0x143,  0,      0,      0  },    // fstp m32
   {"fldenv",    0x100,  0,      0x11,   0,      0x106,  0,      0  },    // fldenv m
   {"fldcw",     0x100,  0,      0x11,   0,      0x102,  0,      0  },    // fldcw m16
   {"fnstenv",   0x100,  0,      0x11,   0x106,  0,      0,      0  },    // fnstenv m
   {"fnstcw",    0x100,  0,      0x11,   0x102,  0,      0,      0  },    // fnstcw m16
   {"fld",       0x100,  0,      0x11,   0,      0x040,  0,      0  },    // fld st(i)
   {"fxch",      0x100,  0,      0x11,   0,      0x040,  0,      0  },    // fxch st(i)
   {"fnop",      0x10,   0,      0x10,   0,      0,      5,      0  },    // Link to tertiary map 0x10 fnop
   {0,           0,      0,      0x4011, 0,      0x040,  0,      0  },    // Illegal
   {0,           0x11,   0,      0,      0,      0,      5,      0  },    // Link to tertiary map 0x11 fchs etc.
   {0,           0x12,   0,      0,      0,      0,      5,      0  },    // Link to tertiary map 0x12 fld1 etc.
   {0,           0x13,   0,      0,      0,      0,      5,      0  },    // Link to tertiary map 0x13 f2xm1 etc.
   {0,           0x14,   0,      0,      0,      0,      5,      0  }};   // Link to tertiary map 0x14 fprem etc.

// Secondary opcode map for x87 f.p. instructions. Opcode DA
// Indexed by reg bits and mod == 3
SOpcodeDef OpcodeMapA[16] = {
//  name         instset prefix  format  dest.   source  link    options
   {"fiadd",     0x100,  0,      0x11,   0,      0x103,  0,      0  },    // fiadd m32
   {"fimul",     0x100,  0,      0x11,   0,      0x103,  0,      0  },    // fimul m32
   {"ficom",     0x100,  0,      0x11,   0,      0x103,  0,      4  },    // ficom m32
   {"ficomp",    0x100,  0,      0x11,   0,      0x103,  0,      4  },    // ficomp m32
   {"fisub",     0x100,  0,      0x11,   0,      0x103,  0,      0  },    // fisub m32
   {"fisubr",    0x100,  0,      0x11,   0,      0x103,  0,      0  },    // fisubr m32
   {"fidiv",     0x100,  0,      0x11,   0,      0x103,  0,      0  },    // fidiv m32
   {"fidivr",    0x100,  0,      0x11,   0,      0x103,  0,      0  },    // fidivr m32
   {"fcmovb",    0x100,  0,      0x11,   0xaf,   0x040,  0,      0  },    // fcmovb st,st(i)
   {"fcmove",    0x100,  0,      0x11,   0xaf,   0x040,  0,      0  },    // fcmovb st,st(i)
   {"fcmovbe",   0x100,  0,      0x11,   0xaf,   0x040,  0,      0  },    // fcmovbe st,st(i)
   {"fcmovu",    0x100,  0,      0x11,   0xaf,   0x040,  0,      0  },    // fcmovbe st,st(i)
   {0,           0,      0,      0x4011, 0,      0x040,  0,      0  },    // Illegal
   {0,           0x15,   0,      0,      0,      0,      5,      0  },    // Link to tertiary map 0x15 fucompp
   {0,           0,      0,      0x4011, 0,      0x040,  0,      0  },    // Illegal
   {0,           0,      0,      0x4011, 0,      0x040,  0,      0  }};   // Illegal

// Secondary opcode map for x87 f.p. instructions. Opcode DB
// Indexed by reg bits and mod == 3
SOpcodeDef OpcodeMapB[16] = {
//  name         instset prefix  format  dest.   source  link    options
   {"fild",      0x100,  0,      0x11,   0,      0x103,  0,      0  },    // fild m32
   {"fisttp",    0x13,   0,      0x11,   0,      0x103,  0,      0  },    // fisttp m32
   {"fist",      0x100,  0,      0x11,   0x103,  0,      0,      0  },    // fist m32
   {"fistp",     0x100,  0,      0x11,   0x103,  0,      0,      0  },    // fistp m32
   {0,           0,      0,      0x4011, 0,      0x103,  0,      0  },    // illegal
   {"fld",       0x100,  0,      0x11,   0,      0x145,  0,      0  },    // fld m80
   {0,           0,      0,      0x4011, 0,      0x103,  0,      0  },    // illegal
   {"fstp",      0x100,  0,      0x11,   0x145,  0,      0,      0  },    // fst m80
   {"fcmovnb",   0x100,  0,      0x11,   0xaf,   0x040,  0,      0  },    // fcmovnb st,st(i)
   {"fcmovne",   0x100,  0,      0x11,   0xaf,   0x040,  0,      0  },    // fcmovne st,st(i)
   {"fcmovnbe",  0x100,  0,      0x11,   0xaf,   0x040,  0,      0  },    // fcmovnbe st,st(i)
   {"fcmovnu",   0x100,  0,      0x11,   0xaf,   0x040,  0,      0  },    // fcmovnu st,st(i)
   {0,           0x16,   0,      0,      0,      0,      5,      0  },    // Link to tertiary map 0x16 fclex etc.
   {"fucomi",    0x101,  0,      0x11,   0xaf,   0x040,  0,      4  },    // fucomi st,st(i)
   {"fcomi",     0x100,  0,      0x11,   0xaf,   0x040,  0,      4  },    // fcomi st,st(i)
   {0,           0,      0,      0x4011, 0,      0x040,  0,      0  }};   // illegal

// Secondary opcode map for x87 f.p. instructions. Opcode DC
// Indexed by reg bits and mod == 3
SOpcodeDef OpcodeMapC[16] = {
//  name         instset prefix  format  dest.   source  link    options
   {"fadd",      0x100,  0,      0x11,   0,      0x144,  0,      0  },    // fadd m64
   {"fmul",      0x100,  0,      0x11,   0,      0x144,  0,      0  },    // fmul m64
   {"fcom",      0x100,  0,      0x11,   0,      0x144,  0,      4  },    // fcom m64
   {"fcomp",     0x100,  0,      0x11,   0,      0x144,  0,      4  },    // fcomp m64
   {"fsub",      0x100,  0,      0x11,   0,      0x144,  0,      0  },    // fsub m64
   {"fsubr",     0x100,  0,      0x11,   0,      0x144,  0,      0  },    // fsubr m64
   {"fdiv",      0x100,  0,      0x11,   0,      0x144,  0,      0  },    // fdiv m64
   {"fdivr",     0x100,  0,      0x11,   0,      0x144,  0,      0  },    // fdivr m64
   {"fadd",      0x100,  0,      0x11,   0x040,  0xaf,   0,      0  },    // fadd st(i),st
   {"fmul",      0x100,  0,      0x11,   0x040,  0xaf,   0,      0  },    // fmul st(i),st
   {0,           0,      0,      0x4011, 0x040,  0xaf,   0,      0  },    // illegal
   {0,           0,      0,      0x4011, 0x040,  0xaf,   0,      0  },    // illegal
   {"fsubr",     0x100,  0,      0x11,   0x040,  0xaf,   0,      0  },    // fsubr st(i),st
   {"fsub",      0x100,  0,      0x11,   0x040,  0xaf,   0,      0  },    // fsub st(i),st
   {"fdivr",     0x100,  0,      0x11,   0x040,  0xaf,   0,      0  },    // fdivr st(i),st
   {"fdiv",      0x100,  0,      0x11,   0x040,  0xaf,   0,      0  }};   // fdiv st(i),st

// Secondary opcode map for x87 f.p. instructions. Opcode DD
// Indexed by reg bits and mod == 3
SOpcodeDef OpcodeMapD[16] = {
//  name         instset prefix  format  dest.   source  link    options
   {"fld",       0x100,  0,      0x11,   0,      0x144,  0,      0  },    // fld m64
   {"fisttp",    0x13,   0,      0x11,   0x104,  0,      0,      0  },    // fisttp m64
   {"fst",       0x100,  0,      0x11,   0x144,  0,      0,      0  },    // fst m64
   {"fstp",      0x100,  0,      0x11,   0x144,  0,      0,      0  },    // fstp m64
   {"frstor",    0x100,  0,      0x11,   0x106,  0,      0,      0  },    // frstor 108 bytes
   {0,           0,      0,      0x4011, 0,      0x106,  0,      0  },    // illegal
   {"fnsave",    0x100,  0,      0x11,   0,      0x106,  0,      0  },    // fnsave 108 bytes
   {"fnstsw",    0x100,  0,      0x11,   0x102,  0,      0,      0  },    // fstsw m16
   {"ffree",     0x100,  0,      0x11,   0x40,   0,      0,      0  },    // ffree st(i)
   {0,           0,      0,      0x4011, 0,      0x106,  0,      0  },    // illegal
   {"fst",       0x100,  0,      0x11,   0x40,   0,      0,      0  },    // fst st(i)
   {"fstp",      0x100,  0,      0x11,   0x40,   0,      0,      0  },    // fstp st(i)
   {"fucom",     0x101,  0,      0x11,   0,      0x40,   0,      0  },    // fucom st(i)
   {"fucomp",    0x101,  0,      0x11,   0,      0x40,   0,      0  },    // fucomp st(i)
   {0,           0,      0,      0x4011, 0,      0x106,  0,      0  },    // illegal
   {0,           0,      0,      0x4011, 0,      0x106,  0,      0  }};   // illegal

// Secondary opcode map for x87 f.p. instructions. Opcode DE
// Indexed by reg bits and mod == 3
SOpcodeDef OpcodeMapE[16] = {
//  name         instset prefix  format  dest.   source  link    options
   {"fiadd",     0x100,  0,      0x11,   0,      0x102,  0,      0  },    // fiadd m16
   {"fimul",     0x100,  0,      0x11,   0,      0x102,  0,      0  },    // fimul m16
   {"ficom",     0x100,  0,      0x11,   0,      0x102,  0,      4  },    // ficom m16
   {"ficomp",    0x100,  0,      0x11,   0,      0x102,  0,      4  },    // ficomp m16
   {"fisub",     0x100,  0,      0x11,   0,      0x102,  0,      0  },    // fisub m16
   {"fisubr",    0x100,  0,      0x11,   0,      0x102,  0,      0  },    // fisubr m16
   {"fidiv",     0x100,  0,      0x11,   0,      0x102,  0,      0  },    // fidiv m16
   {"fidivr",    0x100,  0,      0x11,   0,      0x102,  0,      0  },    // fidivr m16
   {"faddp",     0x100,  0,      0x11,   0x040,  0xaf,   0,      0  },    // faddp st(i),st
   {"fmulp",     0x100,  0,      0x11,   0x040,  0xaf,   0,      0  },    // fmulp st(i),st
   {0,           0,      0,      0x4011, 0x040,  0xaf,   0,      0  },    // illegal
   {0,           0x17,   0,      0,      0,      0,      5,      0  },    // Link to tertiary map 0x17 fcompp
   {"fsubrp",    0x100,  0,      0x11,   0x040,  0xaf,   0,      0  },    // fsubrp st(i),st (Yes, the order is illogical here)
   {"fsubp",     0x100,  0,      0x11,   0x040,  0xaf,   0,      0  },    // fsubp st(i),st
   {"fdivrp",    0x100,  0,      0x11,   0x040,  0xaf,   0,      0  },    // fdivrp st(i),st
   {"fdivp",     0x100,  0,      0x11,   0x040,  0xaf,   0,      0  }};   // fdivp st(i),st

// Secondary opcode map for x87 f.p. instructions. Opcode DF
// Indexed by reg bits and mod == 3
SOpcodeDef OpcodeMapF[16] = {
//  name         instset prefix  format  dest.   source  link    options
   {"fild",      0x100,  0,      0x11,   0,      0x102,  0,      0  },    // fild m16
   {"fisttp",    0x13,   0,      0x11,   0,      0x102,  0,      0  },    // fisttp m16
   {"fist",      0x100,  0,      0x11,   0x102,  0,      0,      0  },    // fist m16
   {"fistp",     0x100,  0,      0x11,   0x102,  0,      0,      0  },    // fistp m16
   {"fbld",      0x100,  0,      0x11,   0,      0x105,  0,      0  },    // fbld m80
   {"fild",      0x100,  0,      0x11,   0,      0x104,  0,      0  },    // fild m64
   {"fbstp",     0x100,  0,      0x11,   0x105,  0,      0,      0  },    // fbstp m80
   {"fistp",     0x100,  0,      0x11,   0x104,  0,      0,      0  },    // fistp m64
   {0,           0,      0,      0x4011, 0,      0x40,   0,      0  },    // illegal
   {0,           0,      0,      0x4011, 0,      0x40,   0,      0  },    // illegal
   {0,           0,      0,      0x4011, 0,      0x40,   0,      0  },    // illegal
   {0,           0,      0,      0x4011, 0,      0x40,   0,      0  },    // illegal
   {0,           0x18,   0,      0,      0,      0,      5,      0  },    // Link to tertiary map 0x18 fnstsw ax
   {"fucomip",   0x101,  0,      0x11,   0xAF,   0x40,   0,      4  },    // fucomp st,st(i)
   {"fcomip",    0x100,  0,      0x11,   0xAF,   0x40,   0,      4  },    // fcomp st,st(i)
   {0,           0,      0,      0x4011, 0,      0x40,   0,      0  }};   // illegal

// Tertiary opcode map for f.p. D9 / reg = 010
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap10[2] = {
//  name         instset prefix  format  dest.   source  link    options
   {"fnop",      0x100,  0,      0x10,   0,      0,      0,     0x40},    // fnop
   {0,           0,      0,      0,      0,      0,      0,      0  }};   // the rest is illegal

// Tertiary opcode map for f.p. D9 / reg = 100
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap11[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {"fchs",      0x100,  0,      0x10,   0,      0,      0,      0  },    // fchs
   {"fabs",      0x100,  0,      0x10,   0,      0,      0,      0  },    // fabs
   {0,           0,      0,      0x4010, 0,      0,      0,      0  },    // illegal
   {0,           0,      0,      0x4010, 0,      0,      0,      0  },    // illegal
   {"ftst",      0x100,  0,      0x10,   0,      0,      0,      0  },    // ftst
   {"fxam",      0x100,  0,      0x10,   0,      0,      0,      0  },    // fxam
   {0,           0,      0,      0x4010, 0,      0,      0,      0  },    // illegal
   {0,           0,      0,      0x4010, 0,      0,      0,      0  }};   // illegal

// Tertiary opcode map for f.p. D9 / reg = 101
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap12[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {"fld1",      0x100,  0,      0x10,   0,      0,      0,      0  },    // fld1
   {"fldl2t",    0x100,  0,      0x10,   0,      0,      0,      0  },    // 
   {"fldl2e",    0x100,  0,      0x10,   0,      0,      0,      0  },    // 
   {"fldpi",     0x100,  0,      0x10,   0,      0,      0,      0  },    // 
   {"fldlg2",    0x100,  0,      0x10,   0,      0,      0,      0  },    // 
   {"fldln2",    0x100,  0,      0x10,   0,      0,      0,      0  },    // 
   {"fldz",      0x100,  0,      0x10,   0,      0,      0,      0  },    // 
   {0,           0,      0,      0x4010, 0,      0,      0,      0  }};   // illegal

// Tertiary opcode map for f.p. D9 / reg = 110
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap13[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {"f2xm1",     0x100,  0,      0x10,   0,      0,      0,      0  },    // f2xm1
   {"fyl2x",     0x100,  0,      0x10,   0,      0,      0,      0  },    // 
   {"fptan",     0x100,  0,      0x10,   0,      0,      0,      0  },    // 
   {"fpatan",    0x100,  0,      0x10,   0,      0,      0,      0  },    // 
   {"fxtract",   0x100,  0,      0x10,   0,      0,      0,      0  },    // 
   {"fprem1",    0x100,  0,      0x10,   0,      0,      0,      0  },    // 
   {"fdecstp",   0x100,  0,      0x10,   0,      0,      0,      0  },    // 
   {"fincstp",   0x100,  0,      0x10,   0,      0,      0,      0  }};   // 

// Tertiary opcode map for f.p. D9 / reg = 111
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap14[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {"fprem",     0x100,  0,      0x10,   0,      0,      0,      0  },    // 
   {"fyl2xp1",   0x100,  0,      0x10,   0,      0,      0,      0  },    // 
   {"fsqrt",     0x100,  0,      0x10,   0,      0,      0,      0  },    // 
   {"fsincos",   0x101,  0,      0x10,   0,      0,      0,      0  },    // 
   {"frndint",   0x100,  0,      0x10,   0,      0,      0,      0  },    // 
   {"fscale",    0x100,  0,      0x10,   0,      0,      0,      0  },    // 
   {"fsin",      0x101,  0,      0x10,   0,      0,      0,      0  },    // 
   {"fcos",      0x101,  0,      0x10,   0,      0,      0,      0  }};   // 

// Tertiary opcode map for f.p. DA / reg = 101
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap15[3] = {
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x4010, 0,      0,      0,      0  },    // illegal
   {"fucompp",   0x101,  0,      0x10,   0,      0,      0,      0  },    // 
   {0,           0,      0,      0x4010, 0,      0,      0,      0  }};   // illegal

// Tertiary opcode map for f.p. DB / reg = 100
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap16[5] = {
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x4010, 0,      0,      0,      0  },    // illegal
   {0,           0,      0,      0x4010, 0,      0,      0,      0  },    // illegal
   {"fnclex",    0x100,  0,      0x10,   0,      0,      0,      0  },    // 
   {"fninit",    0x100,  0,      0x10,   0,      0,      0,      8  },    // 
   {0,           0,      0,      0x4010, 0,      0,      0,      0  }};   // illegal

// Tertiary opcode map for f.p. DE / reg = 011
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap17[3] = {
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x4010, 0,      0,      0,      0  },    // illegal
   {"fcompp",    0x100,  0,      0x10,   0,      0,      0,      4  },    // 
   {0,           0,      0,      0x4010, 0,      0,      0,      0  }};   // illegal

// Tertiary opcode map for f.p. DF / reg = 100
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap18[2] = {
//  name         instset prefix  format  dest.   source  link    options
   {"fnstsw",    0x100,  0,      0x11,   0xa2,   0,      0,      0  },    // 
   {0,           0,      0,      0x4010, 0,      0,      0,      0  }};   // illegal

// Secondary opcode map for IRET. Opcode byte = 0xCF
// Indexed by operand size
SOpcodeDef OpcodeMap19[3] = {
//  name         instset prefix  format  dest.   source  link    options
   {"iret",      0,      0x0102, 2,      0,      0,      0,      0x10},    // CF
   {"iretd",     0,      0x0102, 2,      0,      0,      0,      0x10},    // CF
   {"iretq",     0,      0x1102, 2,      0,      0,      0,      0x10}};   // CF

// Secondary opcode map for immediate group 1. Opcode byte = 0x80
// Indexed by reg bits = 0 - 7
SOpcodeDef OpcodeMap1A[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {"add",       0,      0x10,   0x51,   0x201,   0x1000, 0,      0  },    // 80 /0
   {"or",        0,      0x10,   0x51,   0x201,   0x5000, 0,      0  },    // 80 /1
   {"adc",       0,      0x10,   0x51,   0x201,   0x1000, 0,      0  },    // 80 /2
   {"sbb",       0,      0x10,   0x51,   0x201,   0x1000, 0,      0  },    // 80 /3
   {"and",       0,      0x10,   0x51,   0x201,   0x5000, 0,      0  },    // 80 /4
   {"sub",       0,      0x10,   0x51,   0x201,   0x1000, 0,      0  },    // 80 /5
   {"xor",       0,      0x10,   0x51,   0x201,   0x5000, 0,      0  },    // 80 /6
   {"cmp",       0,      0,      0x51,   0x201,   0x1000, 0,      4  }};   // 80 /7

// Secondary opcode map for immediate group 1. Opcode byte = 0x81
// Indexed by reg bits = 0 - 7
SOpcodeDef OpcodeMap1B[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {"add",       0,      0x1110, 0x91,   0x209,   0x3000, 0,     0x80},    // 81 /0
   {"or",        0,      0x1110, 0x91,   0x209,   0x7000, 0,     0x80},    // 81 /1
   {"adc",       0,      0x1110, 0x91,   0x209,   0x3000, 0,     0x80},    // 81 /2
   {"sbb",       0,      0x1110, 0x91,   0x209,   0x3000, 0,     0x80},    // 81 /3
   {"and",       0,      0x1110, 0x91,   0x209,   0x7000, 0,     0x80},    // 81 /4
   {"sub",       0,      0x1110, 0x91,   0x209,   0x3000, 0,     0x80},    // 81 /5
   {"xor",       0,      0x1110, 0x91,   0x209,   0x7000, 0,     0x80},    // 81 /6
   {"cmp",       0,      0x1100, 0x91,   0x209,   0x3000, 0,     0x84}};   // 81 /7

// Secondary opcode map for immediate group 1. Opcode byte = 0x82.
// Undocumented opcode. Signed byte instructions do the same as unsigned byte instructions at 0x80
SOpcodeDef OpcodeMap1C[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {"add",       0x8000, 0x10,   0x4051, 0x201,   0x2000, 0,      0  },    // 82 /0
   {"or",        0x8000, 0x10,   0x4051, 0x201,   0x6000, 0,      0  },    // 82 /1
   {"adc",       0x8000, 0x10,   0x4051, 0x201,   0x2000, 0,      0  },    // 82 /2
   {"sbb",       0x8000, 0x10,   0x4051, 0x201,   0x2000, 0,      0  },    // 82 /3
   {"and",       0x8000, 0x10,   0x4051, 0x201,   0x6000, 0,      0  },    // 82 /4
   {"sub",       0x8000, 0x10,   0x4051, 0x201,   0x2000, 0,      0  },    // 82 /5
   {"xor",       0x8000, 0x10,   0x4051, 0x201,   0x6000, 0,      0  },    // 82 /6
   {"cmp",       0,      0,      0x4051, 0x201,   0x2000, 0,      4  }};   // 82 /7

// Secondary opcode map for immediate group 1. Opcode byte = 0x83
// Indexed by reg bits = 0 - 7
SOpcodeDef OpcodeMap1D[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {"add",       0,      0x1110, 0x51,   0x209,   0x2000, 0,      0  },    // 83 /0
   {"or",        0,      0x1110, 0x51,   0x209,   0x6000, 0,      0  },    // 83 /1
   {"adc",       0,      0x1110, 0x51,   0x209,   0x2000, 0,      0  },    // 83 /2
   {"sbb",       0,      0x1110, 0x51,   0x209,   0x2000, 0,      0  },    // 83 /3
   {"and",       0,      0x1110, 0x51,   0x209,   0x6000, 0,      0  },    // 83 /4
   {"sub",       0,      0x1110, 0x51,   0x209,   0x2000, 0,      0  },    // 83 /5
   {"xor",       0,      0x1110, 0x51,   0x209,   0x6000, 0,      0  },    // 83 /6
   {"cmp",       0,      0x1100, 0x51,   0x209,   0x2000, 0,      4  }};   // 83 /7

// Secondary opcode map for shift group 2. Opcode byte = 0xC0
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap1E[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {"rol",       0,      0,      0x51,   0x201,   0x1000, 0,      0  },    // C0 /0 rol byte ptr r/m,ib
   {"ror",       0,      0,      0x51,   0x201,   0x1000, 0,      0  },    // C0 /1
   {"rcl",       0,      0,      0x51,   0x201,   0x1000, 0,      0  },    // C0 /2
   {"rcr",       0,      0,      0x51,   0x201,   0x1000, 0,      0  },    // C0 /3
   {"shl",       0,      0,      0x51,   0x201,   0x1000, 0,      0  },    // C0 /4
   {"shr",       0,      0,      0x51,   0x201,   0x1000, 0,      0  },    // C0 /5
   {"sal",       0,      0,      0x51,   0x201,   0x1000, 0,      0  },    // C0 /6
   {"sar",       0,      0,      0x51,   0x201,   0x1000, 0,      0  }};   // C0 /7

// Secondary opcode map for shift group 2. Opcode byte = 0xC1
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap1F[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {"rol",       0,      0x1100, 0x51,   0x209,   0x1000, 0,      0  },    // C1 /0 rol word ptr r/m,ib
   {"ror",       0,      0x1100, 0x51,   0x209,   0x1000, 0,      0  },    // C1 /1
   {"rcl",       0,      0x1100, 0x51,   0x209,   0x1000, 0,      0  },    // C1 /2
   {"rcr",       0,      0x1100, 0x51,   0x209,   0x1000, 0,      0  },    // C1 /3
   {"shl",       0,      0x1100, 0x51,   0x209,   0x1000, 0,      0  },    // C1 /4
   {"shr",       0,      0x1100, 0x51,   0x209,   0x1000, 0,      0  },    // C1 /5
   {"sal",       0,      0x1100, 0x51,   0x209,   0x1000, 0,      0  },    // C1 /6
   {"sar",       0,      0x1100, 0x51,   0x209,   0x1000, 0,      0  }};   // C1 /7

// Secondary opcode map for shift group 2. Opcode byte = 0xD0
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap20[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {"rol",       0,      0,      0x11,   0x201,   0xb1,    0,      0  },    // C2 /0 rol byte ptr r/m,1
   {"ror",       0,      0,      0x11,   0x201,   0xb1,    0,      0  },    // C2 /1
   {"rcl",       0,      0,      0x11,   0x201,   0xb1,    0,      0  },    // C2 /2
   {"rcr",       0,      0,      0x11,   0x201,   0xb1,    0,      0  },    // C2 /3
   {"shl",       0,      0,      0x11,   0x201,   0xb1,    0,      0  },    // C2 /4
   {"shr",       0,      0,      0x11,   0x201,   0xb1,    0,      0  },    // C2 /5
   {"sal",       0,      0,      0x11,   0x201,   0xb1,    0,      0  },    // C2 /6
   {"sar",       0,      0,      0x11,   0x201,   0xb1,    0,      0  }};   // C2 /7

// Secondary opcode map for shift group 2. Opcode byte = 0xD1
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap21[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {"rol",       0,      0x1100, 0x11,   0x209,   0xb1,    0,      0  },    // C3 /0 rol word ptr r/m,1
   {"ror",       0,      0x1100, 0x11,   0x209,   0xb1,    0,      0  },    // C3 /1
   {"rcl",       0,      0x1100, 0x11,   0x209,   0xb1,    0,      0  },    // C3 /2
   {"rcr",       0,      0x1100, 0x11,   0x209,   0xb1,    0,      0  },    // C3 /3
   {"shl",       0,      0x1100, 0x11,   0x209,   0xb1,    0,      0  },    // C3 /4
   {"shr",       0,      0x1100, 0x11,   0x209,   0xb1,    0,      0  },    // C3 /5
   {"sal",       0,      0x1100, 0x11,   0x209,   0xb1,    0,      0  },    // C3 /6
   {"sar",       0,      0x1100, 0x11,   0x209,   0xb1,    0,      0  }};   // C3 /7

// Secondary opcode map for shift group 2. Opcode byte = 0xD2
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap22[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {"rol",       0,      0,      0x11,   0x201,   0x800,   0,      0  },    // C2 /0 rol byte ptr r/m,cl
   {"ror",       0,      0,      0x11,   0x201,   0x800,   0,      0  },    // C2 /1
   {"rcl",       0,      0,      0x11,   0x201,   0x800,   0,      0  },    // C2 /2
   {"rcr",       0,      0,      0x11,   0x201,   0x800,   0,      0  },    // C2 /3
   {"shl",       0,      0,      0x11,   0x201,   0x800,   0,      0  },    // C2 /4
   {"shr",       0,      0,      0x11,   0x201,   0x800,   0,      0  },    // C2 /5
   {"sal",       0,      0,      0x11,   0x201,   0x800,   0,      0  },    // C2 /6
   {"sar",       0,      0,      0x11,   0x201,   0x800,   0,      0  }};   // C2 /7

// Secondary opcode map for shift group 2. Opcode byte = 0xD3
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap23[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {"rol",       0,      0x1100, 0x11,   0x209,   0x800,   0,      0  },    // C3 /0 rol word ptr r/m,cl
   {"ror",       0,      0x1100, 0x11,   0x209,   0x800,   0,      0  },    // C3 /1
   {"rcl",       0,      0x1100, 0x11,   0x209,   0x800,   0,      0  },    // C3 /2
   {"rcr",       0,      0x1100, 0x11,   0x209,   0x800,   0,      0  },    // C3 /3
   {"shl",       0,      0x1100, 0x11,   0x209,   0x800,   0,      0  },    // C3 /4
   {"shr",       0,      0x1100, 0x11,   0x209,   0x800,   0,      0  },    // C3 /5
   {"sal",       0,      0x1100, 0x11,   0x209,   0x800,   0,      0  },    // C3 /6
   {"sar",       0,      0x1100, 0x11,   0x209,   0x800,   0,      0  }};   // C3 /7

// Secondary opcode map for group 3. Opcode byte = 0xF6
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap24[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {"test",      0,      0,      0x51,   0x201,  0x5000, 0,      4  },    // test rm8,ib
   {"test",      0,      0,      0x4051, 0x201,  0x5000, 0,      4  },    // test rm8,ib. undocumented
   {"not",       0,      0,      0x11,   0x201,  0,      0,      0  },    // not rm8
   {"neg",       0,      0,      0x11,   0x201,  0,      0,      0  },    // neg rm8
   {"mul",       0,      0,      0x11,   0x201,  0,      0,      8  },    // mul rm8
   {"imul",      0,      0,      0x11,   0x201,  0,      0,      8  },    // imul rm8
   {"div",       0,      0,      0x11,   0x201,  0,      0,      8  },    // div rm8
   {"idiv",      0,      0,      0x11,   0x201,  0,      0,      8  }};   // idiv rm8

// Secondary opcode map for group 3. Opcode byte = 0xF7
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap25[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {"test",      0,      0x1100, 0x91,   0x209,  0x5000, 0,      4  },    // test rm,i
   {"test",      0,      0x1100, 0x4091, 0x209,  0x5000, 0,      4  },    // test rm,i. undocumented
   {"not",       0,      0x1100, 0x11,   0x209,  0,      0,      0  },    // not rm
   {"neg",       0,      0x1100, 0x11,   0x209,  0,      0,      0  },    // neg rm
   {"mul",       0,      0x1100, 0x11,   0x209,  0,      0,      8  },    // mul rm
   {"imul",      0,      0x1100, 0x11,   0x209,  0,      0,      8  },    // imul rm
   {"div",       0,      0x1100, 0x11,   0x209,  0,      0,      8  },    // div rm
   {"idiv",      0,      0x1100, 0x11,   0x209,  0,      0,      8  }};   // idiv rm

// Secondary opcode map for group 4. Opcode byte = 0xFE
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap26[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {"inc",       0,      0x10,   0x11,   0x201,  0,      0,      0  },    // inc rm8
   {"dec",       0,      0x10,   0x11,   0x201,  0,      0,      0  },    // dec rm8
   {0,           0,      0,      0x4011, 0x201,  0,      0,      0  }};   // illegal opcode

// Secondary opcode map for group 5. Opcode byte = 0xFF
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap27[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {"inc",       0,      0x1110, 0x11,   0x209,  0,      0,     0x80},    // inc rm
   {"dec",       0,      0x1110, 0x11,   0x209,  0,      0,     0x80},    // dec rm
   {"call",      0,      0x2102, 0x11,   0x20C,  0,      0,      8  },    // call indirect rm
   {"call",      0,      0x1102, 0x811,  0x10D,  0,      0,      8  },    // call indirect far
   {"jmp",       0,      0x2100, 0x11,   0x20B,  0,      0,     0x14},    // jmp indirect rm
   {"jmp",       0,      0x1100, 0x811,  0x10D,  0,      0,     0x10},    // jmp indirect far
   {"push",      0,      0x2102, 0x11,   0x20A,  0,      0,      4  },    // push rm
   {0,           0,      0,      0x4011, 0x209,  0,      0,      0  }};   // illegal opcode

// Secondary opcode map for immediate group 1A. Opcode byte = 0x8F
// Indexed by reg bits = 0 - 7. Values != 0 are discouraged
SOpcodeDef OpcodeMap28[2] = {
//  name         instset prefix  format  dest.   source  link    options
   {"pop",       0,      0x2102, 0x11,   0x209,   0,      0,      0  },    // 8F
   {"pop",       0,      0x2102, 0x4011, 0x209,   0,      0,      0  }};   // 8F

// Tertiary opcode map for pinsrw. Opcode byte = 0F C4
// Indexed by mod bits 0 register vs. memory operand
SOpcodeDef OpcodeMap29[2] = {
//  name         instset prefix  format  dest.   source  link    options
   {"pinsrw",    7,      0x1200, 0x52,   0x32,   0x1102, 0,      0  },    // 0F C4 memory operand
   {"pinsrw",    7,      0x1200, 0x52,   0x32,   0x1009, 0,      0  }};   // 0F C4 register operand

// Tertiary opcode map for group 6. Opcode byte = 0F 00
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap2A[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {"sldt",      2,      0x1100, 0x11,   0x202,  0,      0,      0  },    // 
   {"str",       0x802,  0x0100, 0x11,   0x202,  0,      0,      0  },    // 
   {"lldt",      0x802,  0x2000, 0x11,   0,      0x102,  0,      0  },    // 
   {"ltr",       0x802,  0,      0x11,   0,      0x202,  0,      0  },    // 
   {"verr",      0x802,  0,      0x11,   0,      0x202,  0,      0  },    // 
   {"verw",      0x802,  0,      0x11,   0,      0x202,  0,      0  },    // 
   {0,           0,      0,      0x4011, 0,      0x202,  0,      0  },    // illegal
   {0,           0,      0,      0x4011, 0,      0x202,  0,      0  }};   // illegal

// Tertiary opcode map for group 7. Opcode byte = 0F 01
// Indexed by reg bits = 0 - 7 and mod = 11b. 
SOpcodeDef OpcodeMap2B[16] = {
//  name         instset prefix  format  dest.   source  link    options
   {"sgdt",      0x802,  0x1100, 0x11,   0x10D,  0,      0,      0  },    // 
   {"sidt",      0x802,  0x1100, 0x11,   0x10D,  0,      0,      0  },    // 
   {"lgdt",      0x802,  0x1100, 0x11,   0,      0x10D,  0,      0  },    // 
   {"lidt",      0x802,  0x1100, 0x11,   0,      0x10D,  0,      0  },    // 
   {"smsw",      2,      0,      0x11,   0x202,  0,      0,      0  },    // 
   {0,           0,      0,      0x4011, 0,      0,      0,      0  },    // illegal
   {"lmsw",      0x802,  0,      0x11,   0,      0x202,  0,      0  },    // 
   {"invlpg",    4,      0,      0x11,   0,      0x106,  0,      0  },    // 
   {0,           0x36,   0,      0x4011, 0,      0,      5,      0  },    // link to quarternary map, vmcall etc.
   {0,           0x37,   0,      0x4011, 0,      0,      5,      0  },    // link to quarternary map, monitor, mwait
   {0,           0,      0,      0x4011, 0,      0,      0,      0  },    // illegal
   {0,           0,      0,      0x4011, 0,      0,      0,      0  },    // illegal
   {"smsw",      2,      0x1100, 0x11,   0x209,  0,      0,      0  },    // 
   {0,           0,      0,      0x4011, 0,      0,      0,      0  },    // illegal
   {"lmsw",      0x802,  0,      0x11,   0,      0x202,  0,      0  },    // 
   {"swapgs",    0x800,  0,      0x10,   0,      0,      0,      0  }};   // instruction set unknown

// Secondary opcode map for group 8. Opcode byte = 0F BA: bt
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap2C[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x51,   0x209,  0,      0,      0  },    // Illegal
   {0,           0,      0,      0x51,   0x209,  0,      0,      0  },    // Illegal
   {0,           0,      0,      0x51,   0x209,  0,      0,      0  },    // Illegal
   {0,           0,      0,      0x51,   0x209,  0,      0,      0  },    // Illegal
   {"bt",        3,      0x1100, 0x51,   0x209,  0x1000, 0,      0  },    // 
   {"bts",       3,      0x1100, 0x51,   0x209,  0x1000, 0,      0  },    // 
   {"btr",       3,      0x1100, 0x51,   0x209,  0x1000, 0,      0  },    // 
   {"btc",       3,      0x1100, 0x51,   0x209,  0x1000, 0,      0  }};   // 

// Secondary opcode map for addsub. Opcode byte = 0F D0
// Indexed by prefix = none, 66, F2, F3
SOpcodeDef OpcodeMap2D[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {"addsub",    0x13,   0,      0x4012, 0x6F,   0x26F,  0,      0  },    // 0F D0. undefined
   {"addsubpd",  0x13,   0x200,  0x12,   0x6C,   0x26C,  0,      0  },    // 66 0F D0. addsubpd
   {"addsubps",  0x13,   0x800,  0x12,   0x6B,   0x26B,  0,      0  },    // F2 0F D0. addsubps
   {"addsub",    0x13,   0x400,  0x4012, 0x6F,   0x26F,  0,      0  }};   // F3 0F D0. undefined

// Secondary opcode map for group 10. Opcode byte = 0F B9
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap2E[1] = {
//  name         instset prefix  format  dest.   source  link    options
   {"ud1",       0,      0,      0x4012, 9,      0x206,  0,      0  }};   // Invalid opcode, possibly used for emulation

// Secondary opcode map for mov group 11. Opcode byte = 0xC6
// Indexed by reg bits = 0 - 7. Values != 0 are discouraged
SOpcodeDef OpcodeMap2F[2] = {
//  name         instset prefix  format  dest.   source  link    options
   {"mov",       0,      5,      0x51,   0x201,   0x1000, 0,      0  },    // C6 mov m,ib
   {"mov",       0,      5,      0x4051, 0x201,   0x1000, 0,      0  }};   // C6

// Secondary opcode map for mov group 11. Opcode byte = 0xC7
// Indexed by reg bits = 0 - 7. Values != 0 are discouraged
SOpcodeDef OpcodeMap30[2] = {
//  name         instset prefix  format  dest.   source  link    options
   {"mov",       0,      0x1105, 0x91,   0x209,   0x3000, 0,     0  },    // C7 mov m,iw
   {"mov",       0,      5,      0x4091, 0x209,   0x3000, 0,     0  }};   // C7

// Secondary opcode map for group 12. Opcode byte = 0F 71
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap31[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {0,           7,      0x200,  0x51,   0x32,   0x1000, 0,      0  },     // Illegal
   {0,           7,      0x200,  0x51,   0x32,   0x1000, 0,      0  },     // Illegal
   {"psrlw",     7,      0x200,  0x51,   0x32,   0x1000, 0,      0  },
   {0,           7,      0x200,  0x51,   0x32,   0x1000, 0,      0  },     // Illegal
   {"psraw",     7,      0x200,  0x51,   0x32,   0x1000, 0,      0  },
   {0,           7,      0x200,  0x51,   0x32,   0x1000, 0,      0  },     // Illegal
   {"psllw",     7,      0x200,  0x51,   0x32,   0x1000, 0,      0  },
   {0,           7,      0x200,  0x51,   0x32,   0x1000, 0,      0  }};    // Illegal

// Secondary opcode map for group 13. Opcode byte = 0F 72
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap32[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {0,           0x12,   0x200,  0x51,   0x33,   0x1000, 0,      0  },     // Illegal
   {0,           0x12,   0x200,  0x51,   0x33,   0x1000, 0,      0  },     // Illegal
   {"psrld",     0x12,   0x200,  0x51,   0x33,   0x1000, 0,      0  },
   {0,           0x12,   0x200,  0x51,   0x33,   0x1000, 0,      0  },     // Illegal
   {"psrad",     0x12,   0x200,  0x51,   0x33,   0x1000, 0,      0  },
   {0,           0x12,   0x200,  0x51,   0x33,   0x1000, 0,      0  },     // Illegal
   {"pslld",     0x12,   0x200,  0x51,   0x33,   0x1000, 0,      0  },
   {0,           0x12,   0x200,  0x51,   0x33,   0x1000, 0,      0  }};    // Illegal

// Secondary opcode map for group 14. Opcode byte = 0F 73
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap33[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {0,           0x12,   0x200,  0x51,   0x33,   0x1000, 0,      0  },     // Illegal
   {0,           0x12,   0x200,  0x51,   0x33,   0x1000, 0,      0  },     // Illegal
   {"psrlq",     0x12,   0x200,  0x51,   0x33,   0x1000, 0,      0  },
   {"psrldq",    0x12,   0x8200, 0x51,   0x24,   0x1000, 0,      0  },     // Not valid without 66 prefix
   {0,           0x12,   0x200,  0x51,   0x33,   0x1000, 0,      0  },     // Illegal
   {0,           0x12,   0x200,  0x51,   0x33,   0x1000, 0,      0  },     // Illegal
   {"psllq",     0x12,   0x200,  0x51,   0x33,   0x1000, 0,      0  },
   {"pslldq",    0x12,   0x8200, 0x51,   0x24,   0x1000, 0,      0  }};    // Not valid without 66 prefix

// Secondary opcode map for group 15. Opcode byte = 0F AE
// Indexed by reg bits = 0 - 7 and mod = 3 
SOpcodeDef OpcodeMap34[16] = {
//  name         instset prefix  format  dest.   source  link    options
   {"fxsave",    0x11,   0,      0x11,   0x106,  0,      0,      0  },     // 0
   {"fxrstor",   0x11,   0,      0x11,   0,      0x106,  0,      8  },     // 1
   {"ldmxcsr",   0x11,   0,      0x11,   0,      0x103,  0,      0  },     // 2
   {"stmxcsr",   0x11,   0,      0x11,   0x103,  0,      0,      0  },     // 3
   {0,           0,      0,      0x10,   0,      0,      0,      0  },     // Illegal
   {0,           0,      0,      0x10,   0,      0,      0,      0  },     // Illegal
   {0,           0,      0,      0x10,   0,      0,      0,      0  },     // Illegal
   {"clflush",   0x12,   0,      0x11,   0x106,  0,      0,      0  },     // 7
   {0,           0,      0,      0x10,   0,      0,      0,      0  },     // Illegal
   {0,           0,      0,      0x10,   0,      0,      0,      0  },     // Illegal
   {0,           0,      0,      0x10,   0,      0,      0,      0  },     // Illegal
   {0,           0,      0,      0x10,   0,      0,      0,      0  },     // Illegal
   {0,           0,      0,      0x10,   0,      0,      0,      0  },     // Illegal
   {"lfence",    0x12,   0,      0x10,   0,      0,      0,      0  },     // 1-5
   {"mfence",    0x12,   0,      0x10,   0,      0,      0,      0  },     // 1-6
   {"sfence",    0x12,   0,      0x10,   0,      0,      0,      0  }};    // 1-7

// Secondary opcode map for group 16. Opcode byte = 0F 18
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap35[5] = {
//  name         instset prefix  format  dest.   source  link    options
   {"prefetchnta",0x13,  0,      0x11,   0,      0x106,  0,      0  },    // 0F 18 /0
   {"prefetcht0", 0x13,  0,      0x11,   0,      0x106,  0,      0  },    // 0F 18 /1
   {"prefetcht1", 0x13,  0,      0x11,   0,      0x106,  0,      0  },    // 0F 18 /2
   {"prefetcht2", 0x13,  0,      0x11,   0,      0x106,  0,      0  },    // 0F 18 /3
   {"prefetch",   0x13,  0,      0x4011, 0,      0x106,  0,      0  }};   // 0F 18 /4+ illegal

// Quarternary opcode map for group 7. 0F 01 reg = 0
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap36[6] = {
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x4010, 0,      0,      0,      0  },    // illegal
   {"vmcall",    0x813,  0,      0x10,   0,      0,      0,      8  },    // Intel processor only?
   {"vmlaunch",  0x813,  0,      0x10,   0,      0,      0,      8  },    // Intel processor only?
   {"vmresume",  0x813,  0,      0x10,   0,      0,      0,      0  },    // Intel processor only?
   {"vmxoff",    0x813,  0,      0x10,   0,      0,      0,      0  },    // Intel processor only?
   {0,           0,      0,      0x4010, 0,      0,      0,      0  }};   // illegal

// Quarternary opcode map for group 7. 0F 01 reg = 1
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap37[5] = {
//  name         instset prefix  format  dest.   source  link    options
   {"monitor",   0x813,  4,      0x10,   0,      0,      0,      0  },    // 
   {"mwait",     0x813,  0,      0x10,   0,      0,      0,      0  },    // 
   {0,           0,      0,      0x10,   0,      0,      0,      0  }};   // illegal

// Tertiary opcode map for floating point CMP instructions. First two bytes = 0F C2
// Indexed by immediate byte following operands = 0 - 7
SOpcodeDef OpcodeMap38[9] = {
//  name         instset prefix  format  dest.   source  link    options
   {"cmpeq",     0x12,   0xE00,  0x52,   0x6F,   0x26F,  0,      1  },    // 0F C2 op 00: cmpeqps/pd
   {"cmplt",     0x12,   0xE00,  0x52,   0x6F,   0x26F,  0,      1  },    // 0F C2 op 01: 
   {"cmple",     0x12,   0xE00,  0x52,   0x6F,   0x26F,  0,      1  },    // 0F C2 op 02: 
   {"cmpunord",  0x12,   0xE00,  0x52,   0x6F,   0x26F,  0,      1  },    // 0F C2 op 03: 
   {"cmpneq",    0x12,   0xE00,  0x52,   0x6F,   0x26F,  0,      1  },    // 0F C2 op 04: 
   {"cmpnlt",    0x12,   0xE00,  0x52,   0x6F,   0x26F,  0,      1  },    // 0F C2 op 05: 
   {"cmpnle",    0x12,   0xE00,  0x52,   0x6F,   0x26F,  0,      1  },    // 0F C2 op 06: 
   {"cmpord",    0x12,   0xE00,  0x52,   0x6F,   0x26F,  0,      1  },    // 0F C2 op 07: 
   {"cmp",       0x12,   0xE00,  0x4052, 0x6F,   0x126F, 0,      1  }};   // 0F C2 op > 7: cmpps/pd, imm

// Secondary opcode map for cbw/cwde/cdqe. Opcode byte = 0x98
// Indexed by operand size = 16, 32, 64
SOpcodeDef OpcodeMap39[3] = {
//  name         instset prefix  format  dest.   source  link    options
   {"cbw",       0,      0x100,  1,      0,      0,      0,      8  },    // 98
   {"cwde",      0,      0x100,  1,      0,      0,      0,      8  },    // 98
   {"cdqe",      0x4000, 0x1000, 1,      0,      0,      0,      8  }};   // 98

// Secondary opcode map for cwd/cdq/cqo. Opcode byte = 0x99
// Indexed by operand size = 16, 32, 64
SOpcodeDef OpcodeMap3A[3] = {
//  name         instset prefix  format  dest.   source  link    options
   {"cwd",       0,      0x100,  1,      0,      0,      0,      8  },    // 99
   {"cdq",       0,      0x100,  1,      0,      0,      0,      8  },    // 99
   {"cqo",       0x4000, 0x1000, 1,      0,      0,      0,      8  }};   // 99


// Secondary opcode map for arpl/movsxd. Opcode byte = 0x63
// Indexed by mode = 16, 32, 64
SOpcodeDef OpcodeMap3B[3] = {
//  name         instset prefix  format  dest.   source  link    options
   {"arpl",      0x8802, 0,      0x13,   0x202,  2,       0,      0  },    // 63
   {"arpl",      0x8802, 0,      0x13,   0x202,  2,       0,      0  },    // 63
   {"movsxd",    0x4000, 0x1000, 0x12,   9,      0x203,   0,      0  }};   // 63

// Secondary opcode map for nop/pause. Opcode byte = 0x90
// Indexed by prefix = none, 66, F2, F3
SOpcodeDef OpcodeMap3C[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {"nop",       0,      0,      2,      0,      0,      0,     0x40},    // 90
   {"nop",       0,      0,      2,      0,      0,      0,     0x40},    // 66 90
   {"nop",       0,      0,      2,      0,      0,      0,     0x40},    // F2 90
   {"pause",     0,      0x400,  2,      0,      0,      0,      0  }};   // F3 90. (No instset indicated because backwards compatible)

// Secondary opcode map for jcxz. Opcode byte = 0xE3
// Indexed by address size
SOpcodeDef OpcodeMap3D[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {"jcxz",      0,      0x81,   0x42,   0x81,   0,      0,      0  },    // E3
   {"jecxz",     0,      0x81,   0x42,   0x81,   0,      0,      0  },
   {"jrcxz",     0x4000, 0x81,   0x42,   0x81,   0,      0,      0  }};

// Secondary opcode map for pushf/d/q. Opcode byte = 0x9C
// Indexed by operand size
SOpcodeDef OpcodeMap3E[3] = {
//  name         instset prefix  format  dest.   source  link    options
   {"pushf",     0,      0x0102, 2,      0,      0,      0,      0  },    // 9C pushf
   {"pushf",     0,      0x2102, 2,      0,      0,      0,      1  },    // 9C pushf/d/q
   {"pushf",     0,      0x2102, 2,      0,      0,      0,      1  }};   // 9C pushf/d/q

// Secondary opcode map for poof/d/q. Opcode byte = 0x9D
// Indexed by operand size
SOpcodeDef OpcodeMap3F[3] = {
//  name         instset prefix  format  dest.   source  link    options
   {"popf",      0,      0x0102, 2,      0,      0,      0,      0  },    // 9D popf
   {"popf",      0,      0x2102, 2,      0,      0,      0,      1  },    // 9D popf/d/q
   {"popf",      0,      0x2102, 2,      0,      0,      0,      1  }};   // 9D popf/d/q

// Tertiary opcode map for movups etc. Opcode byte = 0F 10
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap40[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {"movups",    0x11,   0,      0x12,   0x6B,   0x228,  0,      0  },    // 0F 10
   {"movupd",    0x12,   0x200,  0x12,   0x6C,   0x228,  0,      0  },    // 66 0F 10
   {"movsd",     0x12,   0x800,  0x12,   0x4C,   0x24C,  0,      0  },    // F2 0F 10
   {"movss",     0x12,   0x400,  0x12,   0x4B,   0x24B,  0,      0  }};   // F3 0F 10

// Tertiary opcode map for movups etc. Opcode byte = 0F 11
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap41[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {"movups",    0x11,   0,      0x13,   0x228,  0x6B,   0,      0  },    // 0F 11
   {"movupd",    0x12,   0x200,  0x13,   0x228,  0x6C,   0,      0  },    // 66 0F 11
   {"movsd",     0x12,   0x800,  0x13,   0x24C,  0x4C,   0,      0  },    // F2 0F 11
   {"movss",     0x12,   0x400,  0x13,   0x24B,  0x4B,   0,      0  }};   // F3 0F 11

// Tertiary opcode map for movlps etc. Opcode byte = 0F 12
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap42[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {0,           0x43,   0,      0,      0,      0,      3,      0  },    // Link to quarternary map
   {"movlpd",    0x12,   0x200,  0x12,   0x26C,  0x110,  0,      0  },    // 66 0F 12
   {"movddup",   0x13,   0x800,  0x12,   0x6C,   0x24C,  0,      0  },    // F2 0F 12
   {"movsldup",  0x13,   0x400,  0x12,   0x26B,  0x4B,   0,      0  }};   // F3 0F 12

// Quarternary opcode map for movlps and movhlps. Opcode byte = 0F 12
// Indexed by mod bits
SOpcodeDef OpcodeMap43[2] = {
//  name         instset prefix  format  dest.   source  link    options
   {"movlps",    0x11,   0,      0x12,   0x6B,   0x110,  0,      0  },    // 0F 12 (mem)
   {"movhlps",   0x11,   0,      0x12,   0x6B,   0x6B,   0,      0  }};   // 0F 12 (reg)

// Tertiary opcode map for movlps etc. Opcode byte = 0F 12
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap44[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {0,           0x45,   0,      0,      0,      0,      3,      0  },    // Link to quarternary map
   {"movhpd",    0x12,   0x200,  0x12,   0x6C,   0x14C,  0,      0  },    // 66 0F 16
   {0,           0x13,   0x800,  0x4012, 0x6C,   0x24C,  0,      0  },    // F2 0F 16
   {"movshdup",  0x13,   0x400,  0x12,   0x6B,   0x26B,  0,      0  }};   // F3 0F 16

// Quarternary opcode map for movhps and movlhps. Opcode byte = 0F 16
// Indexed by mod bits
SOpcodeDef OpcodeMap45[2] = {
//  name         instset prefix  format  dest.   source  link    options
   {"movhps",    0x11,   0,      0x12,   0x6B,   0x110,  0,      0  },    // 0F 12 (mem)
   {"movlhps",   0x11,   0,      0x12,   0x6B,   0x6B,   0,      0  }};   // 0F 12 (reg)

// Tertiary opcode map for cvtpi2ps etc. Opcode byte = 0F 2A
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap46[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {"cvtpi2ps",  0x11,   0,      0x12,   0x6B,   0x213,   0,     0  },    // 0F 2A
   {"cvtpi2pd",  0x12,   0x200,  0x12,   0x6C,   0x213,   0,     0  },    // 66 0F 2A
   {"cvtsi2sd",  0x12,   0x1800, 0x12,   0x4C,   0x209,   0,     0  },    // F2 0F 2A
   {"cvtsi2ss",  0x12,   0x1400, 0x12,   0x4B,   0x209,   0,     0  }};   // F3 0F 2A

// Tertiary opcode map for cvttps2pi etc. Opcode byte = 0F 2C
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap47[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {"cvttps2pi", 0x11,   0,      0x12,   0x13,   0x26B,  0,      0  },    // 0F 2C
   {"cvttpd2pi", 0x12,   0x200,  0x12,   0x13,   0x26C,  0,      0  },    // 66 0F 2C
   {"cvttsd2si", 0x12,   0x1800, 0x12,   0x09,   0x24C,  0,      0  },    // F2 0F 2C
   {"cvttss2si", 0x12,   0x1400, 0x12,   0x09,   0x24B,  0,      0  }};   // F3 0F 2C

// Tertiary opcode map for cvtps2pi etc. Opcode byte = 0F 2D
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap48[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {"cvtps2pi",  0x11,   0,      0x12,   0x13,   0x26B,  0,      0  },    // 0F 2D
   {"cvtpd2pi",  0x12,   0x200,  0x12,   0x13,   0x26C,  0,      0  },    // 66 0F 2D
   {"cvtsd2si",  0x12,   0x1800, 0x12,   0x09,   0x24C,  0,      0  },    // F2 0F 2D
   {"cvtss2si",  0x12,   0x1400, 0x12,   0x09,   0x24B,  0,      0  }};   // F3 0F 2D

// Tertiary opcode map for cvtps2pd etc. Opcode byte = 0F 5A
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap49[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {"cvtps2pd",  0x12,   0,      0x12,   0x6C,   0x26B,  0,      0  },    // 0F 5A
   {"cvtpd2ps",  0x12,   0x200,  0x12,   0x6B,   0x26C,  0,      0  },    // 66 0F 5A
   {"cvtsd2ss",  0x12,   0x800,  0x12,   0x4B,   0x24C,  0,      0  },    // F2 0F 5A
   {"cvtss2sd",  0x12,   0x400,  0x12,   0x4C,   0x24B,  0,      0  }};   // F3 0F 5A

// Tertiary opcode map for cvtdq2ps etc. Opcode byte = 0F 5B
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap4A[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {"cvtdq2ps",  0x12,   0,      0x12,   0x6B,   0x223,  0,      0  },    // 0F 5A
   {"cvtps2dq",  0x12,   0x200,  0x12,   0x23,   0x26B,  0,      0  },    // 66 0F 5B
   {0,           0x12,   0x800,  0x4012, 0x20,   0x220,  0,      0  },    // F2 0F 5B. Illegal
   {"cvttps2dq", 0x12,   0x400,  0x12,   0x23,   0x26B,  0,      0  }};   // F3 0F 5B

// Tertiary opcode map for ucomiss/sd etc. Opcode byte = 0F 2E
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap4B[3] = {
//  name         instset prefix  format  dest.   source  link    options
   {"ucomiss",   0x11,   0x200,  0x12,   0x6B,   0x24B,  0,      4  },    // 0F 2E. ucomiss
   {"ucomisd",   0x11,   0x200,  0x12,   0x6C,   0x24C,  0,      4  },    // 66 0F 2E. ucomisd
   {0,           0,      0,      0x12,   0x6F,   0x26F,  0,      0  }};   // Illegal

// Tertiary opcode map for comiss/sd etc. Opcode byte = 0F 2F
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap4C[3] = {
//  name         instset prefix  format  dest.   source  link    options
   {"comiss",    0x11,   0x200,  0x12,   0x6B,   0x24B,  0,      4  },    // 0F 2F. comiss
   {"comisd",    0x11,   0x200,  0x12,   0x6C,   0x24C,  0,      4  },    // 66 0F 2F. comisd
   {0,           0,      0,      0x12,   0x6F,   0x26F,  0,      0  }};   // Illegal

// Tertiary opcode map for movq/movdqa/movdqu. Opcode byte = 0F 6F
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap4D[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {"movq",      7,      0,      0x12,   0x10,   0x210,  0,      0  },    // 0F 6F
   {"movdqa",    0x12,   0x100,  0x12,   0x20,   0x220,  0,      0  },    // 66 0F 6F
   {0,           7,      0x800,  0x4012, 0x10,   0x210,  0,      0  },    // F2 0F 6F. Illegal
   {"movdqu",    0x12,   0x400,  0x12,   0x20,   0x228,  0,      0  }};   // F3 0F 6F

// Tertiary opcode map for movq/movdqa/movdqu. Opcode byte = 0F 7F
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap4E[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {"movq",      7,      0,      0x13,   0x210,  0x10,   0,      0  },    // 0F 6F
   {"movdqa",    0x12,   0x100,  0x13,   0x220,  0x20,   0,      0  },    // 66 0F 6F
   {0,           7,      0x800,  0x4013, 0x210,  0x10,   0,      0  },    // F2 0F 6F. Illegal
   {"movdqu",    0x12,   0x400,  0x13,   0x228,  0x20,   0,      0  }};   // F3 0F 6F

// Tertiary opcode map for pshufw etc. Opcode byte = 0F 70
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap4F[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {"pshufw",    7,      0,      0x52,   0x12,   0x1212, 0,      0  },    // 0F 70
   {"pshufd",    0x12,   0x100,  0x52,   0x23,   0x1223, 0,      0  },    // 66 0F 70
   {"pshuflw",   0x12,   0x800,  0x52,   0x22,   0x1222, 0,      0  },    // F2 0F 70
   {"pshufhw",   0x12,   0x400,  0x52,   0x22,   0x1222, 0,      0  }};   // F3 0F 70

// Tertiary opcode map for group 9. Opcode byte = 0F C7
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap50[8] = {
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0,      0,      0,      0,      0  },    // 0. Illegal
   {0,           0x51,   0x1010, 0,      0,      0,      8,      0  },    // 1. Link to quarternary map: cmpxchg8b
   {0,           0,      0,      0,      0,      0,      0,      0  },    // Illegal
   {0,           0,      0,      0,      0,      0,      0,      0  },    // Illegal
   {0,           0,      0,      0,      0,      0,      0,      0  },    // Illegal
   {0,           0,      0,      0,      0,      0,      0,      0  },    // Illegal
   {0,           0x52,   0x600,  0,      0,      0,      9,      0  },    // 6. Link to quarternary map: vmptrld
   {"vmptrst",   0x813,  0,      0x11,   0x110,  0,      0,      0  }};   // 7

// Quarternary opcode map for cmpxchg8b. Opcode byte = 0F C7 /1
// Indexed by operand size: 16, 32, 64
SOpcodeDef OpcodeMap51[3] = {
//  name         instset prefix  format  dest.   source  link    options
   {"cmpxchg8b", 5,      0x1010, 0x11,   0x110,  0,      0,      0  },    // 
   {"cmpxchg8b", 5,      0x1010, 0x11,   0x110,  0,      0,      0  },    // 
   {"cmpxchg16b",5,      0x1010, 0x11,   0x120,  0,      0,      0  }};

// Quarternary opcode map for cmpxchg8b. Opcode byte = 0F C7 /6
// Indexed by prefix: none/66/F2/F3
SOpcodeDef OpcodeMap52[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {"vmptrld",   0x813,  0,      0x11,   0x110,  0,      0,      0  },    // 
   {"vmclear",   0x813,  0x200,  0x11,   0x110,  0,      0,      0  },    // 
   {0,           0x813,  0x800,  0x11,   0x110,  0,      0,      0  },    // Illegal
   {"vmxon",     0x813,  0x400,  0x11,   0x110,  0,      0,      0  }};

// Quarternary opcode map for movdq2q etc. Opcode byte = 0F D6
// Indexed by prefix: none/66/F2/F3
SOpcodeDef OpcodeMap53[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0,      0,      0,      0,      0  },    // Illegal
   {"movq",      0x12,   0x200,  0x13,   0x220,  0x20,   0,      0  },    // 66: movq xmm,xmm
   {"movdq2q",   0x12,   0x800,  0x12,   0x10,   0x20,   0,      0  },    // F2
   {"movq2dq",   0x12,   0x400,  0x12,   0x20,   0x10,   0,      0  }};   // F3

// Quarternary opcode map for cvtpd2dq etc. Opcode byte = 0F E6
// Indexed by prefix: none/66/F2/F3
SOpcodeDef OpcodeMap54[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0,      0,      0,      0,      0  },    // Illegal
   {"cvttpd2dq", 0x12,   0x200,  0x12,   0x23,   0x26C,  0,      0  },    // 66: movq xmm,xmm
   {"cvtpd2dq",  0x12,   0x800,  0x12,   0x23,   0x26C,  0,      0  },    // F2
   {"cvtdq2pd",  0x12,   0x400,  0x12,   0x6C,   0x223,  0,      0  }};   // F3

// Quarternary opcode map for movntq etc. Opcode byte = 0F E7
// Indexed by prefix: none/66/F2/F3
SOpcodeDef OpcodeMap55[3] = {
//  name         instset prefix  format  dest.   source  link    options
   {"movntq",    0x11,   0,      0x13,   0x110,  0x10,   0,      0  },    // 
   {"movntdq",   0x12,   0x200,  0x13,   0x120,  0x20,   0,      0  },    // 
   {0,           0,      0,      0,      0,      0,      0,      0  }};   // Illegal

// Quarternary opcode map for lddqu. Opcode byte = 0F F0
// Indexed by prefix: none/66/F2/F3
SOpcodeDef OpcodeMap56[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x12,   0x20,   0x020,  0,      0  },    // Illegal
   {0,           0,      0x200,  0x12,   0x20,   0x020,  0,      0  },    // 
   {"lddqu",     0x13,   0x800,  0x12,   0x20,   0x128,  0,      0  },    // 
   {0,           0,      0x400,  0x12,   0x20,   0x020,  0,      0  }};   // Illegal

// Quarternary opcode map for maskmovq. Opcode byte = 0F F7
// Indexed by prefix: none/66/F2/F3
SOpcodeDef OpcodeMap57[3] = {
//  name         instset prefix  format  dest.   source  link    options
   {"maskmovq",  7,      5,      0x12,   0x10,   0x10,   0,      0  },    // 
   {"maskmovdqu",0x12,   0x205,  0x12,   0x20,   0x20,   0,      0  },    // 
   {0,           0,      0,      0x12,   0x20,   0x120,  0,      0  }};   // Illegal

// Tertiary opcode map for movd/movq. Opcode byte = 0F 6E
// Indexed by operand size 16/32/64
// First two lines are identical because operand size is determined only by REX.W prefix,
// while 66 prefix determines mmx or xmm register
SOpcodeDef OpcodeMap58[3] = {
//  name         instset prefix  format  dest.   source  link    options
   {"movd",      7,      0x1200, 0x12,   0x33,   0x203,  0,      0  },    // 0F 6E
   {"movd",      7,      0x1200, 0x12,   0x33,   0x203,  0,      0  },    // 0F 6E
   {"movd;Name varies: movq",0x4000,0x1200,0x12,0x33,0x204,0,    0  }};   // 0F 6E. Name varies: movd or movq, though the operand is 64 bits

// Tertiary opcode map for movd/movq. Opcode byte = 0F 7E
// Indexed by prefix: none/66/F2/F3
SOpcodeDef OpcodeMap59[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {0,           0x5A,   0,      0,      0,      0,      8,      0  },    // 0F 7E. Link to map 5A. Name depends on REX.W prefix
   {0,           0x5A,   0,      0,      0,      0,      8,      0  },    // 66 0F 7E. Link to map 5A. Name depends on REX.W prefix
   {0,           7,      0,      0x4013, 0x203,  0x33,   0,      0  },    // F2 0F 7E. Doesn't exist
   {0,           0x5B,   0,      0,      0,      0,      3,      0  }};   // F3 0F 7E. Link to map 5B. movq xmm,xmm/m64

// Quarternary opcode map for movd/movq. Opcode byte = 66 0F 7E
// Indexed by operand size 16/32/64
// First two lines are identical because operand size is determined only by REX.W prefix,
// while 66 prefix determines mmx or xmm register
SOpcodeDef OpcodeMap5A[3] = {
//  name         instset prefix  format  dest.   source  link    options
   {"movd",      7,      0x1200, 0x13,   0x203,  0x33,   0,      0  },    // 0F 7E
   {"movd",      7,      0x1200, 0x13,   0x203,  0x33,   0,      0  },    // 0F 7E
   {"movd;Name varies: movq",0x4000,0x1200,0x13,0x204,0x34,0,    0  }};   // 0F 7E. Name varies: movd or movq, though the operand is 64 bits

// Quarternary opcode map for movq xmm,xmm/m64. Opcode byte = F3 0F 7E
// Indexed by memory vs. register operand
SOpcodeDef OpcodeMap5B[2] = {
   {"movq",      0x12,   0x400,  0x12,   0x24,   0x210,  0,      0  },    // F3 0F 7E. movq xmm,m
   {"movq",      0x12,   0x400,  0x12,   0x24,   0x224,  0,      0  }};   // F3 0F 7E. movq xmm,xmm

// Tertiary opcode map for haddps/pd etc. Opcode byte = 0F 7C
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap5C[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x4012, 0x6F,   0x26F,  0,      0  },    // 0F 7C
   {"haddpd",    0x13,   0xA00,  0x12,   0x6C,   0x26C,  0,      0  },    // 66 0F 7C
   {"haddps",    0x13,   0xA00,  0x12,   0x6B,   0x26B,  0,      0  },    // F2 0F 7C
   {0,           0,      0,      0x4012, 0x6F,   0x26F,  0,      0  }};   // F3 0F 7C

// Tertiary opcode map for hsubps/pd etc. Opcode byte = 0F 7D
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap5D[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {0,           0,      0,      0x4012, 0x6F,   0x26F,  0,      0  },    // 0F 7D
   {"hsubpd",    0x13,   0xA00,  0x12,   0x6C,   0x26C,  0,      0  },    // 66 0F 7D
   {"hsubps",    0x13,   0xA00,  0x12,   0x6B,   0x26B,  0,      0  },    // F2 0F 7D
   {0,           0,      0,      0x4012, 0x6F,   0x26F,  0,      0  }};   // F3 0F 7D

// Tertiary opcode map for lar. Opcode byte = 0F 02
// Indexed by memory vs. register operand
SOpcodeDef OpcodeMap5E[2] = {
//  name         instset prefix  format  dest.   source  link    options
   {"lar",       0x802,  0x1100, 0x12,   9,      0x102,  0,      0  },    // 0F 02 memory
   {"lar",       0x802,  0x1100, 0x12,   9,      0x009,  0,      0  }};   // 0F 02 register

// Tertiary opcode map for lsl. Opcode byte = 0F 03
// Indexed by memory vs. register operand
SOpcodeDef OpcodeMap5F[2] = {
//  name         instset prefix  format  dest.   source  link    options
   {"lsl",       0x802,  0x1100, 0x12,   9,      0x102,  0,      0  },    // 0F 03 memory
   {"lsl",       0x802,  0x1100, 0x12,   9,      0x009,  0,      0  }};   // 0F 03 register

// Tertiary opcode map for popcnt. Opcode byte = 0F B8
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap60[4] = {
//  name         instset prefix  format  dest.   source  link    options
   {"jmpe;Itanium only", 0,0,    0x11,   0,      0,      0,      8  },    // 0F B8
   {0,           0,      0,      0,      0,      0,      0,      0  },    // 66 0F B8
   {0,           0,      0,      0,      0,      0,      0,      0  },    // F2 0F B8
   {"popcnt",    0x16,   0x1500, 0x12,   9,      0x209,  0,      0  }};   // F3 0F B8

// Quarternary opcode map for pextrb. Opcode byte = 0F 3A 14
// Indexed by memory vs. register operand
SOpcodeDef OpcodeMap61[2] = {
//  name         instset prefix  format  dest.   source  link    options
   {"pextrb",    0x15,   0x8200, 0x53,   0x101,  0x5021, 0,      0  },    // 0F 3A 14 memory
   {"pextrb",    0x15,   0x9200, 0x53,   0x009,  0x5021, 0,      0  }};   // 0F 3A 14 register

// Quarternary opcode map for pextrw. Opcode byte = 0F 3A 15
// Indexed by memory vs. register operand
SOpcodeDef OpcodeMap62[2] = {
//  name         instset prefix  format  dest.   source  link    options
   {"pextrw",    0x15,   0x9200, 0x53,   0x101,  0x5022, 0,      0  },    // 0F 3A 15 memory
   {"pextrw",    0x15,   0x9200, 0x53,   0x009,  0x5022, 0,      0  }};   // 0F 3A 15 register

// Quarternary opcode map for pextrd/q. Opcode byte = 0F 3A 16
// Indexed by operand size (16, 32, 64)
SOpcodeDef OpcodeMap63[3] = {
//  name         instset prefix  format  dest.   source  link    options
   {"pextrd",    0x15,   0x8200, 0x53,   0x203,  0x5023, 0,      1  },    // 0F 3A 16 pextrd
   {"pextrd",    0x15,   0x8200, 0x53,   0x203,  0x5023, 0,      1  },    // 0F 3A 16 pextrd
   {"pextrq",    0x15,   0x9200, 0x53,   0x204,  0x5024, 0,      1  }};   // 0F 3A 16 pextrq

// Quarternary opcode map for pinsrb. Opcode byte = 0F 3A 20
// Indexed by memory vs. register operand
SOpcodeDef OpcodeMap64[2] = {
//  name         instset prefix  format  dest.   source  link    options
   {"pinsrb",    0x15,   0x8200, 0x52,   0x21,   0x5001, 0,      0  },    // 0F 3A 20 memory
   {"pinsrb",    0x15,   0x8200, 0x52,   0x21,   0x500, 0,      0  }};   // 0F 3A 20 register

// Quarternary opcode map for crc32. Opcode byte = 0F 38 F1
// Indexed by operand size (16, 32, 64)
SOpcodeDef OpcodeMap65[3] = {
//  name         instset prefix  format  dest.   source  link    options
   {"crc32",     0x16,   0x9900, 0x12,   0x3,    0x202,  0,      0  },    // 0F 38 F1
   {"crc32",     0x16,   0x9900, 0x12,   0x3,    0x203,  0,      0  },    // 0F 38 F1
   {"crc32",     0x16,   0x9900, 0x12,   0x4,    0x204,  0,      0  }};   // 0F 38 F1


/************** Make pointers to all opcode maps ***************************/
const SOpcodeDef * const OpcodeTables[] = {    
   OpcodeMap0,  OpcodeMap1,  OpcodeMap2,  OpcodeMap3, 
   OpcodeMap4,  OpcodeMap5,  OpcodeMap6,  0,
   OpcodeMap8,  OpcodeMap9,  OpcodeMapA,  OpcodeMapB, 
   OpcodeMapC,  OpcodeMapD,  OpcodeMapE,  OpcodeMapF, 
   OpcodeMap10, OpcodeMap11, OpcodeMap12, OpcodeMap13,
   OpcodeMap14, OpcodeMap15, OpcodeMap16, OpcodeMap17,
   OpcodeMap18, OpcodeMap19, OpcodeMap1A, OpcodeMap1B,
   OpcodeMap1C, OpcodeMap1D, OpcodeMap1E, OpcodeMap1F,
   OpcodeMap20, OpcodeMap21, OpcodeMap22, OpcodeMap23,
   OpcodeMap24, OpcodeMap25, OpcodeMap26, OpcodeMap27,
   OpcodeMap28, OpcodeMap29, OpcodeMap2A, OpcodeMap2B,
   OpcodeMap2C, OpcodeMap2D, OpcodeMap2E, OpcodeMap2F,
   OpcodeMap30, OpcodeMap31, OpcodeMap32, OpcodeMap33,
   OpcodeMap34, OpcodeMap35, OpcodeMap36, OpcodeMap37,
   OpcodeMap38, OpcodeMap39, OpcodeMap3A, OpcodeMap3B,
   OpcodeMap3C, OpcodeMap3D, OpcodeMap3E, OpcodeMap3F,
   OpcodeMap40, OpcodeMap41, OpcodeMap42, OpcodeMap43,
   OpcodeMap44, OpcodeMap45, OpcodeMap46, OpcodeMap47,
   OpcodeMap48, OpcodeMap49, OpcodeMap4A, OpcodeMap4B, 
   OpcodeMap4C, OpcodeMap4D, OpcodeMap4E, OpcodeMap4F,
   OpcodeMap50, OpcodeMap51, OpcodeMap52, OpcodeMap53,
   OpcodeMap54, OpcodeMap55, OpcodeMap56, OpcodeMap57,
   OpcodeMap58, OpcodeMap59, OpcodeMap5A, OpcodeMap5B,
   OpcodeMap5C, OpcodeMap5D, OpcodeMap5E, OpcodeMap5F,
   OpcodeMap60, OpcodeMap61, OpcodeMap62, OpcodeMap63,
   OpcodeMap64, OpcodeMap65
};

// Macro to calculate the size of an array
#define TableSize(x) sizeof(x)/sizeof(x[0])

// size of each table pointed to by OpcodeTables[]
const uint32 OpcodeTableLength[] = {    
   TableSize(OpcodeMap0),  TableSize(OpcodeMap1),  TableSize(OpcodeMap2),  TableSize(OpcodeMap3), 
   TableSize(OpcodeMap4),  TableSize(OpcodeMap5),  TableSize(OpcodeMap6),  0,
   TableSize(OpcodeMap8),  TableSize(OpcodeMap9),  TableSize(OpcodeMapA),  TableSize(OpcodeMapB), 
   TableSize(OpcodeMapC),  TableSize(OpcodeMapD),  TableSize(OpcodeMapE),  TableSize(OpcodeMapF),
   TableSize(OpcodeMap10), TableSize(OpcodeMap11), TableSize(OpcodeMap12), TableSize(OpcodeMap13),
   TableSize(OpcodeMap14), TableSize(OpcodeMap15), TableSize(OpcodeMap16), TableSize(OpcodeMap17),
   TableSize(OpcodeMap18), TableSize(OpcodeMap19), TableSize(OpcodeMap1A), TableSize(OpcodeMap1B),
   TableSize(OpcodeMap1C), TableSize(OpcodeMap1D), TableSize(OpcodeMap1E), TableSize(OpcodeMap1F),
   TableSize(OpcodeMap20), TableSize(OpcodeMap21), TableSize(OpcodeMap22), TableSize(OpcodeMap23),
   TableSize(OpcodeMap24), TableSize(OpcodeMap25), TableSize(OpcodeMap26), TableSize(OpcodeMap27),
   TableSize(OpcodeMap28), TableSize(OpcodeMap29), TableSize(OpcodeMap2A), TableSize(OpcodeMap2B),
   TableSize(OpcodeMap2C), TableSize(OpcodeMap2D), TableSize(OpcodeMap2E), TableSize(OpcodeMap2F),
   TableSize(OpcodeMap30), TableSize(OpcodeMap31), TableSize(OpcodeMap32), TableSize(OpcodeMap33),
   TableSize(OpcodeMap34), TableSize(OpcodeMap35), TableSize(OpcodeMap36), TableSize(OpcodeMap37),
   TableSize(OpcodeMap38), TableSize(OpcodeMap39), TableSize(OpcodeMap3A), TableSize(OpcodeMap3B),
   TableSize(OpcodeMap3C), TableSize(OpcodeMap3D), TableSize(OpcodeMap3E), TableSize(OpcodeMap3F), 
   TableSize(OpcodeMap40), TableSize(OpcodeMap41), TableSize(OpcodeMap42), TableSize(OpcodeMap43),
   TableSize(OpcodeMap44), TableSize(OpcodeMap45), TableSize(OpcodeMap46), TableSize(OpcodeMap47),
   TableSize(OpcodeMap48), TableSize(OpcodeMap49), TableSize(OpcodeMap4A), TableSize(OpcodeMap4B), 
   TableSize(OpcodeMap4C), TableSize(OpcodeMap4D), TableSize(OpcodeMap4E), TableSize(OpcodeMap4F),
   TableSize(OpcodeMap50), TableSize(OpcodeMap51), TableSize(OpcodeMap52), TableSize(OpcodeMap53),
   TableSize(OpcodeMap54), TableSize(OpcodeMap55), TableSize(OpcodeMap56), TableSize(OpcodeMap57),
   TableSize(OpcodeMap58), TableSize(OpcodeMap59), TableSize(OpcodeMap5A), TableSize(OpcodeMap5B), 
   TableSize(OpcodeMap5C), TableSize(OpcodeMap5D), TableSize(OpcodeMap5E), TableSize(OpcodeMap5F), 
   TableSize(OpcodeMap60), TableSize(OpcodeMap61), TableSize(OpcodeMap62), TableSize(OpcodeMap63), 
   TableSize(OpcodeMap64), TableSize(OpcodeMap65)
};

// number of entries in OpcodeTables
const uint32 NumOpcodeTables1 = TableSize(OpcodeTables);         
const uint32 NumOpcodeTables2 = TableSize(OpcodeTableLength);         


// Define register names

// Names of 8 bit registers
const char * RegisterNames8[8] = {
   "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };

// Names of 8 bit registers with REX prefix
const char * RegisterNames8x[16] = {
   "al", "cl", "dl", "bl", "spl", "bpl", "sil", "dil",
   "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b"  };

// Names of 16 bit registers
const char * RegisterNames16[16] = {
   "ax", "cx", "dx", "bx", "sp", "bp", "si", "di",
   "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w" };
      
// Names of 32 bit registers
const char * RegisterNames32[16] = {
   "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi",
   "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d" };
      
// Names of 64 bit registers
const char * RegisterNames64[16] = {
   "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
   "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15" };
      
// Names of segment registers
const char * RegisterNamesSeg[8] = {
   "es", "cs", "ss", "ds", "fs", "gs", "hs?", "is?" }; // Last two are illegal or undocumented

// Names of control registers
const char * RegisterNamesCR[16] = {
   "cr0", "cr1 ?", "cr2", "cr3", "cr4", "cr5 ?", "cr6 ?", "cr7 ?",
   "cr8", "cr9 ?", "cr10 ?", "cr11 ?", "cr12 ?", "cr13 ?", "cr14 ?", "cr15 ?" }; // Those with ? are illegal
