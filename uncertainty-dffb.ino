#include <Arduino.h>
#include <math.h>
#include <vector>
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

#define SAMPLE_RATE (10000.0)
#define TIMER_INTERVAL ((int)(1000000.0/SAMPLE_RATE))
#define MAX_SAMPLES 3
#define SECTION_LENGTH 6

// number of gate outs
#define NUM_GATES 8

using namespace std;

// hold pins for gates
int gatePins[] = {27,28,29,0,3,4,2,1};
// ADC input pin
int inputPin = 26;

double debugVal = -50;

class RingBuffer {
private:
  double _data[MAX_SAMPLES];
  int _start = 0;
  int _size = 0;

public:
  RingBuffer() {}

  double operator[](int index) const {
    return _data[(_start + index) % MAX_SAMPLES];
  }

  void put(const double& value) {
    if (_size < MAX_SAMPLES) {
      _data[(_start + _size) % MAX_SAMPLES] = value;
      _size += 1;
    }
    else {
      _data[_start] = value;
      _start = (_start + 1) % MAX_SAMPLES;
    }
  }
};

class IIRFilter {
private:
  bool _debug = false;
  int _stages;
  double* _sos;
  RingBuffer* _feedbackSamples;

public:
  IIRFilter(vector<vector<double>> sos, bool debug = false) {
    _stages = sos.size();
    _feedbackSamples = new RingBuffer[_stages];
    _sos = new double[_stages * SECTION_LENGTH]();
    for (int i=0; i<_stages; i++) {
      for (int j=0; j<6; j++) {
        _sos[SECTION_LENGTH * i + j] = sos[i][j];
      }
    }
    _debug = debug;
  }

  double Process(const RingBuffer& samples) {
    double value;
    for (int stage=0; stage<_stages; stage++) {
      const RingBuffer& input = (stage == 0) ? samples : _feedbackSamples[stage-1];
      RingBuffer& feedback = _feedbackSamples[stage];

      int stageStart = SECTION_LENGTH * stage;
      double b0 = _sos[stageStart];
      double b1 = _sos[stageStart + 1];
      double b2 = _sos[stageStart + 2];
      //double a0 = _sos[stageStart + 3]; // (this is always 1.0 in our filters, so this step isn't necessary)
      double a1 = _sos[stageStart + 4];
      double a2 = _sos[stageStart + 5];

      double x0 = input[2]; // "last" element of the ring buffer is the most recent
      double x1 = input[1];
      double x2 = input[0];
      double y1 = feedback[2];
      double y2 = feedback[1];

      value = b0*x0 + b1*x1 + b2*x2 - a1*y1 - a2*y2;
      //value /= a0;

      if (_debug && stage == 0) {
        debugVal = value;
      }

      feedback.put(value);
    }
    return value;
  }
};

// beep at different frequencies
void startupSequence() {
  // startup sequence
  int beepCount = 4;
  int beepLength = 128;
  delay(beepLength / 2);
  for (int beep=0; beep<beepCount; beep++) {
    uint32_t counter = 0;
    while (counter < beepLength) {
      for(int i=0; i<8 ;i++) {
        int pulseWidth = (8 - i) * (beepCount - beep);
        if (counter < pulseWidth * 16) {
          gpio_put(gatePins[i], (counter / pulseWidth) % 2 == 1);
        }
        else {
          gpio_put(gatePins[i], 0);
        }
      }
      counter++;
      delay(1);
    }
    delay(beepLength / 2);
  }
  delay(beepLength / 2);
  for(int i=0;i<8;i++) {
    gpio_put(gatePins[i], 0);
  }
}

RingBuffer samples;
IIRFilter filter1( // lowpass
  {
    {0.010016186,  -0.019417040,   0.010016186,   1.000000000,  -1.972250810,   0.972941224}
  }
);
IIRFilter filter2( // band 10.055497328,   0.088247099,   0.055497328,   1.000000000,  -0.954431745,   0.714624324
  {
    {0.010067696,  -0.019138804,   0.010067696,   1.000000000,  -1.974139272,   0.977827957},
    {1.000000000,  -1.999961581,   1.000000000,   1.000000000,  -1.987165271,   0.988203315}
  }
);

