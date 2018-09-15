#include "spibase.h"

#include <QTime>

namespace SPIW {

static  uint8_t ppFRAME = -1;
static  uint8_t ppINT   = -1;
static  uint8_t ppACK   = -1;

static  int   odevice    = -1;
static  bool  initYet = false;
static  bool  initWirePi = false;



int SPIBase::spiError(int code, const char *message, ...)
{
    va_list argp ;
    char buffer [1024] ;
    va_start(argp, message) ;
    vsnprintf(buffer, 1023, message, argp) ;
    va_end(argp) ;
    qDebug() << buffer ;
    return (code * -1);
}

int SPIBase::spiRead(int fd, const uint8_t *buff, size_t len, uint32_t speed, uint32_t mode, uint32_t delay)
{
    struct spi_ioc_transfer spi;
    memset(&spi, 0, sizeof(spi));

    spi.tx_buf = (unsigned long) NULL;
    spi.rx_buf = (unsigned long) buff;
    spi.len = len;
    spi.delay_usecs = delay;
    spi.speed_hz = speed;
    spi.bits_per_word = 8;
    spi.cs_change = 0;

    // adapted from py_spidev/spidev_module
#ifdef SPI_IOC_WR_MODE32
    spi.tx_nbits = 0;
#endif
#ifdef SPI_IOC_RD_MODE32
    spi.rx_nbits = 0;
#endif

    int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &spi);
    if(ret < 1)
    {
        return spiError(1100, "spiRead(): Can't send spi message");
    }

    if(mode & SPI_CS_HIGH)
    {
        ret = read(fd, (void*) &buff[0], 0);
    }

    // success
    return ret;
}



bool SPIBase::initBoard(void)
{
    if( !initWirePi )
    {
        wiringPiSetupGpio(); // BCM pin layout root mode
        initWirePi = true;
    }

    if( !initYet )
    {
        // Initialize frame signal
        pinMode(ppFRAME, OUTPUT);
        if(disableFrame() < 0)
        {
            return false;
        }

        // Initialize interrupt control
        pinMode( ppINT, INPUT);
        pullUpDnControl( ppINT, PUD_UP);

        // Initialize ACK
        pinMode( ppACK, INPUT);
        pullUpDnControl( ppACK, PUD_UP);
        initYet = true;

        // time to system
        usleep(PP_DELAY);

    }
    return initYet;
}

bool SPIBase::initBoard(uint8_t PinFrame, uint8_t PinSRQ, uint8_t PinACK, int Device)
{
    ppFRAME =  wpiPinToGpio( PinFrame );
    ppINT   =  wpiPinToGpio( PinSRQ  );
    ppACK   =  wpiPinToGpio( PinACK );

    odevice  = Device;
    bool rtn = initBoard();
    if(rtn)
        wiringPiSPISetup (odevice, PP_SPI_BUS_SPEED);

    return rtn;
}

uint8_t SPIBase::getBoardAddress(void)
{
    cmdStructure cmd;
    rtnStructure rtn = SendCommand( cmd, 1);
    if( rtn.valid)
        return rtn.rtn[0];
    return 0xff;

}

QString  SPIBase::getID(void )
{
    char strtemp[256];

    cmdStructure cmd(1);

    rtnStructure rtn = SendCommand( cmd, 20, true );

    if( rtn.valid) {
        snprintf(strtemp, sizeof(strtemp) - 1, "%s", rtn.rtn);
        return QString(strtemp);
    }
    return QString("Not Valid Request");
}

QString SPIBase::getHWRevision()
{
    uint8_t value = 0;
    cmdStructure cmd(0x02);
    rtnStructure rtn = SendCommand(cmd,1,false);
    value = rtn.rtn[0];
    double whole = (double)(value >> 4);
    double point = (double)(value & 0x0F);

    QString rev = QString("%1.%2").arg(whole).arg(point);

    return rev;
}

QString SPIBase::getFWRevision()
{
    uint8_t value = 0;
    cmdStructure cmd(0x03);
    rtnStructure rtn = SendCommand(cmd,1,false);
    value = rtn.rtn[0];
    double whole = (double)(value >> 4);
    double point = (double)(value & 0x0F);
    QString rev = QString("%1.%2").arg(whole).arg(point);
    return rev;
}

