from typing import List
import threading
from ledgercomm import Transport

# from speculos.client import SpeculosClient

DEFAULT_KEY = "c4f5941e81e071c2fd1dae2e71fd3d859d462484391d9a90bf219211dcbb320f"
DEFAULT_PUBKEY = "ed25519:EFr6nRvgKKeteKoEH7hudt8UHYiu94Liq2yMM7x2AU9U"

CLA = 0x80
INS_SIGN = 0x02
INS_GET_PUBKEY = 0x04
INS_GET_WALLET_ID = 0x05
INS_GET_APP_CONFIGURATION = 0x06

# m/44'/397'/0'/0'/1
DERIV_PATH_DATA = bytes.fromhex('8000002c8000018d800000008000000080000001')
P1 = 0x00
P2 = 0x57

START_SCREEN_TEXT = ("Use wallet to", "view accounts")

# def test_get_wallet_id_screen(comm: SpeculosClient):
#     all_events: List[dict] = []

#     thread = threading.Thread(target=ux_thread_approve, args=[comm, all_events])
#     thread.start()
#     key = comm.apdu_exchange(CLA, INS_GET_WALLET_ID, DERIV_PATH_DATA, P1, P2)
#     thread.join()

#     # full_wallet_id = all_events[1]["text"] + all_events[3]["text"] + all_events[5]["text"] + all_events[7]["text"]

#     # assert full_wallet_id == DEFAULT_KEY.upper()
#     assert key.hex() == DEFAULT_KEY


def test_get_public_key_and_confirm_screen(transport: Transport):
    sw, key = transport.exchange(CLA, INS_GET_PUBKEY, P1, P2, None, DERIV_PATH_DATA)
    # assert full_key == DEFAULT_PUBKEY
    assert sw == 0x9000
    assert key.hex() == DEFAULT_KEY


def test_get_public_key_no_confirm(transport: Transport):
    sw, key = transport.exchange(CLA, INS_GET_PUBKEY, 1, P2, None, DERIV_PATH_DATA)
    assert sw == 0x9000
    assert key.hex() == DEFAULT_KEY

# Signature tests

def init_context(transport: Transport):
    transport.exchange(CLA, INS_GET_APP_CONFIGURATION, 0, 0, None, bytes([]))

def send_sign_command(transport: Transport, data: bytes):
    cnt = 0 
    while cnt < len(data):
        to_send = min(len(data) - cnt, 255)
        
        if to_send >= 255:
            p1 = 0x00
        else:
            p1 = 0x80

        resp = transport.exchange(CLA, INS_SIGN, p1, 0x57, None, bytes(data[0:to_send]))
        cnt += to_send
    return resp 

def generic_test_sign(transport: Transport, near_payload: bytes, expected_signature: bytes):
    """
    Generic function to tests NEAR signature mechanism 
    """
    init_context(transport)

    data = DERIV_PATH_DATA + bytes(near_payload)
    resp = send_sign_command(transport, data)

    assert resp[0] == 0x9000
    assert resp[1] == expected_signature


# Test case 0: Transfer
def test_sign_transfer(transport: Transport):
    """
    Transaction {
        signer_id: AccountId(
            "blablatest.testnet",
        ),
        public_key: ed25519:EFr6nRvgKKeteKoEH7hudt8UHYiu94Liq2yMM7x2AU9U,
        nonce: 96520360000015,
        receiver_id: AccountId(
            "speculos.testnet",
        ),
        block_hash: `C32rfeBkSMT1xnsrArkV9Mu81ww9qK7n6Kw17NhEbVuK`,
        actions: [
            Transfer(
                TransferAction {
                    deposit: 123400000000000000000000,
                },
            ),
        ],
    }
    """
    near_payload = bytes.fromhex("12000000626c61626c61746573742e746573746e657400c4f5941e81e071c2fd1dae2e71fd3d859d462484391d9a90bf219211dcbb320f0f7ac5e5c85700001000000073706563756c6f732e746573746e6574a3f5d1167a5c605fed71fc78d4381bef47a5acb3aba6fc9c07d7b8b912fc1e2a010000000300002083c7f60387211a000000000000")
    expected_signature = bytes.fromhex("e22eea0ee27a2d8e0bdfc72fb6337492d10a78aec15ff3cb6126b2944af920863e0907d5462bf2822a6bb0a62f1bb594e899ac96db7e95386895e91f325c460c")
    generic_test_sign(transport, near_payload, expected_signature)


