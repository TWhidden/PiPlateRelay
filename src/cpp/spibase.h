#ifndef SPIBASE_H
#define SPIBASE_H

#include <QDebug>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
// #include <bcm2835.h>


namespace SPIW {


/* SPI device initialization parameters */
#define PP_SPI_BUS_SPEED		500000

#define PP_DELAY 1000

#define PP_MAX_RELAYS 			8
#define PP_MAX_DIGITAL_IN		8
#define PP_MAX_ANALOG_IN		8
#define PP_MAX_DIGITAL_OUT		7

#define SPIERROR                -1

// Relay or LED update modes
#define STATE_OFF	 			0x00
#define STATE_ON 				0x01
#define STATE_TOGGLE 			0x02
#define STATE_ALL				0x03
#define STATE_ERROR             SPIERROR

#define PP_MAX_DAC_VOLT			4.59	//4.095f
#define PP_MAX_DAC_BITRES		1023


/**
 * @brief The cmdStructure struct, is used to talk to PiPlate IO.. it is used for sending data.
 *
 */
struct cmdStructure
{
        /// 4 parameters plus a check sum if anyone would care later
        uint8_t txbuff[5];

        /// default constructor for this structure
        cmdStructure (uint8_t cmd = 0, uint8_t arg1 = 0, uint8_t arg2 = 0 )
        {
           txbuff[0] = 0;
           txbuff[1] = cmd;
           txbuff[2] = arg1;
           txbuff[3] = arg2;
        }

        int cmdSize() {
            return (int)sizeof( txbuff) - 1;
        }
};

/**
 * @brief The rtnStructure struct  returns the data, if any from the sendCommand to the piplate hardware.
 */
struct rtnStructure
{
    /// number of bytes returned from piplate hardware
    int nbr_rtn;

    /// make a lot of bytes, incase needed later
    uint8_t rtn[40];

    /// true if data was send back correctly from piplate hardware
    bool valid;

    /// constructor for the rtnStructor
    rtnStructure(int nbr = 0)
        : nbr_rtn(nbr)
        , valid(true)
    {
        /// use normal c call to fill with 0s
        ::memset(rtn, 0, sizeof(rtn));
    }

    /// returns the max number of bytes possible
    int maxRtnSize() {
        return sizeof(rtn);
    }

    /// returns the actual number of bytes return from the piplate hardware.
    int rtnSize()
    {
        return nbr_rtn;
    }
};


/**
 * @brief The SPIBase class Base class for DAQC2 and RELAY piplate hardware classes.
 */

class SPIBase
{
private :

protected :

    uint8_t  _address;
    uint8_t _ioAddress;

    int spiError(int code, const char* message, ...);
    int spiRead(int fd, uint8_t const* buff, size_t len, uint32_t speed, uint32_t mode, uint32_t delay);

    /**
     * Enable frame signal to transmit commands to the
     * board through the SPI bus.
     * @param pBoard Handle of the PI-Plates board
     * @return 0 success otherwise signal an error
     */
    int enableFrame(void);

    /**
     * Disable frame signal to prevent transmission
     * through the SPI bus.
     * @param pBoard Handle of the PI-Plates board
     * @return 0 success otherwise signal an error
     */
    int   disableFrame(void);

    /// get the io device 0 or 1 to being used to talk to piplates
    int   getODevice ();

public:

    /// constructor
    SPIBase(  uint8_t  x_address );

    /// inits the board, once pins and such already set..
    bool initBoard(void);

    /// normal way to init the board
    bool initBoard( uint8_t PinFrame,   uint8_t PinSRQ,  uint8_t PinACK, int Device);




    /// exchange with piplate hardware
    virtual rtnStructure SendCommand( cmdStructure cmd, int readbackBytes, bool stopAt0 = false );

    /// reset the boards
    virtual int reset();



    /// inherited mostly by DAQC2 to ensure ppACK us down, not needed by others
    virtual bool okToSend( void )
    {
        return true;
    }

    /// could be added to other boards later, so make this vitual so others can implement, used by DAQC2
    virtual bool waitOnAck( int usec);

    /// getAck pin for new DACQ2
    int getAckPin(void);

    /// gets the hardware address for the board.. does the io to get board address
    virtual uint8_t getBoardAddress( void);

    /// get the who is this board
    virtual QString     getID(void);

    /// gets the hardware revision
    virtual QString getHWRevision(void);

    /// gets the firmware revision
    virtual QString getFWRevision(void);

    /// updates the LED
    virtual int     updateLED(const uint8_t led, const uint8_t state);

    /// gets the LED State  STATE_ON, OFF, etc
    virtual bool    getLEDState(const uint8_t led );

    /// gets a bit, a discrete output to a relay or output pin
    virtual int     setBit( int pin, int state );

    /// getsa all input states as a byte
    virtual uint8_t getSTATE();

    /// validates the board and gets the hardware address... returns true is the hardware address is the same as the desired address
    virtual bool ValidBoard();

    /// return the address the board is suppose to be
    virtual uint8_t getAddress(void)
    {
        return _address;
    }




};




}

#endif // SPIBASE_H
