/*
 * hekate (Ряженка ecosystem) — automatic key dumping at launch (DBI key subset).
 *
 * Copyright (c) 2019-2022 shchmue          (Lockpick_RCM key derivation)
 * Copyright (c) 2018      Atmosphère-NX     (key derivation algorithms)
 * Adapted for the Ряженка / hekate launch flow.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#include "dump_keys.h"
#include "key_versions.h"

#include <bdk.h>

#include <mem/heap.h>
#include <mem/minerva.h>
#include <sec/se.h>
#include <sec/se_t210.h>
#include <sec/tsec.h>
#include <soc/fuse.h>
#include <storage/sd.h>
#include <utils/types.h>
#include <libs/fatfs/ff.h>

#include "../config.h"
#include "../gfx/gfx.h"

#include <string.h>

extern hekate_config h_cfg;

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

// Lockpick_RCM keyslots (mapped onto hekate's SE keyslots).
#define KS_AES_ECB       8
#define KS_MARIKO_KEK    12
#define KS_TSEC          12
#define KS_SECURE_BOOT   14
#define KS_TSEC_ROOT_DEV 11
#define KS_TSEC_ROOT     13

// ---------------------------------------------------------------------------
// Source constants (NCA key derivation). Byte-exact, ported from Lockpick_RCM.
// ---------------------------------------------------------------------------

static const u8 aes_kek_generation_source[0x10] __attribute__((aligned(4))) = {
    0x4D, 0x87, 0x09, 0x86, 0xC4, 0x5D, 0x20, 0x72, 0x2F, 0xBA, 0x10, 0x53, 0xDA, 0x92, 0xE8, 0xA9};

static const u8 aes_key_generation_source[0x10] __attribute__((aligned(4))) = {
    0x89, 0x61, 0x5E, 0xE0, 0x5C, 0x31, 0xB6, 0x80, 0x5F, 0xE5, 0x8F, 0x3D, 0xA2, 0x4F, 0x7A, 0xA8};

static const u8 header_kek_source[0x10] __attribute__((aligned(4))) = {
    0x1F, 0x12, 0x91, 0x3A, 0x4A, 0xCB, 0xF0, 0x0D, 0x4C, 0xDE, 0x3A, 0xF6, 0xD5, 0x23, 0x88, 0x2A};

static const u8 header_key_source[0x20] __attribute__((aligned(4))) = {
    0x5A, 0x3E, 0xD8, 0x4F, 0xDE, 0xC0, 0xD8, 0x26, 0x31, 0xF7, 0xE2, 0x5D, 0x19, 0x7B, 0xF5, 0xD0,
    0x1C, 0x9B, 0x7B, 0xFA, 0xF6, 0x28, 0x18, 0x3D, 0x71, 0xF6, 0x4D, 0x73, 0xF1, 0x50, 0xB9, 0xD2};

static const u8 key_area_key_sources[3][0x10] __attribute__((aligned(4))) = {
    {0x7F, 0x59, 0x97, 0x1E, 0x62, 0x9F, 0x36, 0xA1, 0x30, 0x98, 0x06, 0x6F, 0x21, 0x44, 0xC3, 0x0D}, // application
    {0x32, 0x7D, 0x36, 0x08, 0x5A, 0xD1, 0x75, 0x8D, 0xAB, 0x4E, 0x6F, 0xBA, 0xA5, 0x55, 0xD8, 0x82}, // ocean
    {0x87, 0x45, 0xF1, 0xBB, 0xA6, 0xBE, 0x79, 0x64, 0x7D, 0x04, 0x8B, 0xA6, 0x7B, 0x5F, 0xDA, 0x4A}, // system
};

// master_kek_sources / mariko_master_kek_sources / master_key_vectors /
// master_key_source / titlekek_source / package2_key_source.
#include "key_sources.inl"

// ---------------------------------------------------------------------------
// Trimmed key storage (DBI subset only).
// ---------------------------------------------------------------------------

typedef struct {
    u8 temp_key[SE_KEY_128_SIZE];
    u8 header_key[SE_KEY_128_SIZE * 2];
    u8 key_area_key[3][KB_FIRMWARE_VERSION_MAX + 1][SE_KEY_128_SIZE];
    u8 master_kek[KB_FIRMWARE_VERSION_MAX + 1][SE_KEY_128_SIZE];
    u8 master_key[KB_FIRMWARE_VERSION_MAX + 1][SE_KEY_128_SIZE];
    u8 package2_key[KB_FIRMWARE_VERSION_MAX + 1][SE_KEY_128_SIZE];
    u8 titlekek[KB_FIRMWARE_VERSION_MAX + 1][SE_KEY_128_SIZE];
} key_storage_t;

// ---------------------------------------------------------------------------
// Crypto helpers (hekate BDK).
// ---------------------------------------------------------------------------

static inline void se_aes_crypt_block_ecb(u32 ks, int enc, void *dst, const void *src) {
    se_aes_crypt_ecb(ks, enc, dst, src, SE_KEY_128_SIZE);
}

static int key_exists(const void *data) {
    return memcmp(data, "\x00\x00\x00\x00\x00\x00\x00\x00", 8) != 0;
}

// se_aes_key_set(access) then ECB-decrypt key_source -> out_key.
static void load_aes_key(u32 ks, void *out_key, const void *access_key, const void *key_source) {
    se_aes_key_set(ks, access_key, SE_KEY_128_SIZE);
    se_aes_crypt_block_ecb(ks, DECRYPT, out_key, key_source);
}

// spl::GenerateAesKek (non-device-unique path only; seal mask 0 == zeros).
static void generate_aes_kek(u32 ks, key_storage_t *keys, void *out_kek, const void *kek_source, u32 generation) {
    if (generation)
        generation--;
    memcpy(keys->temp_key, keys->master_key[generation], SE_KEY_128_SIZE);
    se_aes_key_set(ks, keys->temp_key, SE_KEY_128_SIZE);
    se_aes_unwrap_key(ks, ks, aes_kek_generation_source);
    se_aes_crypt_block_ecb(ks, DECRYPT, out_kek, kek_source);
}

// spl::GenerateAesKey.
static void generate_aes_key(u32 ks, void *out_key, u32 key_size, const void *access_key, const void *key_source) {
    u8 aes_key[SE_KEY_128_SIZE] __attribute__((aligned(4)));
    load_aes_key(ks, aes_key, access_key, aes_key_generation_source);
    se_aes_key_set(ks, aes_key, SE_KEY_128_SIZE);
    se_aes_crypt_ecb(ks, DECRYPT, out_key, key_source, key_size);
}

static void fs_derive_header_key(key_storage_t *keys, void *out_key) {
    if (!key_exists(keys->master_key[0]))
        return;
    u8 access_key[SE_KEY_128_SIZE] __attribute__((aligned(4)));
    generate_aes_kek(KS_AES_ECB, keys, access_key, header_kek_source, 0);
    generate_aes_key(KS_AES_ECB, out_key, sizeof(header_key_source), access_key, header_key_source);
}

static void fs_derive_key_area_key(key_storage_t *keys, void *out_key, u32 source_type, u32 generation) {
    u8 access_key[SE_KEY_128_SIZE] __attribute__((aligned(4)));
    generate_aes_kek(KS_AES_ECB, keys, access_key, key_area_key_sources[source_type], generation + 1);
    load_aes_key(KS_AES_ECB, out_key, access_key, aes_key_generation_source);
}

// ---------------------------------------------------------------------------
// Master key derivation.
// ---------------------------------------------------------------------------

// Erista-only: run the Atmosphère TSEC keygen firmware to populate the TSEC
// root key slots. The firmware blob is loaded from the SD card (not embedded,
// to keep the IPL uncompressed size within limits). Missing file -> skip.
static int run_ams_keygen(void) {
    u32 fw_size = 0;
    void *fw = sd_file_read("bootloader/sys/keygen.bin", &fw_size);
    if (!fw || !fw_size) {
        EPRINTF("dumpkeys: bootloader/sys/keygen.bin missing.");
        return -1;
    }

    tsec_ctxt_t tsec_ctxt = {0};
    tsec_ctxt.fw = fw;
    tsec_ctxt.size = fw_size;
    tsec_ctxt.type = TSEC_FW_TYPE_NEW;

    u32 retries = 0;
    int res = 0;
    u8 temp_key[SE_KEY_128_SIZE] __attribute__((aligned(4)));
    while ((res = tsec_query(temp_key, &tsec_ctxt)) < 0) {
        if (++retries > 15)
            break;
    }
    free(fw);
    return res < 0 ? -1 : 0;
}

static void _derive_master_keys_mariko(key_storage_t *keys) {
    // Relies on the Mariko KEK being set in slot 12 by the boot ROM.
    for (u32 i = KB_FIRMWARE_VERSION_600; i < ARRAY_SIZE(mariko_master_kek_sources) + KB_FIRMWARE_VERSION_600; i++) {
        u32 idx = i - KB_FIRMWARE_VERSION_600;
        se_aes_crypt_block_ecb(KS_MARIKO_KEK, DECRYPT, keys->master_kek[i], mariko_master_kek_sources[idx]);
        load_aes_key(KS_AES_ECB, keys->master_key[i], keys->master_kek[i], master_key_source);
    }
}

static void _derive_master_keys_from_latest_key(key_storage_t *keys) {
    minerva_periodic_training();

    if (!h_cfg.t210b01) {
        // Derive 8.1.0+ master keks from the TSEC root key (slot 13).
        for (u32 i = KB_FIRMWARE_VERSION_810 - KB_FIRMWARE_VERSION_620; i < ARRAY_SIZE(master_kek_sources); i++) {
            u32 idx = i + KB_FIRMWARE_VERSION_620;
            se_aes_crypt_block_ecb(KS_TSEC_ROOT, DECRYPT, keys->master_kek[idx], master_kek_sources[i]);
            load_aes_key(KS_AES_ECB, keys->master_key[idx], keys->master_kek[idx], master_key_source);
        }
    }

    minerva_periodic_training();

    // Walk down every lower master key from the highest one.
    for (u32 i = KB_FIRMWARE_VERSION_MAX; i > 0; i--)
        load_aes_key(KS_AES_ECB, keys->master_key[i - 1], keys->master_key[i], master_key_vectors[i]);

    load_aes_key(KS_AES_ECB, keys->temp_key, keys->master_key[0], master_key_vectors[0]);
    if (key_exists(keys->temp_key)) {
        EPRINTF("dumpkeys: unable to derive master keys.");
        memset(keys->master_key, 0, sizeof(keys->master_key));
    }
}

static int _derive_master_keys(key_storage_t *keys) {
    if (h_cfg.t210b01) {
        _derive_master_keys_mariko(keys);
        _derive_master_keys_from_latest_key(keys);
    } else {
        if (run_ams_keygen())
            return -1;
        // TSEC root key now lives in slot 13; derive everything from it.
        _derive_master_keys_from_latest_key(keys);
    }
    return 0;
}

static void _derive_non_unique_keys(key_storage_t *keys) {
    fs_derive_header_key(keys, keys->header_key);

    for (u32 gen = 0; gen < ARRAY_SIZE(keys->master_key); gen++) {
        minerva_periodic_training();
        if (!key_exists(keys->master_key[gen]))
            continue;
        for (u32 type = 0; type < ARRAY_SIZE(key_area_key_sources); type++)
            fs_derive_key_area_key(keys, keys->key_area_key[type][gen], type, gen);
        load_aes_key(KS_AES_ECB, keys->package2_key[gen], keys->master_key[gen], package2_key_source);
        load_aes_key(KS_AES_ECB, keys->titlekek[gen], keys->master_key[gen], titlekek_source);
    }
}

// ---------------------------------------------------------------------------
// prod.keys serialization.
// ---------------------------------------------------------------------------

// s_printf is not linked into the hekate IPL, so format manually.
static char *_put_hex(char *p, u8 b) {
    static const char hx[] = "0123456789abcdef";
    *p++ = hx[b >> 4];
    *p++ = hx[b & 0xF];
    return p;
}

static void _save_key(const char *name, const void *data, u32 len, char *outbuf) {
    if (!key_exists(data))
        return;
    char *p = outbuf + strlen(outbuf);
    u32 n = strlen(name);
    memcpy(p, name, n);
    p += n;
    *p++ = ' '; *p++ = '='; *p++ = ' ';
    const u8 *d = (const u8 *)data;
    for (u32 i = 0; i < len; i++)
        p = _put_hex(p, d[i]);
    *p++ = '\n';
    *p = '\0';
}

static void _save_key_family(const char *name, const void *data, u32 num_keys, u32 len, char *outbuf) {
    char temp_name[0x40];
    u32 n = strlen(name);
    memcpy(temp_name, name, n);
    temp_name[n] = '_';
    for (u32 i = 0; i < num_keys; i++) {
        char *q = _put_hex(temp_name + n + 1, (u8)i);
        *q = '\0';
        _save_key(temp_name, (const u8 *)data + i * len, len, outbuf);
    }
}

// ---------------------------------------------------------------------------
// Public entry point.
// ---------------------------------------------------------------------------

void ryazhenka_dump_keys(int mode, bool emummc) {
    if (mode == RYAZHENKA_DUMP_OFF)
        return;

    if (!sd_get_card_mounted())
        return;

    // Skip if already dumped (unless forced).
    if (mode != RYAZHENKA_DUMP_FORCE) {
        FILINFO fno;
        if (f_stat("switch/prod.keys", &fno) == FR_OK)
            return;
    }

    gfx_printf("dumpkeys: deriving keys (%s)...\n", emummc ? "emuMMC" : "sysMMC");

    key_storage_t *keys = (key_storage_t *)calloc(1, sizeof(key_storage_t));
    if (!keys)
        return;

    if (_derive_master_keys(keys)) {
        EPRINTF("dumpkeys: keygen failed, skipping.");
        free(keys);
        return;
    }
    _derive_non_unique_keys(keys);

    if (!key_exists(keys->master_key[0])) {
        EPRINTF("dumpkeys: no master keys derived, skipping.");
        free(keys);
        return;
    }

    char *text = (char *)calloc(1, SZ_16K);
    if (!text) {
        free(keys);
        return;
    }

    _save_key("header_key", keys->header_key, sizeof(keys->header_key), text);
    _save_key_family("key_area_key_application", keys->key_area_key[0], ARRAY_SIZE(keys->key_area_key[0]), SE_KEY_128_SIZE, text);
    _save_key_family("key_area_key_ocean",       keys->key_area_key[1], ARRAY_SIZE(keys->key_area_key[1]), SE_KEY_128_SIZE, text);
    _save_key_family("key_area_key_system",      keys->key_area_key[2], ARRAY_SIZE(keys->key_area_key[2]), SE_KEY_128_SIZE, text);
    _save_key_family("master_key",   keys->master_key,   ARRAY_SIZE(keys->master_key),   SE_KEY_128_SIZE, text);
    _save_key_family("package2_key", keys->package2_key, ARRAY_SIZE(keys->package2_key), SE_KEY_128_SIZE, text);
    _save_key_family("titlekek",     keys->titlekek,     ARRAY_SIZE(keys->titlekek),     SE_KEY_128_SIZE, text);

    f_mkdir("switch");
    if (sd_save_to_file(text, strlen(text), "switch/prod.keys"))
        EPRINTF("dumpkeys: failed to write prod.keys.");
    else
        gfx_printf("dumpkeys: saved switch/prod.keys\n");

    free(text);
    free(keys);
}