# Test case 1: Function call
def test_sign_function_call(transport: Transport):
    """
    Transaction {
        signer_id: AccountId(
            "blablatest.testnet",
        ),
        public_key: ed25519:EFr6nRvgKKeteKoEH7hudt8UHYiu94Liq2yMM7x2AU9U,
        nonce: 96520360000015,
        receiver_id: AccountId(
            "speculos.testnet",
        ),
        block_hash: `C32rfeBkSMT1xnsrArkV9Mu81ww9qK7n6Kw17NhEbVuK`,
        actions: [
            FunctionCall(
                FunctionCallAction {
                    method_name: function_name,
                    args: `Dza`,
                    gas: 9999,
                    deposit: 1122334455,
                },
            ),
        ],
    }
    """

    near_payload = bytes.fromhex("12000000626c61626c61746573742e746573746e657400c4f5941e81e071c2fd1dae2e71fd3d859d462484391d9a90bf219211dcbb320f0f7ac5e5c85700001000000073706563756c6f732e746573746e6574a3f5d1167a5c605fed71fc78d4381bef47a5acb3aba6fc9c07d7b8b912fc1e2a01000000020d00000066756e6374696f6e5f6e616d6502000000aabb0f27000000000000f776e542000000000000000000000000")
    expected_signature = bytes.fromhex("e3329e9411101f0d0556a7106dc55ae10c04e234761e77ef9d78a53652ee8ed3f43c13761e0ea2edd4de21a534930932addb9d3e996b434aa47a3696f1ac2200")
    generic_test_sign(transport, near_payload, expected_signature)


# Test case 2: Stake
def test_sign_stake(transport: Transport):
    """
    Transaction {
        signer_id: AccountId(
            "signer.near",
        ),
        public_key: ed25519:4c2pNM4aqrdTgaeRQyJnP9UwAFvHstDzZ1SCQAB7HnEc,
        nonce: 96520360000015,
        receiver_id: AccountId(
            "receiver.near",
        ),
        block_hash: `C32rfeBkSMT1xnsrArkV9Mu81ww9qK7n6Kw17NhEbVuK`,
        actions: [
            Stake(
                StakeAction {
                    stake: 55556666,
                    public_key: ed25519:J6DmXMFwt894ZXED1BCGiK3y1aRhophVob5VwL8JBTm1,
                },
            ),
        ],
    }
    """
    near_payload = bytes.fromhex("0b0000007369676e65722e6e65617200358c7177d702ee102a3cae18aa84b005bbd03b9188d5312e7d6df8f78d2a6a490f7ac5e5c85700000d00000072656365697665722e6e656172a3f5d1167a5c605fed71fc78d4381bef47a5acb3aba6fc9c07d7b8b912fc1e2a01000000043aba4f0300000000000000000000000000fded04a996ebf5e25e7d6dd4c82edbbb544a397517edea03eadb39fb5211e460")
    expected_signature = bytes.fromhex("e9a74d73b80de013ca90a2794855fa74d35c33697597905e03a3e1a483f68e5ed8e23dacd0b9ff62b881d7b0543c5dfb453fc2bd4e5dd16c8103e6ec72e3bc07")
    generic_test_sign(transport, near_payload, expected_signature)


