#include "Arduino.h"

void setup()
{
    Serial.begin(9600); // Baud IGNORED by teensy USB interface
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }
}

void loop()
{
    Serial.println("Hello");
    delay(1000);
}
