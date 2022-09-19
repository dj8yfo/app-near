from mnemonic import Mnemonic

DEFAULT_SPECULOS_MNEMONIC = "glory promote mansion idle axis finger extra february uncover one " \
                            "trip resource lawn turtle enact monster seven myth punch hobby " \
                            "comfort wild raise skin"

DEFAULT_SETTINGS = {
    # mnemonic to use when running speculos
    "mnemonic": DEFAULT_SPECULOS_MNEMONIC,
    # path of the automation file to use for speculos if used, or None
    "automation_file": None,
    "sdk": "2.0",
    "model": "nanos"
}


def automation(filename):
    """Decorator that adds the automation_file attribute to a test function.

    When present, this filename will be used as the --automation file when creating the
    Speculos fixture.
    """
    def decorator(func):
        func.automation_file = filename
        return func
    return decorator


class SpeculosGlobals:
    def __init__(self, mnemonic: str, network: str = "test"):
        if network not in ["main", "test"]:
            raise ValueError(f"Invalid network: {network}")

        self.mnemonic = mnemonic
        self.seed = Mnemonic("english").to_seed(mnemonic)
