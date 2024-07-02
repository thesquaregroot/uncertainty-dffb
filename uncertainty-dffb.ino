#include <Arduino.h>
#include <math.h>
#include <vector>
#include <list>
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define SAMPLE_RATE (10000.0)
#define TIMER_INTERVAL ((int)(1000000.0/SAMPLE_RATE))
#define MAX_SAMPLES 3

// number of gate outs
#define NUM_GATES 8

using namespace std;

// hold pins for gates
int gatePins[] = {27,28,29,0,3,4,2,1};
// ADC input pin
int inputPin = 26;

double debugVal = -50;

class IIRFilter {
private:
  int _stages;
  vector<vector<double>> _sos;

  vector<list<double>> _feedbackSamples;
  bool _debug = false;

public:
  IIRFilter(vector<vector<double>> sos, bool debug = false) {
    _sos = std::move(sos);
    _stages = _sos.size();
    // initialize feedback lists for each stage
    for (int i=0; i<_stages; i++) {
      _feedbackSamples.push_back(list<double>());
    }
    _debug = debug;
  }

  double Process(const list<double>& samples) {
    double value;
    for (int stage=0; stage<_stages; stage++) {
      vector<double> section = _sos[stage];
      const list<double>& input = (stage == 0) ? samples : _feedbackSamples[stage-1];
      list<double>& feedback = _feedbackSamples[stage];

      double b0 = section[0];
      double b1 = section[1];
      double b2 = section[2];
      double a0 = section[3];
      double a1 = section[4];
      double a2 = section[5];

      auto inputIter = input.begin();
      double x0 = (inputIter == input.end()) ? 0 : *inputIter++;
      double x1 = (inputIter == input.end()) ? 0 : *inputIter++;
      double x2 = (inputIter == input.end()) ? 0 : *inputIter;
      auto feedbackIter = feedback.begin();
      double y1 = (feedbackIter == feedback.end()) ? 0 : *feedbackIter++;
      double y2 = (feedbackIter == feedback.end()) ? 0 : *feedbackIter;

      value = b0*x0 + b1*x1 + b2*x2 - a1*y1 - a2*y2;
      value /= a0;

      feedback.push_front(value);
      if (feedback.size() > MAX_SAMPLES) {
        feedback.pop_back();
      }
    }
    return value;
  }
};

// beep at different frequencies
void startupSequence() {
  uint32_t startupCounter = 0;
  // startup sequence
  while(startupCounter<128) {
    for(int i=0; i<8 ;i++) {
      int pulseWidth = 8 - i;
      if (startupCounter < pulseWidth * 16) {
        gpio_put(gatePins[i], (startupCounter / pulseWidth) % 2 == 1);
      }
      else {
        gpio_put(gatePins[i], 0);
      }
    }
    startupCounter++;
    delay(1);
  }
  for(int i=0;i<8;i++) {
    gpio_put(gatePins[i], 0);
  }
}

std::list<double> samples;
IIRFilter filter1( // lowpass
  {
    {0.0031501927, -0.0062943005, 0.0031501927, 1.0000000000, -1.9933418629, 0.9933705511},
    {1.0000000000, -1.9996294537, 1.0000000000, 1.0000000000, -1.9976620670, 0.9977502532}
  }
);
IIRFilter filter2( // band 1
  {
    {9.3261305e-05, -1.8597351e-04, 9.3261305e-05, 1.0000000e+00, -1.9934158e+00, 9.9373064e-01},
    {1.0000000e+00, -1.9999831e+00, 1.0000000e+00, 1.0000000e+00, -1.9953481e+00, 9.9596664e-01},
    {1.0000000e+00, 6.9410512e-09, -9.9999999e-01, 1.0000000e+00, -1.9977824e+00, 9.9794317e-01}
  }
);

