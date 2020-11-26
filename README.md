# anarch-baremetalpi
A port of [Anarch](https://gitlab.com/drummyfish/anarch) for bare metal Raspberry Pi (with [Circle](https://github.com/rsta2/circle/)).

This is a port of the fps Anarch to raspberry pi in bare metal mode, that is, without operating system (the rpi boots straight into the game). This is possible thanks to Circle.

## Status

Actually the game is playable with keyboard. Keys are mapped to Z, X, A and the cursor. Tested on RPi 3.

Missing feature/TODO:
- no sound
- no mouse
- no save/load
- no gamepad

All these should be easy to implement (except maybe sound).

# How to build

You need an [ARM toolchain](https://github.com/rsta2/circle/#building) and a linux machine.

`PATH=/path/to/gcc-arm-none-eabi-9-2020-q2-update/bin:$PATH` put your path to the arm toolchain bin folder

`git clone https://github.com/rsta2/circle.git`

`git clone https://gitlab.com/drummyfish/anarch.git`

`nano circle/Rules.mk` Set RASPPI to your raspberry board version (like 3)

`cd circle`

`./makeall clean`

`./makeall`

`cd ../anarch-baremetalpi/`

`make`

`cd ../circle/boot`

`make`


copy all file under `circle/boot` (read the README there, expecially for RPi4) to an SD card

copy `anarch-baremetalpi/kernel*.img` to the SD card

now put the SD card into the RPi and start it.
