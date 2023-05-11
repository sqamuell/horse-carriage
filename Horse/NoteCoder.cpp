#include "NoteCoder.h"
#include "Arduino.h"

NoteCoder::NoteCoder() {

}

float NoteCoder::decodeDirection(int * frequencies) {
  int number = 0;

  for (int i = 0; i < sizeof(frequencies) - 1; i++) {
    int digit = round((frequencies[i] - startMessage - step) / step);


    Serial.print("Current frequency:");
    Serial.println(frequencies[i]);
    Serial.print("Current digit:");
    Serial.println(digit);
    number += digit * pow(10, i);
  }

  float direction = float(number) / 1000;
  return direction;
}

void NoteCoder::encodeDirection(int * frequencies, float direction) {
  int intDirection = round(direction * 1000);

  int index = 0; 
  while(intDirection > 0 && index < 3) {
    frequencies[index] = startMessage + (intDirection % 10) * step + step;
    intDirection /= 10;
    index++;
  }
}