#include "../kpapi.h"

bool ProfilingAmp::GenericWahStomp::isStillValid() {
    if (((stompType & StompType::GenericMask) == StompType::GenericWah) && (_slotPage != PageUninitialized)) {
        return true;
    }
    return false;
}

void ProfilingAmp::GenericWahStomp::setManual (uint16_t manual) {
    _amp.updateHighResNRPN (_slotPage, WahManual, manual);
}

ProfilingAmp::GenericWahStomp* ProfilingAmp::getGenericWahStomp (StompSlot stompSlot) {
    return (GenericWahStomp*) getGenericStompInstance (StompType::GenericWah, stompSlot);
}

bool ProfilingAmp::WahWahStomp::isStillValid() {
    if (((stompType & StompType::SpecificMask) == StompType::WahWah) && (_slotPage != PageUninitialized)) {
        return true;
    }
    return false;
}