#include "utils.h"

const int DRY = 820;   //x >= DRY  means the sensor is completly dry
const int WATER = 485; // x <= WATER means the sensor is submerged in water
const int MOIST_SOIL = 700;
int sensorPin = A3;

void setup()
{
    Serial.begin(9600);
}

void loop()
{
    Serial.println(analogRead(sensorPin));
}
