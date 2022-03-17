#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SPI.h>
#ifndef PARSE_SCPI_H
#include "Parse_SCPI.h"
#define PARSE_SCPI_H
#endif
#include "AD7680.h"
#include "AD5662.h"
#include "LED_Controller_Standalone.h"
#include <Arduino.h>

//************************ DEFINE ******************************

#define SIZE_COMMAND 128

// String serial monitor
char receivedChars[SIZE_COMMAND]="";
char replyChars[SIZE_COMMAND]="";
String ReceivedStr= "";
byte terminalEcho = 0;
byte stringComplete = false;

struct SCPI_Command cmd_struct;

int i, j, k, ii, jj, kk;
float tempFloat = 0.;
char str_temp[8];
int loc_rem_temp = 0;

//mode, range, and local/remote settings
byte regulation_mode_reg = 0; //regulation mode: 0 = constant light mode, 1 = constant current
byte photodiode_range_reg = 0; //photodiode range: 0 = low light (0-100uA), 1 = high light (0 - 10mA)
float setpoint_reg = 0.; //the current setpoint, in amps
byte local_remote = 0; // 0 = remote, 1 = local

// ****************** OBJECTS ************************
AD7680 ADCx;
AD5662 DAC;

//************************************************************
//********************* SETUP ********************************
//************************************************************
void setup() 
{

  //setup mode, range, and local/remote pins
  pinMode(PIN_LOCAL_REMOTE,INPUT_PULLUP);
  pinMode(PIN_MODE_SELECT,INPUT_PULLUP);
  pinMode(PIN_RANGE_SELECT,INPUT_PULLUP);
  pinMode(PIN_MODE_SET,OUTPUT);
  digitalWrite(PIN_MODE_SET,HIGH);  
  pinMode(PIN_RANGE_SET,OUTPUT);
  digitalWrite(PIN_RANGE_SET,HIGH);  

  Serial.begin(115200);
  SPI.begin();

  //initialise the DAC and ADC
    ADCx.init(PIN_CS_ADC);
    ADCx.setOffset(ADC_offset);
    ADCx.setGain(ADC_gain);
    ADCx.setSPISpeed(500000);
    DAC.init(PIN_CS_DAC);
    DAC.setOffset(DAC_offset);
    DAC.setGain(DAC_gain);
    DAC.write(0.); 

  jj=0;
  delay(100);
}

//************************************************************
//********************* LOOP *********************************
//************************************************************
void loop() 
{
  //read local/remote input and apply mode and range settings accordingly
  local_remote = digitalRead(PIN_LOCAL_REMOTE);
  if(local_remote) { //1 = remote, apply the values in the registers
    digitalWrite(PIN_MODE_SET,regulation_mode_reg);
    digitalWrite(PIN_RANGE_SET,photodiode_range_reg);
  }else { //0 = local, apply the values from the manual switches
    digitalWrite(PIN_MODE_SET,digitalRead(PIN_MODE_SELECT));
    digitalWrite(PIN_RANGE_SET,digitalRead(PIN_RANGE_SELECT));
  }
  
  //RS232 interface
  if(stringComplete == true) 
  {
    if(terminalEcho) Serial.print("\r\n");
    if(Parse_Command(receivedChars, &cmd_struct) == 0) HandleSource(cmd_struct);
    //clear the values in the command struct
    cmd_struct.type = CTBLANK;
    cmd_struct.subType = CSTBLANK;
    cmd_struct.RW = RWBLANK;
    cmd_struct.data = 0.;
  
    Serial.write(replyChars);
    //clear the buffer
    for(i=0; i<SIZE_COMMAND; i++) receivedChars[i] = '\0';
    for(i=0; i<SIZE_COMMAND; i++) replyChars[i] = '\0';
    jj=0;
    stringComplete = false;
  }

}


//************************************************************
//********************* FUNCTION *****************************
//************************************************************

