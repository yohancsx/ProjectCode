//library imports, use the most current versions of these
#include <FastLED.h>
#include "arduinoFFT.h"

//the number of samples to determine the frequency from, must be a multiple of two
//lower numbers in this case are generally better, as it leads to a faster response time
#define SAMPLES 128          

//samples taken per second in Hz, must be less than 10000 due to thet being the limit of the ADC 
#define SAMPLING_FREQUENCY 9000 

//create the FFT object for calculating the FFT
arduinoFFT FFT = arduinoFFT();

//the sampling period
unsigned int sampling_period_us;

//number of LEDs in your strip, set this to the number of LEDs you have 
#define NUM_LEDS 25

//the data pin which the LEDs are connected to, set this to whatever your LEDs are connected to
#define DATA_PIN 4

//setup the array of led objects, set the type (CRGB) in this case to the type of your LEDs
CRGB leds[NUM_LEDS];



void setup() { 

  //start the serial monitor if you want to see the frequencies
  Serial.begin(115200);

  //setup the smpling period based on the sampling frequency
  sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));

  //add/initialize the LEDs so that we can use them 
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  
}


void loop()
{
  //value to hold the microseconds
  unsigned long microseconds;

  //array to hold the real values of fft (amplitude)
  double vReal[SAMPLES];
  
  //array to hold the imaginary values of fft (phase, which we don't care about)
  double vImag[SAMPLES];

  //for the number of samples
  for(int i=0; i<SAMPLES; i++)
  {
      //get the time
      microseconds = micros();    //Overflows after around 70 minutes!

      //read a sample
      vReal[i] = analogRead(0);

      //set phase (we don't care, but we need it to be zero so that the calculation works out later on)
      vImag[i] = 0;

      //delay here for one sampling period before the next sample is taken
      while(micros() < (microseconds + sampling_period_us)){
      }
  }

  //now that we have all the samples, we can take the FFT
  //window the data first
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);

  //now compute the FFT
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);

  //convert from complex frame to magnitude frame so we can see the amplitude of each frequency
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

  //get the value of the frequency of the peak, that is the most dominant frequency found. This should be the note played by the flute, as long as the background is
  //not very noisy, and the microphone is close to the flute
  double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);

  //print the peak value, if we want
  Serial.println(peak);   

  //delay before the next reading if we want to, you can change this
  delay(50);
  
  //show the color based on the peak value
  showGradientColor(peak);
    
    
}


//shows a color based on a gradient using the color index passed in (in our case the peak)
void showGradientColor(double colorIndex){

  //set the brightness, this is best at 150, if you don't plan on using an external power supply
  uint8_t brightness = 150;

  //set a scaling value, you can change this to change how much of the gradient you see in the flute at once
  double scalingValue = 100;

  //for each of the leds
  for( int i = 0; i < NUM_LEDS; i++) {
    
        //set the color based on the LED number in the strip, as well as the colorindex (peak value), scaled by some factor
        //note that the gradient values are desined based on valued between 0 and 255, so if you change the scalingValue to a lower number, you get
        //more and more of the gradient displayed, also set the blending mode of the gradient
        //also we are using the rainbow colors gradient, but you could veen define your own gradient if you want to
        leds[i] = ColorFromPalette(RainbowColors_p, (i + colorIndex/2)*(255/scalingValue), brightness, LINEARBLEND);
    }

    //show the LEDs with new values
    FastLED.show();

    //delay so that we can see the colors, you can decrease this, but it may caluse jittering in the LEDs
    FastLED.delay(100);
}
