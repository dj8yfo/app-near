#include "get_wallet_id.h"
#include "os.h"
#include "ux.h"
#include "utils.h"
#include "main.h"

static char wallet_id[65];

static uint32_t set_result_get_public_key() {
    memcpy(G_io_apdu_buffer, tmp_ctx.address_context.public_key, 32);
    return 32;
}

//////////////////////////////////////////////////////////////////////

UX_STEP_NOCB(
    ux_display_wallet_flow_id_step,
    bnnn_paging,
    {
        .title = "Wallet Id",
        .text = wallet_id,
    });
UX_STEP_VALID(
    ux_display_wallet_flow_accept_step,
    pb,
    send_response(set_result_get_public_key(), true),
    {
        &C_icon_validate_14,
        "Approve",
    });
UX_STEP_VALID(
    ux_display_wallet_flow_reject_step,
    pb,
    send_response(0, false),
    {
        &C_icon_crossmark,
        "Reject",
    });

UX_FLOW(
    ux_display_wallet_id_flow,
    &ux_display_wallet_flow_id_step,
    &ux_display_wallet_flow_accept_step,
    &ux_display_wallet_flow_reject_step);

void handle_get_wallet_id(uint8_t p1, uint8_t p2, const uint8_t *input_buffer, uint16_t input_length, volatile unsigned int *flags, volatile unsigned int *tx) {
    UNUSED(p1);
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

    bin_to_hex(wallet_id, public_key.W, 32);

    ux_flow_init(0, ux_display_wallet_id_flow, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
