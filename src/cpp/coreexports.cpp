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

int RelaysAvailable()
{
    int boardsAvailable = 0;
    for ( int adr = 24; adr < 24 +8; ++adr )
    {
        if( SPIW::RELAYPlate::isRelayValid(adr ) )
        {
            qDebug() << "FOUND RELAY CARD AT ADDRESS " << adr;
            SPIW::RELAYPlate relay(adr);
            if( relay.ValidBoard())
            {
                qDebug() << relay.getFWRevision() << "   " << relay.getHWRevision() << relay.getID();
            }

            boardsAvailable++;
        }
    }

    return boardsAvailable;
}