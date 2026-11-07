#!/usr/bin/env python3
# Builds linktest.gg: a Game Gear program that uses the Gear-to-Gear cable
# every frame, for the dual link test (dualtest.c).
#
#   C000  mix of everything received (changes if any byte differs)
#   C001  frame counter (sent through the serial port)
#   C002  last byte received through the serial port
#   C010  cartridge RAM byte 0 at boot: the save this Game Gear started with
#   C100+ ring: the other Game Gear's pad bits (up/down), per frame
#   8001  (cartridge RAM) C000 again: the save changes while it plays
import sys

prog = bytes([
    0xF3,                   # 0000 di
    0x31, 0xF0, 0xDF,       # 0001 ld sp,DFF0
    0x3E, 0x08,             # 0004 ld a,08
    0x32, 0xFC, 0xFF,       # 0006 ld (FFFC),a     cartridge RAM at 8000
    0x3A, 0x00, 0x80,       # 0009 ld a,(8000)
    0x32, 0x10, 0xC0,       # 000C ld (C010),a     the save of this one
    0x3E, 0x7C,             # 000F ld a,7C
    0xD3, 0x02,             # 0011 out (2),a       PC0-1 out, PC2-6 in
    0x3E, 0x30,             # 0013 ld a,30
    0xD3, 0x05,             # 0015 out (5),a       serial TX and RX on
    # loop: wait for line C0
    0xDB, 0x7E,             # 0017 in a,(7E)
    0xFE, 0xC0,             # 0019 cp C0
    0x20, 0xFA,             # 001B jr nz,0017
    0xDB, 0xDC,             # 001D in a,(DC)
    0x2F,                   # 001F cpl
    0xE6, 0x03,             # 0020 and 3           up/down pressed
    0xD3, 0x01,             # 0022 out (1),a
    0xDB, 0x01,             # 0024 in a,(1)
    0x0F, 0x0F,             # 0026 rrca, rrca      the other's PC0-1
    0xE6, 0x03,             # 0028 and 3
    0x47,                   # 002A ld b,a
    0x3A, 0x01, 0xC0,       # 002B ld a,(C001)
    0x3C,                   # 002E inc a
    0x32, 0x01, 0xC0,       # 002F ld (C001),a
    0xD3, 0x03,             # 0032 out (3),a       serial send
    0xDB, 0x04,             # 0034 in a,(4)        serial receive
    0x32, 0x02, 0xC0,       # 0036 ld (C002),a
    0x80,                   # 0039 add a,b
    0x4F,                   # 003A ld c,a
    0x3A, 0x00, 0xC0,       # 003B ld a,(C000)
    0x07,                   # 003E rlca
    0xA9,                   # 003F xor c
    0x32, 0x00, 0xC0,       # 0040 ld (C000),a
    0x32, 0x01, 0x80,       # 0043 ld (8001),a
    0x3A, 0x01, 0xC0,       # 0046 ld a,(C001)
    0x6F,                   # 0049 ld l,a
    0x26, 0xC1,             # 004A ld h,C1
    0x70,                   # 004C ld (hl),b
    0xDB, 0x7E,             # 004D in a,(7E)       wait for the line to end
    0xFE, 0xC0,             # 004F cp C0
    0x28, 0xFA,             # 0051 jr z,004D
    0xC3, 0x17, 0x00,       # 0053 jp 0017
])

rom = bytearray(128 * 1024)
rom[:len(prog)] = prog
rom[0x7FF0:0x7FF8] = b"TMR SEGA"
csum = sum(rom[:0x7FF0]) & 0xFFFF
rom[0x7FFA] = csum & 0xFF
rom[0x7FFB] = csum >> 8
rom[0x7FFF] = 0x6F          # Game Gear export, 128 KB
open(sys.argv[1] if len(sys.argv) > 1 else "linktest.gg", "wb").write(rom)