# Test case 3: Add key
def test_sign_add_key(transport: Transport):
    """
    Transaction {
        signer_id: AccountId(
            "arthur",
        ),
        public_key: ed25519:JCuJVU1tbr2tmYGX8b6f3YpvuN2zBZd2MZAYh16cNqGr,
        nonce: 96520360000015,
        receiver_id: AccountId(
            "98793cd91a3f870fb126f66285808c7e094afcfc4eda8a970f6648cdf0dbd6de",
        ),
        block_hash: `C32rfeBkSMT1xnsrArkV9Mu81ww9qK7n6Kw17NhEbVuK`,
        actions: [
            AddKey(
                AddKeyAction {
                    public_key: ed25519:79QAhRR464JQGr5ZXZe6jvXGM8NNgnR5KzRoQhaJyTYT,
                    access_key: AccessKey {
                        nonce: 12345,
                        permission: FullAccess,
                    },
                },
            ),
        ],
    }
    """
    near_payload = bytes.fromhex("060000006172746875720053f9afa67ef91539ff38e2b36bbbed2d1dce6e18d06337cf6647389b5477359b0f7ac5e5c85700004000000039383739336364393161336638373066623132366636363238353830386337653039346166636663346564613861393730663636343863646630646264366465a3f5d1167a5c605fed71fc78d4381bef47a5acb3aba6fc9c07d7b8b912fc1e2a0100000005002ffe256fd9a6e815abc3f220163413ac62871ecc5875d87625a35ce7ea65ee2f393000000000000001")
    expected_signature = bytes.fromhex("003ee4ba2a305db02da9d929c95010e045f1a838dd9550c8e433398f6c8f6f2d71064ccc2b856a0949e2e9d678511a13679b35e787f47b28b68557969ee40b0a")
    generic_test_sign(transport, near_payload, expected_signature)

# Test case 4: Delete key
def test_sign_delete_key(transport: Transport):
    """
    Transaction {
        signer_id: AccountId(
            "speculosaccount",
        ),
        public_key: ed25519:JCuJVU1tbr2tmYGX8b6f3YpvuN2zBZd2MZAYh16cNqGr,
        nonce: 96520360000015,
        receiver_id: AccountId(
            "98793cd91a3f870fb126f66285808c7e094afcfc4eda8a970f6648cdf0dbd6de",
        ),
        block_hash: `C32rfeBkSMT1xnsrArkV9Mu81ww9qK7n6Kw17NhEbVuK`,
        actions: [
            DeleteKey(
                DeleteKeyAction {
                    public_key: ed25519:79QAhRR464JQGr5ZXZe6jvXGM8NNgnR5KzRoQhaJyTYT,
                },
            ),
        ],
    }
    """
    near_payload = bytes.fromhex("0f00000073706563756c6f736163636f756e7400ffa334478481a4a779c54ee30912f37ac23a323261f431f89d2652c277ca51ef0f7ac5e5c85700004000000039383739336364393161336638373066623132366636363238353830386337653039346166636663346564613861393730663636343863646630646264366465a3f5d1167a5c605fed71fc78d4381bef47a5acb3aba6fc9c07d7b8b912fc1e2a0100000006005b4cf697ce3c6ded94c7adfa3c2d8310cc1dda88828a238b48513df3fdec7ab8")
    expected_signature = bytes.fromhex("8c6d615c8b4afc0bfeaab1eab643d5b12ea0d8e017636b37b59433786c11fdd29b91432af4acdb116b88211e4bf4fdb0baaa50bc0243d47dcfed9d5121a98e04")
    generic_test_sign(transport, near_payload, expected_signature)

# Test case 4: Delete account
def test_sign_delete_account(transport: Transport):
    """
    Transaction {
        signer_id: AccountId(
            "speculosaccount",
        ),
        public_key: ed25519:7aE719urLcxUn81B9RkvXwDTgnXM7DEAy1eGWU2nDNd9,
        nonce: 96520360000015,
        receiver_id: AccountId(
            "receiver.near",
        ),
        block_hash: `C32rfeBkSMT1xnsrArkV9Mu81ww9qK7n6Kw17NhEbVuK`,
        actions: [
            DeleteAccount(
                DeleteAccountAction {
                    beneficiary_id: AccountId(
                        "beneficiaryid",
                    ),
                },
            ),
        ],
    }
    """
    near_payload = bytes.fromhex("0f00000073706563756c6f736163636f756e740061a91abba0099d3ef23923645b37f19e6ebfeb220b238ee9abef3eeb32f851b40f7ac5e5c85700000d00000072656365697665722e6e656172a3f5d1167a5c605fed71fc78d4381bef47a5acb3aba6fc9c07d7b8b912fc1e2a01000000070d00000062656e65666963696172796964")
    expected_signature = bytes.fromhex("f518bcebfc57c3bb10d07511a5819f0f048868657661fb84a05f09297d3cadeb9e6e5995addbb6f9cd6c28a31474306adf0a41914a83161f380cd8e6a0a1a705")
    generic_test_sign(transport, near_payload, expected_signature)