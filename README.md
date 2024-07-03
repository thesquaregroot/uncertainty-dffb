# uncertainty-dffb

A distorted fixed filter bank (DFFB) for the Eurorack module
[Uncertainty](https://github.com/oamodular/uncertainty)
by [Olivia Artz Modular](https://github.com/oamodular).

## About this firmware

This firmware passes the input signal through a bank of eight filters with
fixed frequency bands (elliptic IIR filters).  The outputs of those
filters are then distorted into square waves and sent to each of the eight
outputs of the module.

The outputs will go high (+5V) when the filter output signal goes above 0.5V,
otherwise the signal will be low (0V).  This captures the essence of the filter
output while also going silent when the filter output is low.

Sampling is done at a rate of 10kHz, and the filters are designed for the
following bands:

Output | Start | End
--- | --- | ---
1 | (low pass) | 40 Hz
2 | 50 Hz | 100 Hz
3 | 100 Hz | 200 Hz
4 | 200 Hz | 400 Hz
5 | 400 Hz | 800 Hz
6 | 800 Hz | 1600 Hz
7 | 1600 Hz | 3200 Hz
8 | 3600 Hz | (high pass)

Note that due to the low sample rate, the highest detectable frequency will be 5
kHz.  Frequencies above 5kHz will be aliased into the detected frequency range.
This may be mitigated some by the distortion applied on output, but could also
be limited by applying a low pass filter to the signal before sending
it to the module.  It could also be intentionally abused for sonic mayhem.

The first output will capture LFO rate signals and very low audio.  The last
output will capture the highest available frequencies and will likely have the
most aliasing present.  The other six outputs each span an octave and
will overlap slightly with each of the adjacent filters.

For the Octave/Matlab code used to design the filters see
[filter_design.m](filter_design.m).

## Installing

If you want to install this firmware as-is, you can download the .uf2 file and
follow the instructions [here](https://github.com/oamodular/uncertainty?tab=readme-ov-file#how-to-reinstall-the-default-firmware).

If you would like to modify the code and run it, you can do so using the Arduino
IDE, following the instructions
[here](https://wiki.seeedstudio.com/XIAO-RP2040-with-Arduino/).
