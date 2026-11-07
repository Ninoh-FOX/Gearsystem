/*
 * Gearsystem - Sega Master System / Game Gear Emulator
 * Copyright (C) 2013  Ignacio Sanchez

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 *
 */

// Dual Gear-to-Gear link: both Game Gears of a netplay session run on each
// console, joined by the cable in memory (LibretroLink), and only the buttons
// travel over the network.
//
// Instance 0 is always the player's own Game Gear: its ROM, its save (the
// frontend's .srm) and the screen and sound the player gets. Instance 1 is a
// copy of the other console's Game Gear, made from the state that console
// sends once when the link starts; it is never seen nor heard (no picture,
// no sound: only what its game can read). Slot 0 is the netplay host's Game
// Gear, slot 1 the client's: each console runs both slots, in the same
// order, so both compute the very same machines. Everything here depends on
// emulated time only, never on how fast each console happens to run.

#ifndef LIBRETRO_DUAL_H
#define LIBRETRO_DUAL_H

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include "libretro.h"

// Buttons as they travel: the pad of a Game Gear, already resolved.
#define DUAL_KEY_UP     0x01
#define DUAL_KEY_DOWN   0x02
#define DUAL_KEY_LEFT   0x04
#define DUAL_KEY_RIGHT  0x08
#define DUAL_KEY_1      0x10
#define DUAL_KEY_2      0x20
#define DUAL_KEY_START  0x40
#define DUAL_KEY_RESET  0x80000000u   // With the buttons: both consoles restart

// What libretro.cpp does for the session.
struct dual_hooks
{
    // Restart the player's Game Gear (keeping its save) and take its state.
    bool (*capture)(std::vector<uint8_t>& blob);
    // Both Game Gears on the cable: ours from `own`, the other one from `peer`.
    // `my_slot` is the slot of ours.
    bool (*link)(unsigned my_slot, const uint8_t* own, size_t own_size,
                 const uint8_t* peer, size_t peer_size);
    // Back to the player's Game Gear alone.
    void (*unlink)(void);
    // Buttons for the next frame, by slot.
    void (*set_keys)(unsigned slot, uint32_t keys);
    // Both Game Gears restart, with the cable plugged.
    void (*reset_all)(void);
    // What the game of a slot can see, hashed.
    uint32_t (*hash)(unsigned slot);
    void (*message)(const char* text, unsigned ms);
};

void dual_set_hooks(const struct dual_hooks* hooks);
// The game: its identity (ROM, BIOS) and the settings that change the machine.
void dual_game_loaded(uint32_t rom_crc, uint32_t cfg, double fps);
void dual_set_cfg(uint32_t cfg);

// Netpacket interface (libretro.cpp passes the frontend's calls on).
void dual_net_start(uint16_t client_id, retro_netpacket_send_t send,
                    retro_netpacket_poll_receive_t poll_receive);
void dual_net_receive(const void* buf, size_t len, uint16_t client_id);
void dual_net_stop(void);
bool dual_net_connected(uint16_t client_id);
void dual_net_disconnected(uint16_t client_id);

// Whether the link is being set up (true: the game waits this frame).
bool dual_setting_up(void);
// Whether both Game Gears are running.
bool dual_running(void);
// Whether a session uses (or may still use) the dual link: savestates,
// cheats and setting changes wait until it ends.
bool dual_active(void);
void dual_request_reset(void);

enum { DUAL_FRAME_WAIT = 0, DUAL_FRAME_RUN, DUAL_FRAME_HOLD };
int dual_frame_begin(uint32_t local_keys);
void dual_frame_end(void);
void dual_shutdown(void);

#endif /* LIBRETRO_DUAL_H */
