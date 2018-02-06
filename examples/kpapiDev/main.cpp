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
 
    
    auto connectedDevices = SimpleMIDI::PlatformSpecificImplementation::searchMIDIDevices();

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

    std::cout << "Rig has a wah wah! It's switched " << std::endl;

    std::cout << "Gain is set to " << (float)profilingAmp.getAmpGain() << std::endl;

    std::this_thread::sleep_for (std::chrono::seconds(2));

    wah->setToggleState (true);

    std::cout << "Gain is set to " << (float)profilingAmp.getAmpGain() << std::endl;

    std::cout << "Setting Gain to 10" << std::endl;

    profilingAmp.setAmpGain (10);

    std::cout << "Amp Name: " << profilingAmp.getActiveAmpName() << std::endl;
    std::cout << "Amp Manufacturer Name: " << profilingAmp.getActiveAmpManufacturerName() << std::endl;
    std::cout << "Amp Model Name: " << profilingAmp.getActiveAmpModelName() << std::endl;
    std::cout << "Cab Name: " << profilingAmp.getActiveCabName() << std::endl;
    std::cout << "Cab Manufacturer Name: " << profilingAmp.getActiveCabManufacturerName() << std::endl;
    std::cout << "Cab Model Name: " << profilingAmp.getActiveCabModelName() << std::endl;
    std::cout << "Rig 1: " << profilingAmp.getRigName (ProfilingAmp::RigNr::Rig1) << std::endl;
    std::cout << "Rig 2: " << profilingAmp.getRigName (ProfilingAmp::RigNr::Rig2) << std::endl;
    std::cout << "Rig 3: " << profilingAmp.getRigName (ProfilingAmp::RigNr::Rig3) << std::endl;
    std::cout << "Rig 4: " << profilingAmp.getRigName (ProfilingAmp::RigNr::Rig4) << std::endl;
    std::cout << "Rig 5: " << profilingAmp.getRigName (ProfilingAmp::RigNr::Rig5) << std::endl;

    return 0;
}

#endif