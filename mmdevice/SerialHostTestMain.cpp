
#include <MMCore.h>
#include <PluginManager.h>
#include <iostream>
#include <string>
#include <vector>


int main(int argc, char* argv[]) 
{
    using namespace std;

    CPluginManager pmanager;
    vector<string> paths = pmanager.GetSearchPaths();
    cout << "Make sure mmgr_dal_SerialManager.dll is somewhere in the following path" << endl;
    for (auto p : paths) {
        cout << '\t' << p << endl;
    }

    string portName{ "COM3" };

    CMMCore core;
    core.enableStderrLog(true);
    //core.enableDebugLog(true);
    string label("Device");
    try
    {
        // Initialize the device
        // ---------------------
        cout << "Loading " << portName << " from library SerialManager..." << endl;
        core.loadDevice(portName.c_str(), "SerialManager", portName.c_str());
        cout << "Done." << endl;

        core.setSerialProperties(portName.c_str(), // port name
                            "5000.0", // answer timeout
                            "9600", // baud - IGNORED BY TEENSY USB INTERFACE
                            "0.0", // delay between char ms
                            "Off", // handshaking
                            "None", // parity
                            "1"); // stop bits

        cout << "Initializing..." << endl;
        core.initializeAllDevices();
        cout << "Done." << endl;

        cout << "Reading 5 times from serial port" << endl;
        for (int i = 0; i < 5; i++) {
            string ans = core.getSerialPortAnswer(portName.c_str(), "\r\n");
            cout << i << '\t' << ans << endl;
        }
        cout << "Done" << endl;
    }
    catch (CMMError& err)
    {
        cout << err.getMsg();
        return 1;
    }
}