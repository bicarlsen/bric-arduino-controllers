#ifndef PARSE_SCPI_H
#include "Parse_SCPI.h"
#define PARSE_SCPI_H
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SPI.h>

int Parse_Command(char *cmdString, struct SCPI_Command *cmd) {
  char *token;
  char cmdStringLocal[128];  
  
  //les commandes prennent la forme suivante:
  //*IDN?
  //ECHO <ON|OFF>
  //:SOURce:VOLTAage 12.34mV
  //:SOURce:CURRent 0.123mA
  //:MEASure:VOLTage
  //:MEASure:CURRent
  //:CHANnel:SELEct ?|1,2,...,N
  //:CHANnel:LOCAl ?
  //:CHANnel:ENABle ?<ON|OFF>

  //parse rempli une structure comme suit:
  /*
   enum CommandType {IDN,ECHO,SOURCE,MEASURE,CHANNEL};
enum CommandSubType {VOLTAGE,CURRENT,SELECT,LOCAL,ENABLE};
enum readWrite {WRITE,READ}

typedef struct SCPI_Command {
  CommandType type,
  CommandSubType subType,
  readWrite RW,
  float data
}
   */
  //make a copy for strtok to work on
  if(strlen(cmdString) <= 128) strcpy(cmdStringLocal,cmdString);

  if(cmdString[0] == '*') { //*IDN? ou autre
    token = strtok(cmdStringLocal," :\n\r");
    if(!strcmp(token,"*IDN?")) cmd->type = IDN;
  }else if(!strcmp(strtok(cmdStringLocal," :\n\r"),"ECHO")) {
    cmd->type = ECHO;
    token = strtok(NULL," :\n\r");
    if(!strcmp(token,"ON")) cmd->data = 1;
    if(!strcmp(token,"OFF")) cmd->data = 0;
    //Serial.print("i was here...\n");
  }else if(cmdString[0] == ':') {
    //copy again as above use of strtok overwrites the previous copy
    if(strlen(cmdString) <= 128) strcpy(cmdStringLocal,&cmdString[1]);
    token = strtok(cmdStringLocal," :\n\r"); //takes leading : off as well
    //devrait être SOURce, SOUR, MEASure, MEAS, CHANnel, ou CHAN
    
    if(!strcmp(token,"SOURce") || !strcmp(token,"SOUR")) {
      cmd->type = SOURCE;
      token = strtok(NULL," :\n\r");
      if(!strcmp(token,"VOLTage") || !strcmp(token,"VOLT")) {
        cmd->subType = VOLTAGE;
        //la partie numérique
        token = strtok(NULL,"\n\r");
        cmd->data = parseNumeric(token);
      }else if(!strcmp(token,"CURRent") || !strcmp(token,"CURR")) {
        cmd->subType = CURRENT;
        //la partie numérique
        token = strtok(NULL,"\n\r");
        cmd->data = parseNumeric(token);
      }else return 1;
    }else if(!strcmp(token,"MEASure") || !strcmp(token,"MEAS")) {
      cmd->type = MEASURE;
      token = strtok(NULL," :\n\r");
      if(!strcmp(token,"VOLTage") || !strcmp(token,"VOLT")) {
        cmd->subType = VOLTAGE;
      }else if(!strcmp(token,"CURRent") || !strcmp(token,"CURR")) {
        cmd->subType = CURRENT;
      }else if(!strcmp(token,"ALL")) {
        cmd->subType = ALL;
        token = strtok(NULL,"\n\r");
        cmd->data = atoi(token);
      }else return 1;
    }else if(!strcmp(token,"CHANnel") || !strcmp(token,"CHAN")) {
      cmd->type = CHANNEL;
      token = strtok(NULL," :\n\r");
      if(!strcmp(token,"SELEct") || !strcmp(token,"SELE")) {
        cmd->subType = SELECT;
        token = strtok(NULL,"\n\r");
        if(!strcmp(token,"?")) cmd->RW = READ;
        else {
          //nb: atoi retourne zero en cas d'erreur
          cmd->data = atoi(token);
          cmd->RW = WRITE;
        }
      }else if(!strcmp(token,"LOCAl") || !strcmp(token,"LOCA")) {
        //no options, read only
        cmd->subType = LOCAL;
      }else if(!strcmp(token,"ENABle") || !strcmp(token,"ENAB")) {
        cmd->subType = ENABLE;  
        token = strtok(NULL,"\n\r");
        if(!strcmp(token,"?")) cmd->RW = READ;
        else {
          cmd->RW = WRITE;
          if(!strcmp(token,"ON")) cmd->data = 1;
          else if(!strcmp(token,"OFF")) cmd->data = 0;
          else return 1;
        }
      }else if(!strcmp(token,"MODE")) {
        cmd->subType = MODE;
        token = strtok(NULL,"\n\r");
        if(!strcmp(token,"?")) cmd->RW = READ;
        else {
          //nb: atoi retourne zero en cas d'erreur
          cmd->RW = WRITE;
          if(!strcmp(token,"CURRENT") || !strcmp(token,"current")) cmd->data = REGULATION_MODE_CURRENT;
          else if(!strcmp(token,"LIGHT") || !strcmp(token,"light")) cmd->data = REGULATION_MODE_LIGHT;
          else cmd->data = -1;
        }
      }else if(!strcmp(token,"RANGe") || !strcmp(token,"RANG")) {
        cmd->subType = RANGE;
        token = strtok(NULL,"\n\r");
        if(!strcmp(token,"?")) cmd->RW = READ;
        else {
          //nb: atoi retourne zero en cas d'erreur
          cmd->RW = WRITE;
          if(!strcmp(token,"HIGH") || !strcmp(token,"high")) cmd->data = PHOTODIODE_RANGE_HIGH;
          else if(!strcmp(token,"LOW") || !strcmp(token,"low")) cmd->data = PHOTODIODE_RANGE_LOW;
          else cmd->data = -1;
        }
      }else return 1;
    }else return 1;
  }else return 1;

  return 0;
}

float parseNumeric(char *numeric) {
  char *token;
  char numericLocal[16];
  //make a copy for strtok
  if(strlen(numeric) <= 16) strcpy(numericLocal,numeric);
	int i;
  token = strtok(numericLocal," :\n\rnumV");
  float setVal = atof(token);
  if(abs(setVal) < 1e-12) { //atof retourne 0.0 comme erreur, alors assurer que les zeros sont vrai.
    for(i=0; i<strlen(token); i++) { //il ne devrait pas avoir d'autres caractères pour un vrai zero
      if(token[i] != '0' && token[i] != '.' && token[i] != '+' && token[i] != '-' && token[i] != 'e' && token[i] != '\0') return -99999.9;
    }
  }
  //les unités...
  //strtok enlève le valeur qui nous concerne alors il faut le chercher autrement...
  char unitchar;
  unitchar = numeric[strlen(token)];
  if(unitchar=='m') setVal *= 1e-3;
  if(unitchar=='n') setVal *= 1e-9;
  if(unitchar=='u') setVal *= 1e-6;

  return setVal;
}
