#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

// Hardware dependent limits:
//   Ledger Nano X has 32K RAM
//   Ledger Nano S has 4K RAM
#if defined(TARGET_NANOX) || defined(TARGET_NANOS2) || defined(TARGET_STAX)

// Ledger Nano X or Nano S Plus or Stax
// !!! warning !!! replace 10000 by 650 --> overflow occurs when using 10000
// wait the correction of this PR to change https://github.com/LedgerHQ/app-near/pull/18 
#define MAX_DATA_SIZE 650

#else

// Ledger Nano S
#define MAX_DATA_SIZE 650

#endif

// Host innteration communication protocol
#define CLA 0x80                // CLASS? 
#define INS_SIGN 0x02           // Sign Instruction
#define INS_GET_PUBLIC_KEY 0x04 // Get Public Key Instruction
#define INS_GET_WALLET_ID 0x05  // Get Wallet ID
#define INS_GET_APP_CONFIGURATION 0x06 // Get App Version
#define P1_LAST 0x80            // Parameter 1 = End of Bytes to Sign (finalize)
#define P1_MORE 0x00            // Parameter 1 = More bytes coming

#define COLOR_BG_1 0xF9F9F9
#define COLOR_APP 0x0055FF
#define COLOR_APP_LIGHT 0x87dee6

#define SW_OK 0x9000
#define SW_USER_CANCELLED 0x9100
#define SW_DEVICE_IS_LOCKED 0x6986
#define SW_CONDITIONS_NOT_SATISFIED 0x6985
#define SW_BUFFER_OVERFLOW 0x6990
#define SW_PARSING_ERROR 0x6991
#define SW_INCORRECT_P1_P2 0x6A86
#define SW_INS_NOT_SUPPORTED 0x6D00
#define SW_CLA_NOT_SUPPORTED  0x6E00
#define SW_SECURITY_STATUS_NOT_SATISFIED 0x6982
#define SW_WRONG_DATA_LENGTH 0x6A87

#define SIGN_PARSING_ERROR -1
#define SIGN_FLOW_GENERIC 0
#define SIGN_FLOW_TRANSFER 1
#define SIGN_FLOW_FUNCTION_CALL 2
#define SIGN_FLOW_ADD_FUNCTION_CALL_KEY 3
#define SIGN_FLOW_ADD_FULL_ACCESS_KEY 4

#endif 