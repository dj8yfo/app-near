import pytest
from ragger import Firmware
from ragger.backend import SpeculosBackend, LedgerCommBackend

# This variable is needed for Speculos only (physical tests need the application to be already installed)
APPLICATION = "bin/app.elf"
# This variable will be useful in tests to implement different behavior depending on the firmware
NANOS_FIRMWARE = Firmware("nanosp", "1.0")

# adding a pytest CLI option "--backend"
def pytest_addoption(parser):
    print(help(parser.addoption))
    parser.addoption("--backend", action="store", default="speculos")
    parser.addoption("--display", action="store_true", default=False)

# accessing the value of the "--backend" option as a fixture
@pytest.fixture(scope="session")
def backend(pytestconfig):
    return pytestconfig.getoption("backend")

@pytest.fixture(scope="session")
def display(pytestconfig):
    return pytestconfig.getoption("display")

# Providing the firmware as a fixture
# This can be easily parametrized, which would allow to run the tests on several firmware type or version
@pytest.fixture
def firmware():
    return NANOS_FIRMWARE

def prepare_speculos_args(firmware: Firmware, display: bool):
    speculos_args = []

    if display:
        speculos_args += ["--display", "qt"]

    # app_path = app_path_from_app_name(APPS_DIRECTORY, APP_NAME, firmware.device)
    app_path = "bin/app.elf"

    return ([app_path], {"args": speculos_args})


# Depending on the "--backend" option value, a different backend is
# instantiated, and the tests will either run on Speculos or on a physical
# device depending on the backend
def create_backend(backend: str, firmware: Firmware, display: bool):
    if backend.lower() == "ledgercomm":
        return LedgerCommBackend(firmware, interface="hid")
    elif backend.lower() == "ledgerwallet":
        return LedgerWalletBackend(firmware)
    elif backend.lower() == "speculos":
        args, kwargs = prepare_speculos_args(firmware, display)
        return SpeculosBackend(*args, firmware, **kwargs)
    else:
        raise ValueError(f"Backend '{backend}' is unknown. Valid backends are: {BACKENDS}")

# This final fixture will return the properly configured backend client, to be used in tests
@pytest.fixture
def client(backend, firmware, display):
    with create_backend(backend, firmware, display) as b:
        yield b