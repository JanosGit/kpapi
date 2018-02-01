#include "../kpapi.h"

ProfilingAmp::StompType ProfilingAmp::StompBase::getStompType() {
    return stompType;
}

void ProfilingAmp::StompBase::toggleOnOff (bool onOff) {
    _amp.updateLowResNRPN (_slotPage, NRPNParameter::OnOff, onOff);
}






