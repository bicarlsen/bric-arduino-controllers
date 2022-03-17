
enum CommandType {CTBLANK,IDN,ECHO,SOURCE,MEASURE,CHANNEL};
enum CommandSubType {CSTBLANK,VOLTAGE,CURRENT,SELECT,LOCAL,ENABLE,ALL,MODE,RANGE};
enum ReadWrite {RWBLANK,WRITE,READ};

#define REGULATION_MODE_CURRENT 1
#define REGULATION_MODE_LIGHT 0
#define PHOTODIODE_RANGE_HIGH 1
#define PHOTODIODE_RANGE_LOW 0

struct SCPI_Command {
  CommandType type;
  CommandSubType subType;
  ReadWrite RW;
  float data;
};

int Parse_Command(char *str, struct SCPI_Command *cmd);
float parseNumeric(char *numeric);