void serialEvent() 
{
  while (Serial.available() > 0)
 {
    // get the new byte:
    char inChar = (char)Serial.read();
    if(terminalEcho) Serial.print(inChar);
    //Serial.print(inChar);
    // add it to the inputString:
    
    if(jj < SIZE_COMMAND) {
      receivedChars[jj] = inChar;
      jj++;
      receivedChars[jj] = '\0';
    }else {//buffer is full.  trash it
      for(i=0; i<SIZE_COMMAND; i++) receivedChars[i] = '\0';
      jj=0;
    }
    // if the incoming character is a newline then the command is complete.
    // set a flag so we know to pass it to the parse
    if (inChar == '\n' || inChar == '\r') {
      stringComplete = true;
      //Serial.print("Got String!\n\r");
    }
 } 
}


int HandleSource(SCPI_Command cmd)
{
  //float outVal = 0.;

  switch(cmd.type) {
    case IDN :
      //Serial.println("EPFL SB ISIC-GE AECH, Precision LED Driver V1.2");
      sprintf(replyChars,"EPFL SB ISIC-GE AECH, Precision LED Driver V1.2\n\r");
      break;
    case ECHO : 
      terminalEcho = cmd.data;
      break;
    case SOURCE : 
      switch(cmd.subType) {
        case VOLTAGE :
          dtostrf(cmd.data,3,6,str_temp);
          sprintf(replyChars,"SOUR:VOLT %sV\r\n",str_temp);
          //set control voltage directly, with no conversion factor
          //this mode only expected to be used for calibration
          DAC.write(cmd.data);
          break;
        case CURRENT : //apply current setpoint and save to register
          dtostrf(cmd.data,3,6,str_temp);
          sprintf(replyChars,"SOUR:CURR %sA\r\n",str_temp);
          setpoint_reg = cmd.data;
          //set DAC based on current mode/range settings
          if(regulation_mode_reg == REGULATION_MODE_CURRENT) { //0 = constant current mode
            DAC.write(cmd.data*VOLTS_PER_AMP_CUR);
          }else { //0 = constant light mode
            if(photodiode_range_reg == PHOTODIODE_RANGE_HIGH) { //0 = high light mode
              DAC.write(cmd.data*VOLTS_PER_AMP_HI);
            }else { //0 = low light mode
              DAC.write(cmd.data*VOLTS_PER_AMP_LO);
            }
          }
          break;
        default :
          //bad value
          //Serial.print("Err01\r\n");
          sprintf(replyChars,"Err01\r\n");
          
      }
      break;
    case MEASURE : 
      switch(cmd.subType) {
        case VOLTAGE :
          dtostrf(ADCx.read_50Hz_reject(),7,5,str_temp);
          //dtostrf(ADCx.read(),7,5,str_temp);
          sprintf(replyChars,"MEAS:VOLT %sV\r\n",str_temp);
          break;
        case CURRENT :
          tempFloat = ADCx.read_50Hz_reject();
          loc_rem_temp = digitalRead(PIN_LOCAL_REMOTE);
          if((photodiode_range_reg == PHOTODIODE_RANGE_HIGH && loc_rem_temp == 1) ||
             (digitalRead(PIN_LOCAL_REMOTE) == LR_LOCAL && digitalRead(PIN_RANGE_SELECT) == PHOTODIODE_RANGE_HIGH)) {
            tempFloat = tempFloat*AMPS_PER_VOLT_HI;
            tempFloat = tempFloat*1000.;
            dtostrf(tempFloat,6,4,str_temp);
            sprintf(replyChars,"MEAS:CURR %smA\r\n",str_temp);
          }
          else {
            tempFloat = tempFloat*AMPS_PER_VOLT_LO;
            tempFloat = tempFloat*1000000.;
            dtostrf(tempFloat,6,3,str_temp);
            sprintf(replyChars,"MEAS:CURR %suA\r\n",str_temp);
          }
          break;
        default :
          //bad value
          //Serial.print("Err02\r\n");
          sprintf(replyChars,"Err02\r\n");
          break;
      }
      break;
    case CHANNEL : 
      switch(cmd.subType) {
        case LOCAL :
          //read only
          if(digitalRead(PIN_LOCAL_REMOTE)) sprintf(replyChars,"CHAN:LOCA REMOTE\r\n"); //Serial.print("Local\r\n");
          else sprintf(replyChars,"CHAN:LOCA LOCAL\r\n"); //Serial.print("Remote\r\n");
          break;
        case MODE :
          switch(cmd.RW) {
              case READ :
                if(digitalRead(PIN_LOCAL_REMOTE) == LOW) { //local
                  if(digitalRead(PIN_MODE_SELECT) == HIGH) sprintf(replyChars,"CHAN:MODE CURRENT\r\n");
                  else sprintf(replyChars,"CHAN:MODE LIGHT\r\n");
                }else { //remote
                  if(regulation_mode_reg == REGULATION_MODE_CURRENT) sprintf(replyChars,"CHAN:MODE CURRENT\r\n");
                  else sprintf(replyChars,"CHAN:MODE LIGHT\r\n");
                }
                break;
              case WRITE : 
                if(cmd.data == REGULATION_MODE_CURRENT) { //current mode
                  digitalWrite(PIN_MODE_SET,HIGH);
                  regulation_mode_reg = REGULATION_MODE_CURRENT;
                  sprintf(replyChars,"CHAN:MODE CURRENT\r\n");
                }else if(cmd.data == REGULATION_MODE_LIGHT) {
                  digitalWrite(PIN_MODE_SET,LOW);
                  regulation_mode_reg = REGULATION_MODE_LIGHT;
                  sprintf(replyChars,"CHAN:MODE LIGHT\r\n");
                }else sprintf(replyChars,"Err23\r\n");
                break;
              default :  
                sprintf(replyChars,"Err03\r\n");
                break;
           }
           break;
        case RANGE :
          switch(cmd.RW) {
              case READ :
                if(digitalRead(PIN_LOCAL_REMOTE) == LOW) { //local
                  if(digitalRead(PIN_RANGE_SELECT) == HIGH) sprintf(replyChars,"CHAN:RANG HIGH\r\n");
                  else sprintf(replyChars,"CHAN:RANG LOW\r\n");
                }else { //remote
                  if(photodiode_range_reg == PHOTODIODE_RANGE_HIGH) sprintf(replyChars,"CHAN:RANG HIGH\r\n");
                  else sprintf(replyChars,"CHAN:RANG LOW\r\n");
                }
                break;
              case WRITE : 
                if(cmd.data == PHOTODIODE_RANGE_HIGH) { //high range
                  //if in constant light mode, change setpoint first, then apply range
                  if(regulation_mode_reg == REGULATION_MODE_LIGHT) {
                    DAC.write(setpoint_reg*VOLTS_PER_AMP_HI);
                  }
                  digitalWrite(PIN_RANGE_SET,HIGH);
                  photodiode_range_reg = 1;
                  sprintf(replyChars,"CHAN:RANG HIGH\r\n");
                }else if(cmd.data == PHOTODIODE_RANGE_LOW) {
                  //if in constant light mode, change range first, then apply setpoint
                  digitalWrite(PIN_RANGE_SET,LOW);
                  photodiode_range_reg = 0;
                  sprintf(replyChars,"CHAN:RANG LOW\r\n");
                  if(regulation_mode_reg == REGULATION_MODE_LIGHT) {
                    DAC.write(setpoint_reg*VOLTS_PER_AMP_LO);
                  }
                }else sprintf(replyChars,"Err24\r\n");
                break;
              default :  
                sprintf(replyChars,"Err04\r\n");
                break;
           }
           break;
        default :
          //CHANNEL sub type bad value
          //Serial.print("Err05\r\n");
          sprintf(replyChars,"Err05\r\n");
          break;
      }
      break;
    default :
      //cmd type bad value
      //Serial.print("Err06\r\n");
      sprintf(replyChars,"Err06\r\n");
      break;
  }
 }
