# NEAR Ledger app

## Overview
This is Nano S/SP/X and Stax app for NEAR protocol.

## Building and installing
To build and install the app on your Ledger Nano S you must set up the Ledger Nano S build environments. Please follow the Getting Started instructions at [here](https://ledger.readthedocs.io/en/latest/userspace/getting_started.html).

If you don't want to setup a global environnment, you can also setup one just for this app by sourcing `prepare-devenv.sh` with the right target (`s` or `x`).

install prerequisite and switch to a Nano X dev-env:

```bash
sudo apt install python3-venv python3-dev libudev-dev libusb-1.0-0-dev

# (x or s, depending on your device)
source prepare-devenv.sh x 
```

Compile and load the app onto the device:
```bash
make load
```

Refresh the repo (required after Makefile edits):
```bash
make clean
```

Remove the app from the device:
```bash
make delete
```


## Example of Ledger wallet functionality

Test functionality:
```bash
# (x or s, depending on your device)
source prepare-devenv.sh x
python test_example.py --account_number 12345
```

## Documentation
This follows the specification available in the [`api.asc`](https://github.com/LedgerHQ/app-near/blob/master/workdir/app-near/doc/api.asc).
