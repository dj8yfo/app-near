from hashlib import sha256
import json
from pathlib import Path
from typing import Tuple, List, Dict, Any
import pytest
import threading

from speculos.client import SpeculosClient

from utils import automation

default_key = "c4f5941e81e071c2fd1dae2e71fd3d859d462484391d9a90bf219211dcbb320f"
default_pubkey = "ed25519:EFr6nRvgKKeteKoEH7hudt8UHYiu94Liq2yMM7x2AU9U"

CLA = 0x80
INS_GET_PUBKEY = 0x04
INS_GET_WALLET_ID = 0x05
DERIV_PATH_DATA = bytes.fromhex('8000002c8000018d800000008000000080000001')
P1 = 0x00
P2 = 0x57

START_SCREEN_TEXT = ("Use wallet to", "view accounts")


def ux_thread_approve(comm: SpeculosClient, all_events: List[dict]):
    """Completes the validation flow always going right and accepting at the appropriate time, while collecting all the events in all_events."""

    # Skip start screen
    event = comm.get_next_event()
    while event["text"] in START_SCREEN_TEXT:
        event = comm.get_next_event()

    while (event["text"] != "Approve"):
        all_events.append(event)
        comm.press_and_release("right")
        event = comm.get_next_event()

    comm.press_and_release("left")
    comm.press_and_release("both")


def test_get_wallet_id_screen(comm: SpeculosClient):
    all_events: List[dict] = []

    x = threading.Thread(target=ux_thread_approve, args=[comm, all_events])
    x.start()
    key = comm.apdu_exchange(CLA, INS_GET_WALLET_ID, DERIV_PATH_DATA, P1, P2)
    x.join()

    full_wallet_id = all_events[1]["text"] + all_events[3]["text"] + all_events[5]["text"] + all_events[7]["text"]

    assert full_wallet_id == default_key.upper()
    assert key.hex() == default_key


def test_get_public_key_and_confirm_screen(comm: SpeculosClient):
    all_events: List[dict] = []

    x = threading.Thread(target=ux_thread_approve, args=[comm, all_events])
    x.start()
    key = comm.apdu_exchange(CLA, INS_GET_PUBKEY, DERIV_PATH_DATA, P1, P2)
    x.join()

    full_key = all_events[1]["text"] + all_events[3]["text"] + all_events[5]["text"]

    assert full_key == default_pubkey
    assert key.hex() == default_key


def test_get_public_key_no_confirm(transport):
    P1 = 0x01
    sw, key = transport.exchange(CLA, INS_GET_PUBKEY, P1, P2, None, DERIV_PATH_DATA )
    assert sw == 0x9000
    assert key.hex() == default_key