int SPIBase::updateLED(const uint8_t led, const uint8_t state)
{
    if( led == 0 || led == 1)
        ;
    else
        return 1;

    uint8_t  command = 0x00;
    switch(state)
    {
        // set
        case STATE_ON:
        {
            command = 0x60;
            break;
        }
        // clear
        case STATE_OFF:
        {
            command = 0x61;
            break;
        }
        // toggle
        case STATE_TOGGLE:
        {
            command = 0x62;
            break;
        }
        default:
        {
            qDebug() << "Invalid LED state value %d." << state;
            return -1;
        }
    }

    cmdStructure cmd(command,led);
    rtnStructure rtn = SendCommand(cmd,0,false);
    if( rtn.valid)
        return 0;

    return SPIERROR;
}

bool SPIBase::getLEDState(const uint8_t led)
{
    if( led == 0 || led == 1)
        ;
    else
        return false;

    cmdStructure cmd(0x63, led, 0);
    rtnStructure rtn = SendCommand(cmd,1,false);

    if( rtn.rtn[0] == 0)
        return false;
    return true;

}

int SPIBase::setBit(int  bit, const int state)
{
    Q_UNUSED(bit)
    Q_UNUSED(state)
    return STATE_ERROR;
}

uint8_t SPIBase::getSTATE()
{
    return STATE_ERROR;
}

bool SPIBase::ValidBoard()
{
    _ioAddress = getBoardAddress();
    return (( _address != _ioAddress) ? false : true);
}


int SPIBase::enableFrame(void)
{
    // enable SPI frame transfer
    digitalWrite(ppFRAME, HIGH);

    // time to system
    usleep(PP_DELAY);

    // check bit has raised
    if(!digitalRead(ppFRAME))
    {
        qDebug() << "Unable to Enable a ppFRAME";
        return SPIERROR;
    }
    return 0;
}

int SPIBase::disableFrame(void)
{
    // enable SPI frame transfer
    digitalWrite(ppFRAME, LOW);

    // time to system
    usleep(PP_DELAY);

    // check bit has released
    if(digitalRead(ppFRAME))
    {
        qDebug() << "Unable to Disable a ppFRAME";
        return SPIERROR;
    }
    return 0;
}

int SPIBase::getODevice()
{
    return odevice;
}

SPIBase::SPIBase(uint8_t x_address) :
     _address(x_address)
    ,_ioAddress(0xfe)
{
    if( !initWirePi )
    {
        wiringPiSetupGpio(); // BCM pin layout root mode
        initWirePi = true;
    }
}


int SPIBase::getAckPin()
{
    return (int)digitalRead(ppACK);
}


rtnStructure SPIBase::SendCommand(cmdStructure cmd, int readbackBytes, bool stopAt0)
{
    rtnStructure rtn(readbackBytes);
    cmd.txbuff[0] += getAddress();
    enableFrame();
    //int fd =  wiringPiSPISetup (getODevice(), PP_SPI_BUS_SPEED);
    int fd =  wiringPiSPIGetFd (getODevice());
    if(fd < 0)
    {
        rtn.nbr_rtn = 0;
        rtn.valid = false;
        qDebug() << 1400 << "Unable to open SPI bus device. Make sure SPI is enabled by raspi-config tool.";
        return rtn;
    }
    else
    {
        int rw = wiringPiSPIDataRW(getODevice(), cmd.txbuff, cmd.cmdSize());
        if( rw < 0)
        {
            qDebug() << " DAQC2 failed wiringPiSPIDataRW(getODevice(), cmd.txbuff, cmd.cmdSize());";
            rtn.valid = false;
            return rtn;
        }
        usleep(70);

        if( readbackBytes > 0  || stopAt0 )
        {
            int i = 0;
            uint8_t byte[1] = {0x00};
            uint32_t mode = SPI_CPHA | SPI_RX_DUAL | SPI_TX_DUAL | SPI_NO_CS;
            while(i < readbackBytes && i < rtn.maxRtnSize() )
            {
                if ( spiRead(fd, &byte[0], 1, PP_SPI_BUS_SPEED, mode, 20) < 0)
                {
                    rtn.nbr_rtn = i;
                    break;
                }

                // stop at zero terminator
                if((byte[0] == 0x0) && stopAt0)
                {
                    rtn.nbr_rtn = i;
                    rtn.rtn[i] = byte[0];
                    break;

                }
                rtn.rtn[i] = byte[0];
                i++;
            }
        }
        else
        {
           rtn.nbr_rtn = 0;
        }
    }
    disableFrame();
    return rtn;
}

int SPIBase::reset()
{
    cmdStructure cmd(0x0f);
    rtnStructure rtn = SendCommand( cmd, 0);
    if( !rtn.valid)
        return SPIERROR;
    return 0;

}


bool SPIBase::waitOnAck(int usec)
{
    usec = usec;
    return true;
}

}


