#include "relayplate.h"

namespace SPIW {

int RELAYPlate::setBit(int pin, int state)
{
    if( pin >= 1 && pin <=7)
        ;
    else
        return STATE_ERROR;
    uint8_t _state;

    switch (state) {
    case STATE_OFF:
        _state = 0x11;
        break;
    case STATE_ON:
        _state = 0x10;
        break;
    case STATE_TOGGLE:
        _state = 0x12;
        break;
    default:
         return( STATE_ERROR);
    }
    cmdStructure cmd(_state, pin, 0);
    rtnStructure rtn = SendCommand( cmd, 0, false );
    if( !rtn.valid)
        return( STATE_ERROR);
    return(state);
}

int RELAYPlate::getPINSTATE(int pin)
{
    if( pin >= 1 && pin <=7)
        ;
    else
        return STATE_ERROR;

    cmdStructure cmd(0x14, 0, 0);
    rtnStructure rtn = SendCommand( cmd, 1, false );
    if( !rtn.valid)
        return( STATE_ERROR);
    return (int)rtn.rtn[0];
}

bool RELAYPlate::isRelayValid(uint8_t addr, uint8_t PinFrame, uint8_t PinSRQ, uint8_t PinACK, int Device)
{
    RELAYPlate testRELAYPlate(addr, PinFrame, PinSRQ,  PinACK, Device  );
    return  testRELAYPlate.ValidBoard();
}



}
