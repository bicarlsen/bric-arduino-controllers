#include <SPI.h>
#include "AD5662.h"

//#define ARDUINO  //already defined by IDE
//#define ST_DISCOVERY

void AD5662::init(int CS_pin, int A0_pin, int A1_pin, byte Addr)
{
	_CS_pin = CS_pin;
	_A0_pin = A0_pin;
	_A1_pin = A1_pin;
	_Addr = Addr;
#ifdef ARDUINO
	pinMode(_CS_pin, OUTPUT);
	digitalWrite(_CS_pin, HIGH);
#endif
#ifdef ST_DISCOVERY

#endif
	if(A0_pin > 0) {
#ifdef ARDUINO
		pinMode(_A0_pin, OUTPUT);
		digitalWrite(_A0_pin, (byte)_Addr&0x01);
#endif
#ifdef ST_DISCOVERY

#endif
	}
	if(A1_pin > 0) {
#ifdef ARDUINO
		pinMode(_A1_pin, OUTPUT);
		digitalWrite(_A1_pin, (byte)_Addr&0x02);
#endif
#ifdef ST_DISCOVERY

#endif
	}
	
}

void AD5662::select()
{
	if(_A0_pin > 0) {
#ifdef ARDUINO
    digitalWrite(_A0_pin, (byte)_Addr&0x01);
#endif
#ifdef ST_DISCOVERY

#endif
	}
	if(_A1_pin > 0) {
#ifdef ARDUINO
    digitalWrite(_A1_pin, (byte)_Addr&0x02);
#endif
#ifdef ST_DISCOVERY

#endif
	}
}

int AD5662::write(float setPoint)
{
	setPoint = setPoint/_gain;
	setPoint = setPoint + _offset;
	unsigned int DAC_word; 
	if(setPoint < 0.) setPoint = 0;
	else if(setPoint > 65535) setPoint = 65535; 
	DAC_word = (unsigned int)setPoint;
	

#ifdef ARDUINO
	SPI.beginTransaction(SPISettings(_SPI_Speed, MSBFIRST, SPI_MODE2));  
	digitalWrite(_CS_pin,LOW);
	SPI.transfer(0x00);                 //Normal opération
	SPI.transfer((byte)(DAC_word>>8));     //MSB
	SPI.transfer((byte)(DAC_word&0x00FF)); //LSB
	digitalWrite(_CS_pin,HIGH);
	SPI.endTransaction();
#endif
#ifdef ST_DISCOVERY

#endif
 
  return 0;
	
}
