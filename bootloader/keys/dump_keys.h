/*
 * Ряженка — automatic key dumping at launch (DBI key subset).
 *
 * Key-derivation logic and source constants are ported from Lockpick_RCM
 * (Copyright (c) 2019-2022 shchmue), GPLv2, adapted to hekate's BDK.
 *
 * This derives the NCA-decryption keyset that tools like DBI need
 * (master keys, header_key, key_area_key_*, titlekek) and writes prod.keys.
 */
#ifndef _RYAZHENKA_DUMP_KEYS_H_
#define _RYAZHENKA_DUMP_KEYS_H_

#include <utils/types.h>

// dump mode: 0 = disabled, 1 = once (skip if prod.keys exists), 2 = force.
#define RYAZHENKA_DUMP_OFF   0
#define RYAZHENKA_DUMP_ONCE  1
#define RYAZHENKA_DUMP_FORCE 2

/*
 * Derive and save sd:/switch/prod.keys. Designed to never abort the boot:
 * on any failure it prints a warning and returns. `emummc` only affects the
 * informational log (console keys are identical for sysMMC and emuMMC).
 */
void ryazhenka_dump_keys(int mode, bool emummc);

#endif
