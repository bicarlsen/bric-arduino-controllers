// ********************* Define I/O ***************************

// pins utilisées pour le SPI
#define PIN_SCLK      13
#define PIN_MISO      12
#define PIN_MOSI      11

// Pins utilisées pour chip select
#define PIN_CS_ADC  9
#define PIN_CS_DAC  10

// Pins pour local/remote, mode, range
#define PIN_LOCAL_REMOTE    7
#define LR_REMOTE HIGH
#define LR_LOCAL LOW
#define PIN_MODE_SELECT   6
#define PIN_RANGE_SELECT  5
#define PIN_MODE_SET   4
#define PIN_RANGE_SET  3

// ********************** CONSTANTES **************************
const long ADC_offset = 18; //zero offset in bits
const float ADC_gain = 0.00007731628; //5.067V divided by 2^16
const long DAC_offset = 15; //zero offset in bits, this takes the 10Mohm pull down to -5V into account
const float DAC_gain = 0.00007716369628; //5.057V divided by 2^16  this takes the 10Mohm pull-down to -5V into account
//5V setpoint = 4.3825A based on 50mohm current sense resistor and 2.4K / 110 ohm divider
const float VOLTS_PER_AMP_CUR = 1.135909; //constant for current mode
const float AMPS_PER_VOLT_HI = 0.002; //2mA per Volt in high-light mode (500 ohm transimpedance)
const float VOLTS_PER_AMP_HI = 500.;
const float AMPS_PER_VOLT_LO = 0.000019678; //51k transimpedance for low-light mode
const float VOLTS_PER_AMP_LO = 51000.;

// ****************** FONCTIONS ***************************
void serialEvent();
int HandleSource(struct SCPI_Command cmd);
