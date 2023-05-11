#include <Arduino.h>
#line 1 "/Users/corneliuscarl/Downloads/Sound_Meter/Sound_Meter.ino"
#include "arduinoFFT.h"

arduinoFFT FFT;
/*
These values can be changed in order to evaluate the functions
*/
const uint16_t samples = 64; //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 100; //Hz, must be less than 10000 due to ADC

unsigned int sampling_period_us;
unsigned long microseconds;

double vReal[samples];
double vImag[samples];

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03


#include <PDM.h>



// buffer to read samples into, each sample is 16-bits
short sampleBuffer[256];

// number of samples read
volatile int samplesRead;

#line 32 "/Users/corneliuscarl/Downloads/Sound_Meter/Sound_Meter.ino"
void setup();
#line 53 "/Users/corneliuscarl/Downloads/Sound_Meter/Sound_Meter.ino"
void loop();
#line 113 "/Users/corneliuscarl/Downloads/Sound_Meter/Sound_Meter.ino"
void onPDMdata();
#line 127 "/Users/corneliuscarl/Downloads/Sound_Meter/Sound_Meter.ino"
void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType);
#line 32 "/Users/corneliuscarl/Downloads/Sound_Meter/Sound_Meter.ino"
void setup() {
  sampling_period_us = round(1000000*(1.0/samplingFrequency));

  Serial.begin(9600);
  while (!Serial);

  // configure the data receive callback
  PDM.onReceive(onPDMdata);

  // optionally set the gain, defaults to 20
  // PDM.setGain(30);

  // initialize PDM with:
  // - one channel (mono mode)
  // - a 16 kHz sample rate
  if (!PDM.begin(1, 16000)) {
    Serial.println("Failed to start PDM!");
    while (1);
  }
}

void loop() {
  // wait for samples to be read
  if (samplesRead) {
    microseconds = micros();
    for(int i=0; i<samples; i++)
    {
        vReal[i] = sampleBuffer[i];
        vImag[i] = 0;
        while(micros() - microseconds < sampling_period_us){
          //empty loop
        }
        microseconds += sampling_period_us;
    }
    FFT = arduinoFFT(vReal, vImag, samples, samplingFrequency); /* Create FFT object */
    /* Print the results of the sampling according to time */
    Serial.println("Data:");
    PrintVector(vReal, samples, SCL_TIME);
    FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);	/* Weigh data */
    Serial.println("Weighed data:");
    PrintVector(vReal, samples, SCL_TIME);
    FFT.Compute(FFT_FORWARD); /* Compute FFT */
    Serial.println("Computed Real values:");
    PrintVector(vReal, samples, SCL_INDEX);
    Serial.println("Computed Imaginary values:");
    PrintVector(vImag, samples, SCL_INDEX);
    FFT.ComplexToMagnitude(); /* Compute magnitudes */
    Serial.println("Computed magnitudes:");
    PrintVector(vReal, (samples >> 1), SCL_FREQUENCY);
    double x = FFT.MajorPeak();
    Serial.println(x, 6); //Print out what frequency is the most dominant.
    //while(1); /* Run Once */
     delay(2000); /* Repeat after delay */  

    // print samples to the serial monitor or plotter
    // for (int i = 0; i < samplesRead; i++) {
    //   Serial.println(sampleBuffer[i]);
    //   // check if the sound value is higher than 500
    //   if (sampleBuffer[i]>=500){
    //     digitalWrite(LEDR,LOW);
    //     digitalWrite(LEDG,HIGH);
    //     digitalWrite(LEDB,HIGH);
    //   }
    //   // check if the sound value is higher than 250 and lower than 500
    //   if (sampleBuffer[i]>=250 && sampleBuffer[i] < 500){
    //     digitalWrite(LEDB,LOW);
    //     digitalWrite(LEDR,HIGH);
    //     digitalWrite(LEDG,HIGH);
    //   }
    //   //check if the sound value is higher than 0 and lower than 250
    //   if (sampleBuffer[i]>=0 && sampleBuffer[i] < 250){
    //     digitalWrite(LEDG,LOW);
    //     digitalWrite(LEDR,HIGH);
    //     digitalWrite(LEDB,HIGH);
    //   }
    }

    // clear the read count
    samplesRead = 0;
  }

void onPDMdata() {
  // query the number of bytes available
  int bytesAvailable = PDM.available();
  //Serial.println(bytesAvailable);

  //while(1);

  // read into the sample buffer
  PDM.read(sampleBuffer, bytesAvailable);

  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;
}

void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa;
    /* Print abscissa value */
    switch (scaleType)
    {
      case SCL_INDEX:
        abscissa = (i * 1.0);
	break;
      case SCL_TIME:
        abscissa = ((i * 1.0) / samplingFrequency);
	break;
      case SCL_FREQUENCY:
        abscissa = ((i * 1.0 * samplingFrequency) / samples);
	break;
    }
    Serial.print(abscissa, 6);
    if(scaleType==SCL_FREQUENCY)
      Serial.print("Hz");
    Serial.print(" ");
    Serial.println(vData[i], 4);
  }
  Serial.println();
}
