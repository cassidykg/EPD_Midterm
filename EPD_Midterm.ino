#include <StevesAwesomeButton.h>
#include <Adafruit_NeoPixel.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S i2s1;                 //xy=104,310
AudioEffectDelay delay1;            //xy=265,344
AudioEffectWaveshaper waveshape1;   //xy=405,558
AudioMixer4 mixer1;                 //xy=412,355
AudioEffectBitcrusher bitcrusher1;  //xy=436,505
AudioMixer4 mixer2;                 //xy=592,513
AudioFilterStateVariable filter1;   //xy=655,242
AudioMixer4 mixer3;                 //xy=700,348
AudioEffectFreeverb freeverb1;      //xy=721,470
AudioMixer4 mixer4;                 //xy=810,262
AudioOutputI2S i2s2;                //xy=905,312
AudioConnection patchCord1(i2s1, 0, delay1, 0);
AudioConnection patchCord2(i2s1, 0, mixer1, 0);
AudioConnection patchCord3(delay1, 0, mixer1, 1);
AudioConnection patchCord4(waveshape1, 0, mixer2, 2);
AudioConnection patchCord5(mixer1, 0, mixer2, 0);
AudioConnection patchCord6(mixer1, waveshape1);
AudioConnection patchCord7(mixer1, bitcrusher1);
AudioConnection patchCord8(bitcrusher1, 0, mixer2, 1);
AudioConnection patchCord9(mixer2, 0, mixer3, 0);
AudioConnection patchCord10(mixer2, freeverb1);
AudioConnection patchCord11(filter1, 0, mixer4, 0);
AudioConnection patchCord12(mixer3, 0, filter1, 0);
AudioConnection patchCord13(freeverb1, 0, mixer3, 1);
AudioConnection patchCord14(mixer4, 0, i2s2, 0);
AudioConnection patchCord15(mixer4, 0, i2s2, 1);
AudioControlSGTL5000 sgtl5000_1;  //xy=386,214
// GUItool: end automatically generated code

Adafruit_NeoPixel neopixel = Adafruit_NeoPixel(4, 34, NEO_RGB);

int delayPot = 0;
int delayLength = 0;
StevesAwesomeButton delayButton(32, 0, INPUT_PULLUP);

int distortionPot = 0;
float waveShapePot = 0.0;
int bitDepth = 0;
StevesAwesomeButton distortionButton(31, 0, INPUT_PULLUP);

float waveShapeOff[17] = { -1.0, -0.875, -0.75, -0.625, -0.5, -0.375, -0.25, -0.125, 0.0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875, 1.0 };
float waveShapeActive[17] = { -1.0, -0.966, -0.933, -0.9, -0.87, -0.82, -0.75, -0.15, 0.0, 0.15, 0.3, 0.35, 0.5, 0.57, 0.6, 0.65, 0.7 };
float fuzzArray[17] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

StevesAwesomeButton distortionSwitch(33, 0, INPUT_PULLUP);

int reverbPot = 0;
int reverbLength = 0;
StevesAwesomeButton reverbButton(30, 0, INPUT_PULLUP);

bool delayOn = false;
bool distortionOn = false;
bool reverbOn = false;
bool distortionType = false;

int volumePot = 0;
float volume = 0.0;

void setup() {
  Serial.begin(9600);

  AudioMemory(175);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);

  delayButton.pressHandler(delayButtonPress);
  distortionButton.pressHandler(distortionButtonPress);
  reverbButton.pressHandler(reverbButtonPress);
  distortionSwitch.pressHandler(distortionSwitchPress);

  mixer1.gain(0, 0.8);  // clean signal
  mixer1.gain(1, 0.8);  // delay output
  mixer2.gain(0, 0.8);  // output of mixer 1 (delay or clean)
  mixer2.gain(1, 0.5);  // bit crush output
  mixer2.gain(1, 0.8);  // waveshaper distortion output
  mixer3.gain(0, 0.8);  // output of mixer 2 (delay and/or distortion or clean)
  mixer3.gain(1, 0.8);  // reverb output
  mixer4.gain(0, 0);    // output of mixer 3 through filter (delay and/or distortion and/or reverb or clean)

  filter1.frequency(8500.00);

  neopixel.begin();
  neopixel.clear();
  neopixel.show();
}

void loop() {
  neopixel.setPixelColor(0, 127, 0, 127);  //neopixel indicating that the circuit is on
  neopixel.show();

  delayPot = analogRead(A10);
  delayLength = map(delayPot, 0, 1023, 50, 125);

  reverbPot = analogRead(A12);
  reverbLength = map(reverbPot, 0, 1023, 0.2, 1);

  volumePot = analogRead(A14);
  volume = map(volumePot, 0, 1023, 0, 3);
  mixer4.gain(0, volume);

  runEffects();

  delayButton.process();
  distortionButton.process();
  reverbButton.process();
  distortionSwitch.process();
}

