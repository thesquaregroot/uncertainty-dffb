# uncertainty-dffb

A distorted fixed filter bank (DFFB) for the Eurorack module
[Uncertainty](https://github.com/oamodular/uncertainty)
by [Olivia Artz Modular](https://github.com/oamodular).

## About this firmware

This firmware passes the input signal through a bank of eight filters with
fixed frequency bands (elliptic IIR filters).  The outputs of those
filters are then distorted into square waves and sent to each of the eight
outputs of the module.

Sampling is done at a rate of 40kHz, and the filter pass bands are as follows:

Output | Start | End
--- | --- | ---
1 | (lowpass) | 60 Hz
2 | 80 Hz | 160 Hz
3 | 200 Hz | 400 Hz
4 | 450 Hz | 900 Hz
5 | 1 kHz | 2 kHz
6 | 2.3 kHz | 4.6 kHz
7 | 5 kHz | 10 kHz
8 | 11 kHz | (hipass)

The outputs will go high (+5V) when the filter output signal goes above .1V,
otherwise the signal will be low (0V).  The threshold of .1V (versus 0V) allows
for outputting silence when the signal level is very low.

## Installing

If you want to install this firmware as-is, you can download the .uf2 file and
follow the instructions [here](https://github.com/oamodular/uncertainty?tab=readme-ov-file#how-to-reinstall-the-default-firmware).

If you would like to modify the code and run it, you can do so using the Arduino
IDE, following the instructions
[here](https://wiki.seeedstudio.com/XIAO-RP2040-with-Arduino/).
