# uncertainty-dffb

A distorted fixed filter bank (DFFB) for the Eurorack module
[Uncertainty](https://oamodular.org/products/uncertainty) by
[Olivia Artz Modular](https://github.com/oamodular).

## About this firmware

The input signal through a bank of eight digital filters with fixed frequency
bands (elliptic IIR filters).  The outputs of those filters are then distorted
into square waves and sent to each of the eight outputs of the module.

The outputs will go high (+5V) when the filtered signal goes above 0.5V,
otherwise the signal will be low (0V).  This captures the essence of the
filtered signal while also emitting silence when the signal level is low.

Sampling is done at a rate of 10kHz, and the filters are designed for the
following bands:

Output | Start | End
--- | --- | ---
1 | (low-pass) | 40 Hz
2 | 50 Hz | 100 Hz
3 | 100 Hz | 200 Hz
4 | 200 Hz | 400 Hz
5 | 400 Hz | 800 Hz
6 | 800 Hz | 1600 Hz
7 | 1600 Hz | 3200 Hz
8 | 3600 Hz | (high-pass)

Note that due to the low sample rate, the highest detectable frequency will be 5
kHz.  Frequencies above 5kHz will be aliased into the detected frequency range.
This may be mitigated some by the distortion applied on output, but could also
be limited by applying a low-pass filter to the signal before sending
it to the module.  It could also be intentionally abused for sonic mayhem.

The first output will capture LFO rate signals and very low audio.  The last
output will capture the highest available frequencies and will likely have the
most aliasing present.  The other six outputs each span an octave and
will overlap slightly with each of the adjacent filters.

For the Octave/Matlab code used to design the filters see
[filter_design.m](filter_design.m).

## Use Cases

### Eight Flavors of Distortion

Patch a low-pitched sawtooth wave (or other signal with a lot of harmonics)
into the input.  Many, if not all, of the LEDs will light up to varying
degrees.  In a way, the LEDs on the module will act as a kind of spectrum
analyzer, generally with fainter lights toward the bottom (the higher frequency
outputs) where the harmonics have decreased in level[^1].

Ultimately, because of this distortion, each output will offer a unique
combination of partials that have been boosted, added, or removed.

[^1]: The amplitude of these outputs isn't actually lower, however, they are
still distorted to +5V when the threshold is crossed.  Instead, they are very
narrow pulses, creating the illusion of a dimmer LED at audio rate.

### Multiband Bitcrusher

Another way of looking at the distortion applied to each of the filter outputs
is as a fixed-resolution (1-bit) bitcrusher.  From this perspective, the
operation can be viewed as multiband bitcrusher with sample rate reduction
applied before the filters and bit-depth reduction occurring after.

Sending a copy of the original input signal to a mixer, along with several of
the outputs, allows for a wide range of effects, from subtle saturation to
static-like sounds.  Keep in mind, however, that the outputs are 0-5V signals,
so inverting/attenuating some of the inputs may be necessary, especially if
using a DC-coupled mixer.

## Installing

If you want to install this firmware as-is, you can download the .uf2 file (see
"Releases") and follow the instructions
[here](https://github.com/oamodular/uncertainty?tab=readme-ov-file#how-to-reinstall-the-default-firmware).

If you would like to modify the code and run it, you can do so using the Arduino
IDE, following the instructions
[here](https://wiki.seeedstudio.com/XIAO-RP2040-with-Arduino/).

