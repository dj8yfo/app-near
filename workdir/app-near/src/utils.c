#include "os.h"
#include "cx.h"
#include <stdbool.h>
#include <stdlib.h>
#include "utils.h"
#include "menu.h"
#include "context.h"

void bin_to_hex(char *out, const uint8_t *in, size_t len) {
    const unsigned char hex_digits[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    while (len--) {
        *out++ = hex_digits[(*in >> 4) & 0xF];
        *out++ = hex_digits[(*in++) & 0xF];
    }
    *out = 0;
}

void send_response(uint8_t tx, bool approve) {
    PRINTF("send_response: tmp_ctx.signing_context.buffer_used: %d\n", tmp_ctx.signing_context.buffer_used);
    G_io_apdu_buffer[tx++] = approve? 0x90 : 0x69;
    G_io_apdu_buffer[tx++] = approve? 0x00 : 0x85;
    // Send back the response, do not restart the event loop
    PRINTF("apdu out: %.*h\n", tx, G_io_apdu_buffer);
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);

    tmp_ctx.signing_context.buffer_used = 0;
    #ifdef HAVE_BAGL
    // Display back the original UX
    ui_idle();
    #endif
}