IIRFilter filter3( // band 2
  {
    {0.010599523,  -0.017280187,   0.010599523,   1.000000000,  -1.941596936,   0.956177626},
    {1.000000000,  -1.999846468,   1.000000000,   1.000000000,  -1.972407026,   0.976533780}
  }
);
IIRFilter filter4( // band 3
  {
    {0.012961527,  -0.010771256,   0.012961527,   1.000000000,  -1.857622553,   0.914506880},
    {1.000000000,  -1.999388156,   1.000000000,   1.000000000,  -1.937225860,   0.953524066}
  }
);
IIRFilter filter5( // band 4
  {
    {0.022292240,   0.012093023,   0.022292240,   1.000000000,  -1.622586984,   0.838041179},
    {1.000000000,  -1.997588941,   1.000000000,   1.000000000,  -1.845050693,   0.908480168}
  }
);
IIRFilter filter6( // band 5
  {
    {0.055497328,   0.088247099,   0.055497328,   1.000000000,  -0.954431745,   0.714624324},
    {1.000000000,  -1.990921630,   1.000000000,   1.000000000,  -1.581802229,   0.819755786}
  }
);
IIRFilter filter7( // band 6
  {
    {0.1594224,   0.3128679,   0.1594224,   1.0000000,   0.6411885,   0.6061573},
    {1.0000000,  -1.9717995,   1.0000000,   1.0000000,  -0.8337540,   0.6265015}
  }
);
IIRFilter filter8( // highpass
  {
    {0.1300065,  -0.2372743,   0.1300065,   1.0000000,   0.8580251,   0.4159908}
  }
);

double to_double(uint32_t value) {
  double center = 1 << 11;
  return (value - center) / center;
}

bool distort(const double& value) {
  return value > 0.1;
}

void core1_entry() {
  double output5 = filter5.Process(samples);
  gpio_put(gatePins[4], distort(output5));

  double output6 = filter6.Process(samples);
  gpio_put(gatePins[5], distort(output6));

  double  output7 = filter7.Process(samples);
  gpio_put(gatePins[6], distort(output7));

  double output8 = filter8.Process(samples);
  gpio_put(gatePins[7], distort(output8));
}

// audio rate callback- meat of the program goes here
static bool audioHandler(struct repeating_timer *t) {
  uint32_t start = micros();
  // poll ADC and convert to [-1.0, 1.0]
  double input = to_double(adc_read());

  //debugVal = input;

  samples.put(input);

  // run half of the filters on the other core
  multicore_reset_core1();
  multicore_launch_core1(core1_entry);

  // run the other half here
  double output1 = filter1.Process(samples);
  gpio_put(gatePins[0], distort(output1));

  double output2 = filter2.Process(samples);
  gpio_put(gatePins[1], distort(output2));

  double output3 = filter3.Process(samples);
  gpio_put(gatePins[2], distort(output3));

  double output4 = filter4.Process(samples);
  gpio_put(gatePins[3], distort(output4));

  debugVal = (micros() - start);

  return true;
}

struct repeating_timer _timer_;

void setup() {
  // overclock - dialed back a little after adding second core until things seemed stable
  set_sys_clock_khz(200000, true);

  // initialize ADC
  adc_init();
  adc_gpio_init(inputPin);
  adc_select_input(0);
  
  // initialize gate out pins
  for(int i=0; i<NUM_GATES; i++) {
    int pin = gatePins[i];
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
  }

  // oooh beeps
  startupSequence();

  // audio callback
  add_repeating_timer_us(-TIMER_INTERVAL, audioHandler, NULL, &_timer_);

  // init serial debugging
  Serial.begin(115200);
}

void loop() {
  Serial.println(debugVal, 6);
  delay(1);
}