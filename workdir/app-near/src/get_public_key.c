#include "get_public_key.h"
#include "os.h"
#include "ux.h"
#include "base58.h"
#include "utils.h"
#include "main.h"

#define ADDRESS_PREFIX "ed25519:"
#define ADDRESS_PREFIX_SIZE strlen(ADDRESS_PREFIX)

static char address[FULL_ADDRESS_LENGTH];

static uint32_t set_result_get_public_key() {
    memcpy(G_io_apdu_buffer, tmp_ctx.address_context.public_key, 32);
    return 32;
}

//////////////////////////////////////////////////////////////////////

UX_STEP_NOCB(
    ux_display_public_flow_5_step,
    bnnn_paging,
    {
        .title = "Public Key",
        .text = address,
    });
UX_STEP_VALID(
    ux_display_public_flow_6_step,
    pb,
    send_response(set_result_get_public_key(), true),
    {
        &C_icon_validate_14,
        "Approve",
    });
UX_STEP_VALID(
    ux_display_public_flow_7_step,
    pb,
    send_response(0, false),
    {
        &C_icon_crossmark,
        "Reject",
    });

UX_FLOW(
    ux_display_public_flow,
    &ux_display_public_flow_5_step,
    &ux_display_public_flow_6_step,
    &ux_display_public_flow_7_step);

void handle_get_public_key(uint8_t p1, uint8_t p2, const uint8_t *input_buffer, uint16_t input_length, volatile unsigned int *flags, volatile unsigned int *tx) {
    UNUSED(p2);
    UNUSED(tx);

    init_context();

    // Get the public key and return it.
    cx_ecfp_public_key_t public_key;

    uint32_t path[5];
    if (input_length < sizeof(path)) {
        THROW(INVALID_PARAMETER);
    }
    read_path_from_bytes(input_buffer, path);

    if (!get_ed25519_public_key_for_path(path, &public_key))
    {
        THROW(INVALID_PARAMETER);
    }

    memcpy(tmp_ctx.address_context.public_key, public_key.W, 32);

    memset(address, 0, sizeof(address));
    strcpy(address, ADDRESS_PREFIX);
    if (base58_encode(tmp_ctx.address_context.public_key, sizeof(tmp_ctx.address_context.public_key),
        address + ADDRESS_PREFIX_SIZE, sizeof(address) - ADDRESS_PREFIX_SIZE - 1) < 0) {
            THROW(INVALID_PARAMETER);
    }

    if (p1 == RETURN_ONLY) {
        send_response(set_result_get_public_key(), true);
    }
    else if (p1 == DISPLAY_AND_CONFIRM) {
        ux_flow_init(0, ux_display_public_flow, NULL);
        *flags |= IO_ASYNCH_REPLY;
    }
    else {
        THROW(SW_INCORRECT_P1_P2);
    }
}
