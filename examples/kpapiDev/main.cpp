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
    
    ProfilingAmp profilingAmp (connectedDevices[2]);
    
    std::cout << "Performance " << profilingAmp.getActivePerformanceName () << " loaded" << std::endl;
    std::cout << "Rig " << profilingAmp.getActiveRigName() << " is active" << std::endl;

    //auto *wah = profilingAmp.getWahWahStomp();
    auto *wah = profilingAmp.getGenericWahStomp ();

    if (wah == nullptr) {
        std::cout << "No wah wah in current rig!" << std::endl;
        return 0;
    }

    std::cout << "Rig has a wah wah!" << std::endl;

    std::this_thread::sleep_for (std::chrono::seconds(2));

    wah->toggleOnOff (true);
    
    return 0;
}

#endif