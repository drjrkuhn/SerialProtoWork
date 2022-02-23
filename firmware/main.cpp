// NOTES: The current toolset for teensyduino is arm-gcc-5 branch
// This supports most of C++14

#include "Arduino.h"
#include "arduinoslip.h"
#include "slipproto.h"
#include <deque>

using namespace sproto;
ArduinoSlipProtocol<usb_serial_class> SlipSerial(Serial);

void setup() {
    SlipSerial.begin();
    Serial.println("========== RESET ==========");
}

const size_t rxbuffer_size = 128;
char rxbuffer[rxbuffer_size];

int outrep = 0;

void loop() {
    if (outrep++ < 5) {
        String out1 = "Lorus Ipsum";
        String out2 = "Favius## Rex\\ \\\\#\\##Aeturnum padre##";
        Serial.print("   >>");
        SlipSerial.crcKermitReset();
        uint16_t crc = SlipSerial.crcKermitCalc(out1.c_str(), out1.length());
        SlipSerial.writeSlipEscaped(out1.c_str(), out1.length());
        SlipSerial.writeSlipEnd(crc);
        Serial.println();
        Serial.print("   >>");
        SlipSerial.crcKermitReset();
        crc = SlipSerial.crcKermitCalc(out2.c_str(), out2.length());
        SlipSerial.writeSlipEscaped(out2.c_str(), out2.length());
        SlipSerial.writeSlipEnd(crc);
        Serial.println();
    }
    // Serial.println("Waiting for input");
    // delay(500);
    size_t bytes_read = 0;
    error_t err       = SlipSerial.readSlipEscaped(rxbuffer, rxbuffer_size, bytes_read);
    if (err != NO_ERROR) {
        if (err == ERROR_TIMEOUT) {
            Serial.print("!!timeout ");
        } else {
            Serial.print("!!error ");
            Serial.print(err);
        }
        if (bytes_read > 0) {
            Serial.print("<<[");
            Serial.print(bytes_read);
            Serial.print("]");
            rxbuffer[bytes_read] = '\0';
            Serial.print(rxbuffer);
        }
        Serial.println();
        SlipSerial.clearInput();
    } else if (bytes_read > 0) {
        Serial.print("<<[");
        Serial.print(bytes_read);
        Serial.print("]");
        rxbuffer[bytes_read] = '\0';
        Serial.println(rxbuffer);
    } else {
        // Serial.println("==no input==");
        // SlipSerial.clearInput();
    }
    // delay(1000);
}
