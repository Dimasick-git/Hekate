/*
 * Copyright (c) 2018-2025 CTCaer
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <bdk.h>

#include "hos/hos.h"

typedef struct _hekate_config
{
	// Non-volatile config.
	u32 autoboot;
	u32 autoboot_list;
	u32 bootwait;
	u32 noticker;
	u32 backlight;
	u32 autohosoff;
	u32 autonogc;
	u32 updater2p;
	u32 bootprotect;
	u32 autokeys;   // Ряженка: снять ключи бандлом Lockpick, если нет switch/prod.keys.
	u32 autosecmon; // Ряженка: pkg3 берёт atmosphere/exosphere.bin с SD, как fusee.
	// Global temporary config.
	bool t210b01;
	bool devmode;
	bool emummc_force_disable;
	bool rcm_patched;
	u32  errors;
	hos_eks_mbr_t *eks;
} hekate_config;

extern hekate_config h_cfg;

void set_default_configuration();

#endif /* _CONFIG_H_ */