void delayButtonPress(int _buttonNum) {
  delayOn = !delayOn;
}

void distortionButtonPress(int _buttonNum) {
  distortionOn = !distortionOn;
}

void reverbButtonPress(int _buttonNum) {
  reverbOn = !reverbOn;
}

void distortionSwitchPress(int _buttonNum) {
  distortionType = !distortionType;
}

void runEffects() {

  if (delayOn == true) {  // turn on delay
    delay1.delay(0, delayLength);
    neopixel.setPixelColor(1, 100, 100, 100);
    delay(5);

    if (distortionOn == true) {  // turn on distortion after delay
      if (distortionType == true) {
        neopixel.setPixelColor(2, 100, 100, 100);
        runWaveShaper();
      } else {
        runBitCrush();
        neopixel.setPixelColor(2, 100, 75, 0);
      }

      if (reverbOn == true) {  // turn on reverb after delay and distortion
        freeverb1.roomsize(reverbLength);
        neopixel.setPixelColor(3, 100, 100, 100);
        delay(5);

      } else {  // by pass reverb with delay and distortion still running
        freeverb1.roomsize(0);
        neopixel.setPixelColor(3, 0, 0, 0);
        delay(5);
      }
    } else {  // bypass distortion with delay still running
      bypassDistortion();
      neopixel.setPixelColor(2, 0, 0, 0);

      if (reverbOn == true) {  // turn on reverb after delay with distortion bypassed
        freeverb1.roomsize(reverbLength);
        neopixel.setPixelColor(3, 100, 100, 100);
        delay(5);

      } else {  // bypass reverb with delay running and distortion bypassed
        freeverb1.roomsize(0);
        neopixel.setPixelColor(3, 0, 0, 0);
        delay(5);
      }
    }
  } else {  // bypass delay
    delay1.disable(0);
    neopixel.setPixelColor(1, 0, 0, 0);
    delay(5);

    if (distortionOn == true) {  //turn on distortion with delay bypassed
      if (distortionType == true) {
        neopixel.setPixelColor(2, 100, 100, 100);
        runWaveShaper();
      } else {
        runBitCrush();
        neopixel.setPixelColor(2, 100, 75, 0);
      }

      if (reverbOn == true) {  //turn on reverb after distortion with delay bypassed
        freeverb1.roomsize(reverbLength);
        neopixel.setPixelColor(3, 100, 100, 100);
        delay(5);

      } else {  // bypass reverb with distortion running and delay bypassed
        freeverb1.roomsize(0);
        neopixel.setPixelColor(3, 0, 0, 0);
        delay(5);
      }
    } else {  //bypass distortion and delay
      bypassDistortion();
      neopixel.setPixelColor(2, 0, 0, 0);
      if (reverbOn == true) {  // turn on reverb with delay and distortion bypassed
        freeverb1.roomsize(reverbLength);
        neopixel.setPixelColor(3, 100, 100, 100);
        delay(5);

      } else {  // bypass all
        freeverb1.roomsize(0);
        neopixel.setPixelColor(3, 0, 0, 0);
        delay(5);
      }
    }
  }
}


void bypassDistortion() {  // bypass all distortion
  bitcrusher1.bits(16);
  mixer2.gain(0, 0.8);
  mixer2.gain(1, 0);
  mixer2.gain(2, 0);
  delay(5);
}

void runBitCrush() {  //run bitcrusher as distortion type
  distortionPot = analogRead(A11);
  bitDepth = map(distortionPot, 0, 1023, 6, 1);
  bitcrusher1.bits(bitDepth);
  mixer2.gain(0, 0);
  mixer2.gain(1, 0.5);
  mixer2.gain(2, 0);
  delay(5);
}

void runWaveShaper() {  //run waveshaper as distortion type
  mixer2.gain(0, 0);
  mixer2.gain(1, 0);
  mixer2.gain(2, 0.8);
  waveShapePot = analogRead(A11);
  for (int i = 0.0; i < 17; i++) {  //the waveshaper is mapped in 17 increments where waveShapeOff sends through the audio unchanged and waveShapeActive fully distorts it
    fuzzArray[i] = map(waveShapePot, 0, 1023, waveShapeOff[i], waveShapeActive[i]);  //mapping the array to the difference between waveShapeOff and waveShapeActive allows it to smoothly fade from undistorted to fully distorted
    waveshape1.shape(fuzzArray, 17);  //this reads the full array into the waveshaper code and tells it that is has 17 elements in it
    delay(5);
  }
}