IIRFilter filter3( // band 2
  {
    {0.00099611680, -0.00195110019, 0.00099611680, 1.00000000000, -1.98495934402, 0.98764878378},
    {1.00000000000, -1.98952911463, 1.00000000044, 1.00000000000, -1.98851087182, 0.98991749122},
    {1.00000000000, -1.99962888502, 0.99999999031, 1.00000000000, -1.99082600702, 0.99472253780},
    {1.00000000000, -1.99990625580, 1.00000000929, 1.00000000000, -1.99705029659, 0.99807013166}
  }
);
IIRFilter filter4( // band 3
  {
    {5.3012166e-04, -9.6526901e-04, 5.3012166e-04, 1.0000000e+00, -1.9554125e+00, 9.6522762e-01},
    {1.0000000e+00, -1.9994654e+00, 1.0000000e+00, 1.0000000e+00, -1.9581968e+00, 9.7755649e-01},
    {1.0000000e+00, 2.8103075e-12, -1.0000000e+00, 1.0000000e+00, -1.9834120e+00, 9.8847267e-01}
  }
);
IIRFilter filter5( // band 4
  {
    {0.0011910203, -0.0013760519, 0.0011910203, 1.0000000000,  -1.8747407903,   0.9393966842},
    {1.0000000000, -1.7485373989, 1.0000000000, 1.0000000000,  -1.8779692592,   0.9738225783},
    {1.0000000000, -1.9907564881, 1.0000000000, 1.0000000000,  -1.9192283562,   0.9547372628},
    {1.0000000000, -1.9976795314, 1.0000000000, 1.0000000000,  -1.9619051873,   0.9865586068}
  }
);
IIRFilter filter6( // band 5
  {
    {0.0022903843,   0.0010514855,   0.0022903843,   1.0000000000,  -1.4608185176,   0.9428065629},
    {1.0000000000,  -0.8717885857,   1.0000000000,   1.0000000000,  -1.5425022405,   0.8665626904},
    {1.0000000000,  -1.9520900190,   1.0000000000,   1.0000000000,  -1.7162704805,   0.8973147312},
    {1.0000000000,  -1.9880983461,   1.0000000000,   1.0000000000,  -1.8406864239,   0.9688042492}
  }
);
IIRFilter filter7( // band 6
  {
    {0.0097637011,   0.0169180227,   0.0097637011,   1.0000000000,  -0.0090050166,   0.8991050575},
    {1.0000000000,   1.0425407387,   1.0000000000,   1.0000000000,  -0.4165277839,   0.7391453658},
    {1.0000000000,  -1.7950938238,   1.0000000000,   1.0000000000,  -0.9984037310,   0.7735489244},
    {1.0000000000,  -1.9514599247,   1.0000000000,   1.0000000000,  -1.3582586621,   0.9273517779}
  }
);
IIRFilter filter8( // highpass
  {
    {0.0304759,  -0.0229736,   0.0304759,   1.0000000,   0.7278049,   0.5509325},
    {1.0000000,  -1.3483508,   1.0000000,   1.0000000,   0.2979928,   0.8673432},
    {1.0000000,  -1.0000000,           0,   1.0000000,   0.5649213,           0}
  }
);

double to_double(uint32_t value) {
  int center = 1 << 11;
  double d = ((int)value - center) / (double) center;
  return d;
}

bool distort(double value) {
  return value >= 0.5 || value < -0.75;
}

// audio rate callback- meat of the program goes here
static bool audioHandler(struct repeating_timer *t) {
  uint32_t start = micros();
  // poll ADC and convert to [-1.0, 1.0]
  double input = to_double(adc_read());

  //debugVal = input;

  samples.push_front(input);
  if (samples.size() > MAX_SAMPLES) {
    samples.pop_back();
  }

  double output1 = filter1.Process(samples);
  //debugVal = output1;
  gpio_put(gatePins[0], distort(output1));

  double output2 = filter2.Process(samples);
  gpio_put(gatePins[1], distort(output2));

  /*double output3 = filter3.Process(samples);
  gpio_put(gatePins[2], distort(output3));

  double output4 = filter4.Process(samples);
  gpio_put(gatePins[3], distort(output4));

  double output5 = filter5.Process(samples);
  gpio_put(gatePins[4], distort(output5));

  double output6 = filter6.Process(samples);
  gpio_put(gatePins[5], distort(output6));

  double output7 = filter7.Process(samples);
  gpio_put(gatePins[6], distort(output7));

  double output8 = filter8.Process(samples);
  gpio_put(gatePins[7], distort(output8));*/

  debugVal = (micros() - start);

  return true;
}

struct repeating_timer _timer_;

void setup() {
  // 2x overclock for MAX POWER
  set_sys_clock_khz(250000, true);

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
  Serial.begin(9600);
}

void loop() {
  Serial.println(debugVal);
  delay(1);
}