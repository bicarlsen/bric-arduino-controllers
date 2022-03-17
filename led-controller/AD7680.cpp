#include <SPI.h>
#include "AD7680.h"

//#define ARDUINO  //already defined by IDE
//#define ST_DISCOVERY

void AD7680::init(int CS_pin, int A0_pin, int A1_pin, byte Addr)
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

void AD7680::select()
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

float AD7680::read()
{
	
  byte data_byte = 0;
  byte data_byte1 = 0;
  byte data_byte2 = 0;
  byte data_byte3 = 0;
  long data_word = 0L;

#ifdef ARDUINO
  SPI.beginTransaction(SPISettings(_SPI_Speed, MSBFIRST, SPI_MODE2));  
  digitalWrite(_CS_pin,LOW);
  data_byte1 = SPI.transfer(0x00);
  data_byte2 = SPI.transfer(0x00);
  data_byte3 = SPI.transfer(0x00);
  digitalWrite(_CS_pin,HIGH);
  SPI.endTransaction();
#endif
#ifdef ST_DISCOVERY

#endif
  
  data_word = data_byte1;
  data_word = data_word << 8;
  data_word = data_word | data_byte2;
  data_word = data_word << 4;
  data_word = data_word | (data_byte3 >> 4);

  //apply offset
  data_word = data_word - _offset;
  
  return (float)data_word*_gain;
	
}

float AD7680::read_50Hz_reject()
{
  
  byte data_byte = 0;
  byte data_byte1 = 0;
  byte data_byte2 = 0;
  byte data_byte3 = 0;
  long data_word = 0L;
  long data_word_average = 0L;
  int ii=0;

#ifdef ARDUINO
  SPI.beginTransaction(SPISettings(_SPI_Speed, MSBFIRST, SPI_MODE2));  

  for(ii=0; ii< 200; ii++) {
    digitalWrite(_CS_pin,LOW);
    data_byte1 = SPI.transfer(0x00);
    data_byte2 = SPI.transfer(0x00);
    data_byte3 = SPI.transfer(0x00);
    digitalWrite(_CS_pin,HIGH);
  
    data_word = data_byte1;
    data_word = data_word << 8;
    data_word = data_word | data_byte2;
    data_word = data_word << 4;
    data_word = data_word | (data_byte3 >> 4);
  
    //apply offset
    data_word = data_word - _offset;
    data_word_average += data_word;
    //delay for 100us minus processing time of above
    delayMicroseconds(39);
  }
  
  SPI.endTransaction();
#endif
#ifdef ST_DISCOVERY

#endif
  data_word_average = data_word_average/200;
  
  return (float)data_word_average*_gain;
  
}
