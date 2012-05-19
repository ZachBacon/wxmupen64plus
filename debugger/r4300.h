/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   wxMupen64Plus debugger                                                *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2012 Markus Heikkinen                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef R4300_H
#define R4300_H

enum R4300opcodes
{
    special_op = 0,
    regimm_op  = 1,
    j       = 2,    // jump
    jal     = 3,    // jump and link
    beq     = 4,    // branch on equal
    bne     = 5,    // branch on not equal
    blez    = 6,    // branch on less or equal than zero (signed)
    bgtz    = 7,    // branch on greater than zero (signed)
    addi    = 8,    // add immediate
    addiu   = 9,    // add unsigned imm
    slti    = 10,   // set on less than imm
    sltiu   = 11,   // set on less than unsigned imm
    andi    = 12,   // and imm
    ori     = 13,   // or imm
    xori    = 14,   // xor imm
    lui     = 15,   // load upper imm
    cop0_op = 16,
    cop1_op = 17,
    beql    = 20,   // branch equal likely
    bnel    = 21,   // branch not equal likely
    blezl   = 22,   // branch less than or equal to zero likely (signed)
    bgtzl   = 23,   // branch greater than zero likely (signed)
    daddi   = 24,   // dword add immediate
    daddiu  = 25,   // dword add immediate unsigned

    ldl     = 26,   // load dword left
    ldr     = 27,   // load dword rigt
    lb      = 32,   // load byte
    lh      = 33,   // load hword
    lwl     = 34,   // load word left
    lw      = 35,   // load word
    lbu     = 36,   // load byte unsigned
    lhu     = 37,   // load hword unsigned
    lwr     = 38,   // load word right
    lwu     = 39,   // load word unsigned
    sb      = 40,   // store byte
    sh      = 41,   // store hword
    swl     = 42,   // store word left
    sw      = 43,   // store word
    sdl     = 44,   // store dword left
    sdr     = 45,   // store dword right
    swr     = 46,   // store word right
    cache   = 47,   // cache (duh)
    ll      = 48,   // load linked word (atomic)
    lwc1    = 49,   // load word to cop1
    lld     = 52,   // load linked dword (atomic)
    ldc1    = 53,   // load word to cop1
    ld      = 55,   // load dword
    sc      = 56,   // store conditional word (atomic)
    swc1    = 57,   // store word from cop1
    scd     = 60,   // store conditional dword (atomic)
    sdc1    = 61,   // store dword from
    sd      = 63,   // store dword
};

#endif // R4300_H

