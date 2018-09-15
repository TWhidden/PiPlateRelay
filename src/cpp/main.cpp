#include <QCoreApplication>
#include <QDebug>
#include <unistd.h>
#include "spibase.h"
#include "relayplate.h"
#include "daqc2plate.h"
#include <QTime>

int main(int argc, char *argv[])
{

    Q_UNUSED(argc)
    Q_UNUSED(argv)

    for ( int adr = 32; adr < 32+8; ++adr )
    {
        /// use the first one to set the ppACK, ppFRAME, Device, and ppINT for the system.. not needed again
        if ( SPIW::DAQC2Plate::isDAQC2Valid( adr, 6,3,4,1) )
        {
            qDebug() << "FOUND DAQC2 at ADDRESS " << adr;
            SPIW::DAQC2Plate adc(adr);
            qDebug() << QTime::currentTime();  /// get the time of the day..
            if( adc.ValidBoard())
            {
                qDebug() << adc.getFWRevision() << "   " << adc.getHWRevision() << adc.getID();
                adc.ppCal();
                qDebug() << QTime::currentTime();
                QTime tms;

                double dataArray[5][8];
                tms.start();  /// start a time to recording
                for(int k = 0; k < 5; k++)
                {
                     adc.getADCall(dataArray[k]);
                }
                int time = tms.elapsed() / 5; /// not take the average over 5 readings
                qDebug() << "Elapsed ms average for 5 complete reading of all 8 adcs is: " << time;
                for( int k = 0; k < 5; ++k )
                {
                    for ( int i = 0; i < 8; ++i)
                    {
                        qDebug() << QString("ADC[%1][%2] = %3").arg(k).arg(i).arg(dataArray[k][i]);
                    }
                    qDebug() << "NEXT.... Set";
                }

                qDebug() << "Testing one by one";
                for ( int i = 0; i < 8; ++i)
                {
                    double value;
                    if ( adc.getADC(i,value) != SPIERROR)
                    {
                        qDebug() << "One By One adc " << i << " Value " << value;
                    }
                }

              //  red , green, yellow , blue, magenta , cyan ,white

                qDebug() << "LED =  red ";
                adc.setLedCondition(SPIW::DAQC2Plate::red  );
                sleep(2);
                qDebug() << "LED =  blue ";
                adc.setLedCondition(SPIW::DAQC2Plate::blue  );

                sleep(2);
                qDebug() << "LED =  green ";
                adc.setLedCondition(SPIW::DAQC2Plate::green  );

                sleep(2);
                qDebug() << "LED =  yellow ";
                adc.setLedCondition(SPIW::DAQC2Plate::yellow  );

                sleep(2);
                qDebug() << "LED =  cyan ";
                adc.setLedCondition(SPIW::DAQC2Plate::cyan );

                sleep(2);
                qDebug() << "LED =  magenta ";
                adc.setLedCondition(SPIW::DAQC2Plate::magenta  );

                sleep(2);
                qDebug() << "LED =  white ";
                adc.setLedCondition(SPIW::DAQC2Plate::white );

                sleep(2);
                qDebug() << "LED =  off ";
                adc.setLedCondition(SPIW::DAQC2Plate::off );

                for ( int i = 0; i < 8; ++i)
                {
                    qDebug() << "Setting Bit " << i << "High";
                    adc.setBit(i,STATE_ON);
                    sleep(2);
                    qDebug() << "Setting Bit " << i << "Low";
                    adc.setBit(i,STATE_OFF);
                    sleep(2);
                }

                for ( int i = 0; i < 8; ++i)
                {
                    int pin;
                    adc.getBit(i,pin);
                    qDebug() << "getting Bit " << i << "it is " << pin;
                }

            }
        }
    }

    for ( int adr = 24; adr < 24 +8; ++adr )
    {
        if( SPIW::RELAYPlate::isRelayValid(adr ) )
        {
            qDebug() << "FOUND RELAY CARD at ADDRESS " << adr;
            SPIW::RELAYPlate relay(adr);
            if( relay.ValidBoard())
            {
                qDebug() << relay.getFWRevision() << "   " << relay.getHWRevision() << relay.getID();
            }

            for ( int i = 0; i < 8; ++i)
            {
                qDebug() << "Setting Bit RELAY " << i << "High";
                relay.setBit(i,STATE_ON);
                sleep(2);
                qDebug() << "Setting Bit  RELAY" << i << "Low";
                relay.setBit(i,STATE_OFF);
                sleep(2);

            }
        }



    }
    return 0;

}

