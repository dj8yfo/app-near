import hashlib
from typing import Literal, Union

from mnemonic import Mnemonic
import json

mnemo = Mnemonic("english")

DEFAULT_SPECULOS_MNEMONIC = "glory promote mansion idle axis finger extra february uncover one trip resource lawn turtle enact monster seven myth punch hobby comfort wild raise skin"

default_settings = {
    # mnemonic to use when running speculos
    "mnemonic": DEFAULT_SPECULOS_MNEMONIC,
    # path of the automation file to use for speculos if used, or None
    "automation_file": None,
    "sdk": "2.0",
    "model": "nanos"
}


def automation(filename):
    """Decorator that adds the automation_file attribute to a test function.

    When present, this filename will be used as the --automation file when creating the speculos fixture.
    """
    def decorator(func):
        func.automation_file = filename
        return func
    return decorator


def mnemonic(mnemo: str):
    """Adds the `mnemonic` setting to the test settings."""
    return test_settings({"mnemonic": mnemo})


class SpeculosGlobals:
    def __init__(self, mnemonic: str, network: str = "test"):
        if network not in ["main", "test"]:
            raise ValueError(f"Invalid network: {network}")

        self.mnemonic = mnemonic
        self.seed = mnemo.to_seed(mnemonic)

