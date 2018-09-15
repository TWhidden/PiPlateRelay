#ifndef DAQC2PLATE_H
#define DAQC2PLATE_H

#include "spibase.h"
#include <algorithm>

namespace SPIW {

class DAQC2Plate : public SPIBase
{

public :
        enum leds {off = 0, red , green, yellow , blue, magenta , cyan ,white };

private :


   double calDAC[8];
   double calScale[8];
   double calOffset[8];

   virtual uint8_t  CalGetByte(int ptr);

   virtual bool okToSend( void )
   {
       if( getAckPin())
           return true;
       return false;
   }

   virtual rtnStructure SendCommand( cmdStructure cmd, int readbackBytes, bool stopAt0 = false );

   virtual bool waitOnAck(int usec);

public:



   /// constructor
   DAQC2Plate ( uint8_t addr = 32,  uint8_t PinFrame = 6,   uint8_t PinSRQ = 3,  uint8_t PinACK = 4, int Device = 1  )
       :  SPIBase(addr)

   {
       std::fill( &calDAC[0], &calDAC[8], 0 );
       std::fill( &calScale[0], &calScale[8], 1 );
       std::fill( &calOffset[0], &calOffset[8], 0 );
       initBoard( PinFrame,  PinSRQ,   PinACK,  Device);
       ppCal();
   }

   virtual ~DAQC2Plate() {}

   /// get all the adc at one time
   virtual int   getADCall( double values[8]);

   /// get only 1 adc, get by channel numner
   virtual int   getADC( int channel, double &value);

   /// set the dac but channel number
   virtual int   setDAC( int channel,   double value );

   /// get the calibration constants as set by the factory, used after init board..
   virtual void  ppCal(void);

   /// set a bit by pin number
   virtual int   setBit( int pin, int state );

   /// get a pin by pin number
   virtual int   getBit( int pin, int &bit);

   /// get all the bits at one time
   virtual int   getAllBits( int &inputByte);

   /// pin, then when = INT_EDGE_FALLING  INT_EDGE_RISING INT_EDGE_BOTH
   virtual int   enableDinIRQ( int pin, int when);

   /// disables a boards interrupts by pin number
   virtual int   disableDinIRQ( int pin);

   /// turns on boards ability to interrupt
   virtual int  intEnable();	///  DAQC2 will pull down on INT pin if an enabled event occurs

   /// turns off a boards ability to interrupt
   virtual int  intDisable();  /// DAQC2 will not assert interrupts

   /// read INT flag register in DAQC2 - this clears interrupt line and the register
   virtual int  getINTflags(unsigned short &reg);

   /// turns a led by leds colors
   virtual int  setLedCondition( DAQC2Plate::leds led );

   /// get a led current condition
   virtual int  getLedCondition( DAQC2Plate::leds &led );

   /// static members  to determine is an address contains a board.
   static bool isDAQC2Valid(uint8_t addr = 32,  uint8_t PinFrame = 6,   uint8_t PinSRQ = 3,  uint8_t PinACK = 4, int Device = 1);


};
}
#endif // DAQC2PLATE_H
