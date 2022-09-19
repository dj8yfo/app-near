import subprocess
import os
import socket
import time
import logging
import pytest
from typing import Union
from pathlib import Path
import re
import json

from ledgercomm import Transport
from speculos.client import SpeculosClient
from utils import default_settings


logging.basicConfig(level=logging.INFO)
# root of the repository
repo_root_path: Path = Path(__file__).parent.parent
ASSIGNMENT_RE = re.compile(
    r'^\s*([a-zA-Z_][a-zA-Z_0-9]*)\s*=\s*(.*)$', re.MULTILINE)


def pytest_addoption(parser):
    parser.addoption("--hid",
                     action="store_true")
    parser.addoption("--headless", action="store_true")
    parser.addoption("--model", action="store", default="nanos")
    parser.addoption("--sdk", action="store", default="2.1")

def get_app_version() -> str:
    makefile_path = repo_root_path / "Makefile"
    if not makefile_path.is_file():
        raise FileNotFoundError(f"Can't find file: '{makefile_path}'")

    makefile: str = makefile_path.read_text()

    assignments = {
        identifier: value for identifier, value in ASSIGNMENT_RE.findall(makefile)
    }

    return f"{assignments['APPVERSION_M']}.{assignments['APPVERSION_N']}.{assignments['APPVERSION_P']}"


@pytest.fixture(scope="module")
def app_version() -> str:
    return get_app_version()


@pytest.fixture
def sdk(pytestconfig):
    return pytestconfig.getoption("sdk")

@pytest.fixture
def hid(pytestconfig):
    return pytestconfig.getoption("hid")

@pytest.fixture
def device(request, hid):
    # If running on real hardware, nothing to do here
    if hid:
        yield
        return

    # Gets the speculos executable from the SPECULOS environment variable,
    # or hopes that "speculos.py" is in the $PATH if not set
    speculos_executable = os.environ.get("SPECULOS", "speculos.py")

    base_args = [
        speculos_executable, "./NEAR-bin/app.elf",
        "--sdk", "2.0",
        "--display", "headless"
    ]

    # Look for the automation_file attribute in the test function, if present
    try:
        automation_args = ["--automation", f"file:{request.function.automation_file}"]
    except AttributeError:
        automation_args = []

    speculos_proc = subprocess.Popen([*base_args, *automation_args])


    # Attempts to connect to speculos to make sure that it's ready when the test starts
    for _ in range(100):
        try:
            socket.create_connection(("127.0.0.1", 9999), timeout=1.0)
            connected = True
            break
        except ConnectionRefusedError:
            time.sleep(0.1)
            connected = False

    if not connected:
        raise RuntimeError("Unable to connect to speculos.")

    yield

    speculos_proc.terminate()
    speculos_proc.wait()

@pytest.fixture
def settings(request) -> dict:
    try:
        return request.function.test_settings
    except AttributeError:
        return default_settings.copy()


@pytest.fixture
def transport(device, hid):
    transport = (Transport(interface="hid", debug=True)
                 if hid else Transport(interface="tcp",
                                       server="127.0.0.1",
                                       port=9999,
                                       debug=True))
    yield transport
    transport.close()

@pytest.fixture(scope='session', autouse=True)
def root_directory(request):
    return Path(str(request.config.rootdir))

@pytest.fixture
def hid(pytestconfig):
    return pytestconfig.getoption("hid")


@pytest.fixture
def headless(pytestconfig):
    return pytestconfig.getoption("headless")


@pytest.fixture
def enable_slow_tests(pytestconfig):
    return pytestconfig.getoption("enableslowtests")


@pytest.fixture
def model(pytestconfig):
    return pytestconfig.getoption("model")


@pytest.fixture
def comm(settings, root_directory, hid, headless, model, sdk, app_version: str) -> Union[Transport, SpeculosClient]:
    if hid:
        client = TransportClient("hid")
    else:
        # We set the app's name before running speculos in order to emulate the expected
        # behavior of the SDK's GET_VERSION default APDU.

        if not os.getenv("SPECULOS_APPNAME"):
            os.environ['SPECULOS_APPNAME'] = f'app:{app_version}'

        app_binary = os.getenv("BINARY", str(
            repo_root_path.joinpath("NEAR-bin/app.elf")))

        client = SpeculosClient(
            app_binary,
            ['--model', model, '--sdk', sdk, '--seed', f'{settings["mnemonic"]}']
            + ["--display", "qt" if not headless else "headless"]
        )
        client.start()

        if settings["automation_file"]:
            automation_file = root_directory.joinpath(
                settings["automation_file"])
            with open(automation_file) as f:
                rules = json.load(f)
            client.set_automation_rules(rules)

    yield client

    client.stop()


