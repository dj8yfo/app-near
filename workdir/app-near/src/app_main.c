/*******************************************************************************
*   Waves platform Wallet App for Nano Ledger S. Updated By Waves community.
*   Copyright (c) 2017-2018 Sergey Tolmachev (Tolsi) <tolsi.ru@gmail.com>
* 
*   Based on Sample code provided and (c) 2016 Ledger and 2017-2018 Jake B. (Burstcoin)
*
*   Based on Ledger app boilerplate (c) 2016 Ledger
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

#include "utils.h"
#include "ui.h"
#include "get_public_key.h"
#include "get_wallet_id.h"
#include "sign_transaction.h"
#include "menu.h"
#include "main.h"
#include "near.h"
#include "crypto/ledger_crypto.h"
#include "dispatcher.h"
#include "constants.h"
#include "io.h"

// Temporary area to sore stuff and reuse the same memory
tmpContext_t tmp_ctx;
uiContext_t ui_context;

uint32_t deserialize_uint32_t(const uint8_t *buffer)
{
    uint32_t value = 0;

    value |= buffer[0] << 24;
    value |= buffer[1] << 16;
    value |= buffer[2] << 8;
    value |= buffer[3];
    return value;
}

// 20 bytes total
void read_path_from_bytes(const uint8_t *buffer, uint32_t *path) {
    path[0] = deserialize_uint32_t(buffer);
    path[1] = deserialize_uint32_t(buffer + 4);
    path[2] = deserialize_uint32_t(buffer + 8);
    path[3] = deserialize_uint32_t(buffer + 12);
    path[4] = deserialize_uint32_t(buffer + 16);
}

// like https://github.com/lenondupe/ledger-app-stellar/blob/master/src/main.c#L1784
uint32_t set_result_sign() {
    cx_ecfp_private_key_t private_key;
    get_private_key_for_path((uint32_t *) tmp_ctx.signing_context.bip32, &private_key);

    BEGIN_TRY {
        TRY {
            uint8_t signature[64];
            near_message_sign(&private_key, (unsigned char *)tmp_ctx.signing_context.buffer, tmp_ctx.signing_context.buffer_used, signature);

            memcpy(G_io_apdu_buffer, signature, sizeof(signature));
        } FINALLY {
            // reset all private stuff
            explicit_bzero(&private_key, sizeof(cx_ecfp_private_key_t));
        }
    }
    END_TRY;

    return 64;
}

#define OFFSET_CLA 0
#define OFFSET_INS 1
#define OFFSET_P1 2
#define OFFSET_P2 3
#define OFFSET_LC 4
#define OFFSET_CDATA 5

// Called by both the U2F and the standard communications channel
void handle_apdu(volatile unsigned int *flags, volatile unsigned int *tx, volatile unsigned int rx) {
    unsigned short sw = 0;

    BEGIN_TRY {
        TRY {
            if (G_io_apdu_buffer[OFFSET_CLA] != CLA) {
                THROW(SW_CLA_NOT_SUPPORTED);
            }

            PRINTF("command: %d\n", G_io_apdu_buffer[OFFSET_INS]);
            switch (G_io_apdu_buffer[OFFSET_INS]) {
            case INS_SIGN: {
                if (G_io_apdu_buffer[OFFSET_LC] != rx - 5) {
                    // the length of the APDU should match what's in the 5-byte header.
                    // If not fail.  Don't want to buffer overrun or anything.
                    THROW(SW_CONDITIONS_NOT_SATISFIED);
                }

                handle_sign_transaction(G_io_apdu_buffer[OFFSET_P1], G_io_apdu_buffer[OFFSET_P2], G_io_apdu_buffer + OFFSET_CDATA, G_io_apdu_buffer[OFFSET_LC], flags, tx);
            } break;

            case INS_GET_PUBLIC_KEY: {
                if (G_io_apdu_buffer[OFFSET_LC] != rx - 5 || G_io_apdu_buffer[OFFSET_LC] != 20) {
                    // the length of the APDU should match what's in the 5-byte header.
                    // If not fail.  Don't want to buffer overrun or anything.
                    THROW(SW_CONDITIONS_NOT_SATISFIED);
                }

                handle_get_public_key(G_io_apdu_buffer[OFFSET_P1], G_io_apdu_buffer[OFFSET_P2], G_io_apdu_buffer + OFFSET_CDATA, G_io_apdu_buffer[OFFSET_LC], flags, tx);
            } break;

            case INS_GET_WALLET_ID: {
                if (G_io_apdu_buffer[OFFSET_LC] != rx - 5 || G_io_apdu_buffer[OFFSET_LC] != 20) {
                    // the length of the APDU should match what's in the 5-byte header.
                    // If not fail.  Don't want to buffer overrun or anything.
                    THROW(SW_CONDITIONS_NOT_SATISFIED);
                }

                handle_get_wallet_id(G_io_apdu_buffer[OFFSET_P1], G_io_apdu_buffer[OFFSET_P2], G_io_apdu_buffer + OFFSET_CDATA, G_io_apdu_buffer[OFFSET_LC], flags, tx);
            } break;

            case INS_GET_APP_CONFIGURATION:
                // NOTE: This allows using INS_GET_APP_CONFIGURATION as "reset state" command
                init_context();

                G_io_apdu_buffer[0] = MAJOR_VERSION;
                G_io_apdu_buffer[1] = MINOR_VERSION;
                G_io_apdu_buffer[2] = PATCH_VERSION;
                *tx = 3;
                THROW(SW_OK);
                break;

            default:
                THROW(0x6D00);
                break;
            }
        }
        CATCH(EXCEPTION_IO_RESET) {
            THROW(EXCEPTION_IO_RESET);
        }
        CATCH_OTHER(e) {
        switch (e & 0xF000) {
            case 0x6000:
                sw = e;
                break;
            case 0x9000:
                // All is well
                sw = e;
                break;
            default:
                // Internal error
                sw = 0x6800 | (e & 0x7FF);
                break;
            }
            // Unexpected exception => report
            G_io_apdu_buffer[*tx] = sw >> 8;
            G_io_apdu_buffer[*tx + 1] = sw;
            *tx += 2;
        }
        FINALLY {
        }
    END_TRY;
    }
}

void init_context() {
    memset(&tmp_ctx, 0, sizeof(tmp_ctx));
}

void app_main(void) {
    volatile unsigned int rx = 0;
    volatile unsigned int tx = 0;
    volatile unsigned int flags = 0;

    // DESIGN NOTE: the bootloader ignores the way APDU are fetched. The only
    // goal is to retrieve APDU.
    // When APDU are to be fetched from multiple IOs, like NFC+USB+BLE, make
    // sure the io_event is called with a
    // switch event, before the apdu is replied to the bootloader. This avoid
    // APDU injection faults.
    for (;;) {
        volatile unsigned short sw = 0;
        BEGIN_TRY {
            TRY {
                rx = tx;
                tx = 0; // ensure no race in catch_other if io_exchange throws
                        // an error
                rx = io_exchange(CHANNEL_APDU | flags, rx);
                flags = 0;

                // no apdu received, well, reset the session, and reset the
                // bootloader configuration
                if (rx == 0) {
                    THROW(SW_SECURITY_STATUS_NOT_SATISFIED);
                }

                PRINTF("New APDU received:\n%.*H\n", rx, G_io_apdu_buffer);
                handle_apdu(&flags, &tx, rx);
            }
            CATCH(EXCEPTION_IO_RESET) {
              THROW(EXCEPTION_IO_RESET);
            }
            CATCH_OTHER(e) {
                switch (e & 0xF000) {
                    case 0x6000:
                        sw = e;
                        break;
                    case 0x9000:
                        // All is well
                        sw = e;
                        break;
                    default:
                        // Internal error
                        sw = 0x6800 | (e & 0x7FF);
                        break;
                }
                if (e != 0x9000) {
                    flags &= ~IO_ASYNCH_REPLY;
                }
                // Unexpected exception => report
                G_io_apdu_buffer[tx] = sw >> 8;
                G_io_apdu_buffer[tx + 1] = sw;
                tx += 2;
            }
            FINALLY {
            }
        }
        END_TRY;
    }

//return_to_dashboard:
    return;
}
