#include <relayplate.h>

int SetPinState(uint8_t boardId, uint8_t pin, uint8_t state)
{
    SPIW::RELAYPlate relay(24 + boardId);
    if (state == 1)
    {
        relay.setBit(pin, STATE_ON);
    }
    else
    {
        relay.setBit(pin, STATE_OFF);
    }
    return 0;
}