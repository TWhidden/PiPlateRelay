#ifndef RELAYPLATE_H
#define RELAYPLATE_H

#include "spibase.h"

namespace SPIW {

/**
 * @brief The RELAYPlate class  Used an inherited class for the relay piplate board
 */

class RELAYPlate : public SPIBase
{
private :

public:


    /// constructor for the pi plate relay board
    RELAYPlate( uint8_t addr = 24,  uint8_t PinFrame = 6,   uint8_t PinSRQ = 3,  uint8_t PinACK = 4, int Device = 1 )
            :  SPIBase(addr)
    {
        initBoard( PinFrame,  PinSRQ,   PinACK,  Device);
    }

    virtual ~RELAYPlate() {}

    /// sets a bit on to state, STATE_ON STATE_OFF
    virtual int setBit( int pin, int state );

    /// gets a pin state
    virtual int getPINSTATE( int pin);

    static bool isRelayValid(uint8_t addr = 24,  uint8_t PinFrame = 6,   uint8_t PinSRQ = 3,  uint8_t PinACK = 4, int Device = 1);


};

}
#endif // RELAYPLATE_H
