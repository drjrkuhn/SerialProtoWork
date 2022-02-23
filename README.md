# SerialProtoWork

Development branch for new Micro-manager device to Arduino/Teensy serial com protocol.

Probably using CBOR encoding and either COBS or SLIP

**Makefiles don't work well with spaces**, so we need the old DOS 8.3 short
name in windows or the escaped space '\ ' for mac.
For windows use the short path format (no spaces), open up a command prompt
```cmd
cd [folder]
for %I in ("%cd%") do echo %~sI
```

System environment variables used
- `ARDUINO_HARDWARE` folder [VS Code, makefile]
    - Windows: `C:\PROGRA~2\Arduino\hardware`
    - Mac with Teensyduino: `/Applications/Teensyduino.app/Contents/Java/hardware`
    - Mac with Arduino: `/Applications/Arduino.app/Contents/Java/hardware`
- `TY_TOOLS` folder [make]
- `ARDUINO_LIBS` folder [make]
    - Windows: `C:\Users\jrkuhn\DOCUME~1\Arduino\LIBRAR~1`
