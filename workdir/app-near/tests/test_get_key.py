from hashlib import sha256
import json
from pathlib import Path
from typing import Tuple, List, Dict, Any
import pytest

from utils import automation

default_key = "c4f5941e81e071c2fd1dae2e71fd3d859d462484391d9a90bf219211dcbb320f"

def test_get_public_key_no_confirm(transport):
    apdu = "80040157148000002c8000018d800000008000000080000001"
    sw, key = transport.exchange_raw(apdu)
    assert sw == 0x9000
    assert key.hex() == default_key


@automation("automations/accept.json")
def test_get_wallet_id(transport):
    apdu = "80050057148000002c8000018d800000008000000080000001"
    __, key = transport.exchange_raw(apdu)
    assert key.hex() == default_key


@automation("automations/accept.json")
def test_get_public_key_and_confirm(transport):
    apdu = "80040057148000002c8000018d800000008000000080000001"
    _, key = transport.exchange_raw(apdu)
    assert key.hex() == default_key

