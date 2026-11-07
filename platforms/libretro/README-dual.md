# Gear-to-Gear Dual Link (netplay)

With the **Netplay Gear-to-Gear Dual Link** core option (on by default), a
netplay session with a Game Gear game runs **both Game Gears on each
console**: the player's own one, and a hidden copy of the other player's.
The two are joined by the Gear-to-Gear cable in memory, and only the
buttons travel over the network. The game's link traffic never waits for
the WiFi. The price is a few frames of input delay (2 or 3 on a local
network), chosen automatically from the round trip time.

- No ROM is sent: both consoles must have the very same dump (same CRC).
- No save is sent by hand: each console keeps its own `.srm`. When the link
  starts, each console sends its Game Gear once (savestate with its
  cartridge RAM, run length coded) so that the other one can run a copy of
  it. When the session ends, each console has only its own save, changed by
  its own game.
- The hidden copy draws no picture and makes no sound: it only computes what
  its game can read (CPU, memory, VDP registers and sprite flags, the cable).

## Settings that must match

| what | where |
|---|---|
| the game | the ROM file: the same dump on both consoles |
| System, Region, Mapper, Timing | core options `gearsystem_system`, `gearsystem_region`, `gearsystem_mapper`, `gearsystem_timing` |
| Game Gear BIOS | core option **Game Gear BIOS** and, if enabled, the same `bios.gg` |
| Sprite limit | core option **No Sprite Limit** (it decides the sprite overflow flag) |
| the core | the same build on both consoles |

If something differs, both consoles say **"Gear-to-Gear: games or settings
differ, no link"** and each game goes on alone.

Options changed in the menu while linked (picture, sound, controls...) wait
until the link ends.

## How a session goes

1. Both consoles load the same game with the option enabled; one hosts a
   netplay session, the other joins.
2. The client says HELLO with its ROM and settings; the host compares them.
3. The host measures the round trip and picks the input delay.
4. Each console restarts its game once (games look for the cable at boot),
   keeping its save, and sends that Game Gear to the other one.
5. From then on only the buttons go. Every 2 seconds both consoles compare
   both Game Gears; if they differ twice in a row the link is cut with a
   message ("the consoles went out of sync").
6. Both consoles show the same frame at the same time: the one that is
   ahead holds a frame now and then. If the WiFi has a bad moment, a console
   sends its buttons a frame earlier (up to 4 more), and takes that frame
   off again after a calm minute.

A reset while linked restarts both games on both consoles in the same
frame. While linked, savestates cannot be loaded and cheats are off. When a
console leaves, each player's Game Gear goes on alone.

With the option enabled, netplay with a Game Gear game always uses the dual
link (2 players). Master System games, and Game Gear games with the option
disabled, use the usual RetroArch netplay. The local two screen link
(**Game Link Cable Enable**) is separate: with it on, the dual link is not
used.

## Code

| file | what |
|---|---|
| `libretro_dual.cpp/.h` | the session: protocol, input delay, time sync, agreement checks |
| `libretro.cpp` | hooks: take/load the machines, buttons, reset, hash, netpacket interface |
| `libretro_link.cpp/.h` | the cable between the two Game Gears (`SetFirst`: same order on both consoles; `SetHeadless`; one machine's state) |
| `src/Video.*`, `src/Audio.*` | `SetHeadless`: no picture, no sound |

## Build

    make platform=miyoomini

gives `gearsystem_libretro.so` (toolchain in `/opt/miyoomini-toolchain`).

## Test without consoles

`dualtest/` runs two consoles in one process, as RetroArch would, joined by
a network in memory, each on its own thread at 59.92 frames a second
(needs a native build of the core):

    make CC=gcc CXX=g++ AR=ar
    python3 dualtest/makerom.py linktest.gg
    gcc -O2 -o dualtest/dualtest dualtest/dualtest.c -ldl -lpthread
    dualtest/dualtest $PWD/gearsystem_libretro.so linktest.gg 900 5 120

Arguments: core, ROM, frames, one way latency (ms), frame the session
starts. `linktest.gg` sends its pad through the parallel port and a counter
through the serial port every frame, and keeps what it gets; each console
starts with its own save (A0, A1). The test checks that each console keeps
its own save and gets the other one's pad.

`GEARSYSTEM_DUAL_DEBUG=1` prints every agreement check,
`GEARSYSTEM_DUAL_FAULT=1` gives the hidden copy other buttons for a while
(the check must cut the link), `ROM2=file` gives the client another game,
`RESET_AT=n` resets the host, `STOP_AT=n` ends the session, `DUAL_OFF=1`
disables the option.
