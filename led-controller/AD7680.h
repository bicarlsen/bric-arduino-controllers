class AD7680
{
	private:
	long _offset;
	float _gain;
	int _CS_pin;
	int _A0_pin;
	int _A1_pin;
	byte _Addr;
	long _SPI_Speed = 1000000;
	
	public:
	void init(int CS_pin, int A0_pin = 0, int A1_pin = 0, byte Addr = 0);
	void select();
	float read();
  float read_50Hz_reject();
	void setOffset(long offset) {_offset = offset;} //zero offset in bit
	void setGain(float gain) {_gain = gain;}  //gain in V/bit
	void setSPISpeed(long SPI_Speed) {_SPI_Speed = SPI_Speed;}
};
