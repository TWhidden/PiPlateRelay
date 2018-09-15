#include <QTime>
#include <QString>
#include <QVariant>

#include "daqc2plate.h"

namespace SPIW {

rtnStructure DAQC2Plate::SendCommand(cmdStructure cmd, int readbackBytes, bool stopAt0)
{
   if (!getAckPin())
       qDebug() << "ppACK still low from last move.";

    rtnStructure rtn(readbackBytes + 1);
    cmd.txbuff[0] += getAddress();

    //int fd =  wiringPiSPISetup (getODevice(), PP_SPI_BUS_SPEED);
    int fd =  wiringPiSPIGetFd(getODevice());
    if(fd < 0)
    {
        rtn.nbr_rtn = 0;
        qDebug() << 1400 << "Unable to open SPI bus device. Make sure SPI is enabled by raspi-config tool.";
    }
    else
    {
        bool DataGood = true;
        enableFrame();
        int rw = wiringPiSPIDataRW(getODevice(), cmd.txbuff, cmd.cmdSize());
        if( rw < 0)
        {
           rtn.valid = false;
           qDebug() << " DAQC2 failed wiringPiSPIDataRW(getODevice(), cmd.txbuff, cmd.cmdSize());";
           return rtn;
        }

        DataGood = waitOnAck(50);

        if( (readbackBytes > 0  || stopAt0) && DataGood)
        {
            readbackBytes++;
            DataGood = waitOnAck(80);

            int i = 0;
            uint8_t byte[1] = {0x00};
            uint32_t mode = SPI_CPHA | SPI_RX_DUAL | SPI_TX_DUAL | SPI_NO_CS;

            while(i < readbackBytes && i < rtn.maxRtnSize() && DataGood )
            {
                if ( spiRead(fd, &byte[0], 1, PP_SPI_BUS_SPEED, mode, 20) < 0)
                {
                    qDebug() << "spiRead Error";
                    rtn.nbr_rtn = i;
                    rtn.valid = false;
                    break;
                }
                // stop at zero terminator
                if((byte[0] == 0x0) && stopAt0)
                {
                    rtn.nbr_rtn = i;
                    rtn.rtn[i] = byte[0];
                    if ( spiRead(fd, &byte[0], 1, PP_SPI_BUS_SPEED, mode, 20) < 0)
                    {
                          qDebug() << "spiRead Error";
                          rtn.valid = false;
                    }
                    else
                    {
                         rtn.rtn[++i] = byte[0];
                    }
                    rtn.nbr_rtn = i;
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

bool DAQC2Plate::waitOnAck(int usec )
{
    QTime v;
    bool Wait = true;
    v.start();
    while( Wait)
    {
        if( !getAckPin() )
        {
           return true;
        }
        if(v.elapsed() > usec)
        {
           break;
        }
     }
    return false;
}


int DAQC2Plate::getADCall(double values[8])
{
    cmdStructure cmd(0x31);
    rtnStructure rtn = SendCommand( cmd, 16, false );
    ::memset(values, 0, sizeof(values[0]) * 8);
    if( rtn.valid)
    {
        uint8_t *resp = rtn.rtn;
        for(int i = 0; i < 8; i++)
        {
            values[i]=(256*resp[2*i]+resp[2*i+1]);
            values[i]=(values[i]*24.0/65536)-12.0;
            values[i]=values[i]*calScale[i]+calOffset[i];
        }
        return 0;
    }
    return SPIERROR;
}

int DAQC2Plate::getADC(int channel, double &value)
{
    if( channel >= 0 && channel <= 8)
        ;
    else
        return SPIERROR;

    // ppCMD(addr,0x30,channel,0,2);
    cmdStructure cmd(0x30,channel,0);
    rtnStructure rtn = SendCommand( cmd, 2, false );
    if( !rtn.valid)
    {
        return SPIERROR;
    }
    uint8_t *resp = rtn.rtn;
    value=(256*resp[0]+resp[1]);

    if (channel==8)
        value=value*5.0*2.4/65536;
    else
    {
        value=(value*24.0/65536)-12.0;
        value=value*calScale[channel]+calOffset[channel];
    }

    return 0;
}

uint8_t DAQC2Plate::CalGetByte(int ptr)
{
    // resp=ppCMD(addr,0xFD,2,ptr,1)
    cmdStructure cmd(0xfd,2,ptr);
    rtnStructure rtn = SendCommand( cmd, 1, false );
    if( rtn.valid)
        return( rtn.rtn[0]);
    return 0;
}


int DAQC2Plate::setDAC(int channel, double volts)
{
    double percent;
    uint16_t digval;
    uint8_t hibyte = 0;
    uint8_t lobyte = 0;

    if(volts < 0)
        volts = 0;
    else if ( volts >= 4.095)
        volts = 4.095;

    int ichannel = channel;

    if (ichannel >= 0 && ichannel < 4)
        ;
    else
    {
        qDebug() <<  "ERROR: DAC channel %d must be 0, 1, or 3 ";
        return SPIERROR;
    }

   percent = volts * 100.0 / PP_MAX_DAC_VOLT;
   digval = round(percent * PP_MAX_DAC_BITRES / 100.0);

   if (digval > 4095)
        digval =4095;

   hibyte = digval >> 8;
   lobyte = digval - (hibyte << 8);

   cmdStructure cmd(0x40+channel,hibyte,lobyte);
   rtnStructure rtn = SendCommand( cmd, 0, false );
   if( !rtn.valid)
   {
       qDebug() << "Error DAC command failed";
       return SPIERROR;
   }
   return 0;
}

void DAQC2Plate::ppCal()
{
    short values[6];
    for( int i = 0; i < 8; ++i)
    {
        for( int j = 0; j < 6; j++)
        {
            values[j]=CalGetByte(6*i+j);
        }
        short cSign=values[0] & 0x80;
        calScale[i]=0.04*((values[0]&0x7F)*256+values[1])/32767;
        if (cSign != 0)
            calScale[i] *= -1;
        calScale[i]+=1;

        cSign=values[2]&0x80;
        calOffset[i]=0.2*((values[2]&0x7F)*256+values[3])/32767;  // #16 bit signed offset calibration values - range is +/- 0.1

        if (cSign != 0)
            calOffset[i] *= -1;
        cSign=values[4] & 0x80;
        calDAC[i]=0.04*((values[4]&0x7F)*256+values[5])/32767; //#16 bit signed DAC calibration values - range is +/-4%

        if (cSign != 0)
            calDAC[i] *= -1;
        calDAC[i] += 1;

    }
}

int DAQC2Plate::setBit(int pin, int state)
{
    if( pin >= 0 && pin <= 7)
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

int DAQC2Plate::getBit(int pin, int &bit)
{
    if( pin >= 0 && pin <= 7)
        ;
    else
        return STATE_ERROR;

    cmdStructure cmd(0x20, bit, 0);
    rtnStructure rtn = SendCommand( cmd, 1, false );
    if( !rtn.valid)
        return( STATE_ERROR);

    bit = (int)rtn.rtn[0];
    return 0;
}

int DAQC2Plate::getAllBits(int &inputByte)
{

    cmdStructure cmd(0x25, 0, 0);
    rtnStructure rtn = SendCommand( cmd, 1, false );
    if( !rtn.valid)
        return( STATE_ERROR);

    inputByte = (int)rtn.rtn[0];
    return 0;
}

int DAQC2Plate::enableDinIRQ(int pin, int when)
{

    if( pin >= 0 && pin <= 7)
        ;
    else
        return STATE_ERROR;

    uint8_t _cmd;

    switch (when) {
    case INT_EDGE_FALLING:
        _cmd = 0x21;
        break;
    case INT_EDGE_RISING:
        _cmd = 0x22;
        break;
    case INT_EDGE_BOTH :
         _cmd = 0x23;
        break;
    default:
       return STATE_ERROR;
    }
    cmdStructure cmd(_cmd, pin, 0);
    rtnStructure rtn = SendCommand( cmd, 0, false );
    if( !rtn.valid)
        return( STATE_ERROR);
    return 0;

}

int DAQC2Plate::disableDinIRQ(int pin)
{
    if( pin >= 0 && pin <= 7)
        ;
    else
        return STATE_ERROR;
    cmdStructure cmd(0x24, pin, 0);
    rtnStructure rtn = SendCommand( cmd, 0, false );
    if( !rtn.valid)
        return( STATE_ERROR);
    return 0;
}

int DAQC2Plate::intEnable()
{
    cmdStructure cmd(0x04, 0, 0);
    rtnStructure rtn = SendCommand( cmd, 0, false );
    if( !rtn.valid)
        return( STATE_ERROR);
    return 0;
}

int DAQC2Plate::intDisable()
{
    cmdStructure cmd(0x05, 0, 0);
    rtnStructure rtn = SendCommand( cmd, 0, false );
    if( !rtn.valid)
        return( STATE_ERROR);
    return 0;
}

int DAQC2Plate::getINTflags(unsigned short &reg)
{
    cmdStructure cmd(0x04, 0, 0);
    rtnStructure rtn = SendCommand( cmd, 2, false );
    if( !rtn.valid)
        return( STATE_ERROR);

    reg = 256*rtn.rtn[0]+rtn.rtn[1];
    return 0;

}

int DAQC2Plate::setLedCondition(DAQC2Plate::leds led)
{

    int x_led = 0;
    x_led += led;  /// for debugging purposes
    cmdStructure cmd(0x60, x_led, 0);
    rtnStructure rtn = SendCommand( cmd, 0, false );
    if( !rtn.valid)
        return( STATE_ERROR);
    return 0;
}

int DAQC2Plate::getLedCondition(DAQC2Plate::leds &led)
{
    cmdStructure cmd(0x63, 0, 0);
    rtnStructure rtn = SendCommand( cmd, 1, false );
    if( !rtn.valid)
        return( STATE_ERROR);

    led =  (DAQC2Plate::leds)rtn.rtn[0];
    return 0;
}

bool DAQC2Plate::isDAQC2Valid(uint8_t addr, uint8_t PinFrame, uint8_t PinSRQ, uint8_t PinACK, int Device)
{
    DAQC2Plate TestDAQCC2(addr, PinFrame, PinSRQ,  PinACK, Device  );
    return TestDAQCC2.ValidBoard();
}

}
