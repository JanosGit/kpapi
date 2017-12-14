//
//  main.cpp
//  kpapiDev
//
//  Created by Janos Buttgereit on 28.05.17.
//  Copyright © 2017 Janos Buttgereit. All rights reserved.
//


#include "../../kpapi.h"

#ifndef SIMPLE_MIDI_ARDUINO // avoid any Arduino IDE from compiling this example

#include <iostream>


int main(int argc, const char * argv[]) {
 
    
    std::vector<SimpleMIDI::HardwareResource> connectedDevices = searchMIDIDevices();

    for (auto &dev : connectedDevices) {
        std::cout << dev.deviceName << std::endl;
    }
    
    KemperProfilingAmp kemperProfilingAmp (connectedDevices[2]);
    
    std::cout << kemperProfilingAmp.getActiveRigName() << std::endl;

    std::this_thread::sleep_for (std::chrono::seconds(2));

    //kemperProfilingAmp.setTempo (500);
    
    return 0;
}

#endif