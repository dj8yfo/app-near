#include "ledger_crypto.h"
#include <string.h>
#include "crypto_helpers.h"

#include "os.h"
#include "cx.h"

// converts little endian 65 byte (0x4 32X 32Y) public key to 32 byte Y big endian form (for other applications)
static void public_key_le_to_be(uint8_t in_raw_public_key[static 65], uint8_t out_public_key[static 32]) {
    // copy public key little endian to big endian
    for (uint8_t i = 0; i < 32; i++) {
        out_public_key[i] = in_raw_public_key[64 - i];
    }
    // set sign bit
    if ((in_raw_public_key[32] & 1) != 0) {
        out_public_key[31] |= 0x80;
    }
}

// Get a public key from the 44'/397' keypath.
bool get_ed25519_public_key_for_path(const uint32_t* path, uint8_t public_key[static 32]) {
    uint8_t raw_public_key[65];
    if (bip32_derive_with_seed_get_pubkey_256(HDW_ED25519_SLIP10, CX_CURVE_Ed25519, path, 5, raw_public_key, NULL, CX_SHA512, NULL, 0)) {
        return false;
    }
    public_key_le_to_be(raw_public_key, public_key);
    return true;
}