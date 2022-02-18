// NOTES: The current toolset for teensyduino is arm-gcc-5 branch
// This supports most of C++14

#include "Arduino.h"
#include <deque>
#include "slipproto.h"
#include "arduinoslip.h"

ArduinoSlipProtocol<usb_serial_class> SlipSerial(Serial);

void setup()
{
    SlipSerial.begin();
    Serial.println("========== RESET ==========");
}

const size_t rxbuffer_size = 128;
char rxbuffer[rxbuffer_size];

void loop()
{
    String out1 = "Lorus Ipsum";
    String out2 = "Favius## Rex\\ \\\\#\\##Aeturnum padre##";
    SlipSerial.writeSlipEscaped(out1.c_str(), out1.length(), true);
    Serial.println();
    SlipSerial.writeSlipEscaped(out2.c_str(), out2.length(), true);
    Serial.println();
    Serial.println("Waiting for input");
    delay(1000);
    size_t bytes_read = SlipSerial.readSlipEscaped(rxbuffer, rxbuffer_size);
    if (bytes_read > 0) {
        Serial.print("Read ");
        Serial.print(bytes_read);
        Serial.print(" bytes: ");
        rxbuffer[bytes_read] = '\0';
        Serial.println(rxbuffer);
    } else {
        Serial.println("==no input==");
    }
    delay(1000);
}
