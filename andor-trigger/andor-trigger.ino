/**
 * External trigger source for Andor spectrometer.
 * 
 * Author: Brian Carlsen
 * Contact: carlsen.bri@gmail.com
 * Date: 08-2019
 */

 /**
  * API
  * 
  * trigger( camera ): Triggers the given camera.
  *   Values are [ 'idus', 'ingaas' ].
  * 
  */


#include <ArduinoJson.h>

char TERMINATION_CHAR = '\n';

// pin setup
int IDUS    = 12;
int INGAAS  = 13;

void setup() {
  // put your setup code here, to run once:
  Serial.setTimeout( 5000 );
  Serial.begin( 9600 );

  pinMode( IDUS,    OUTPUT );
  pinMode( INGAAS,  OUTPUT );

  digitalWrite( IDUS,   LOW  );
  digitalWrite( INGAAS, HIGH ); 
}

void loop() {
}

void serialEvent() {
  String actionStr = Serial.readStringUntil( TERMINATION_CHAR );
  actionStr.trim();

  if ( actionStr == "init" ) {
    Serial.println( actionStr );
  }
  else {
    runAction( actionStr );
  }
}


//--- HELPER FUNCTIONS ---

//--- utilities ---
int str2int( String str, int len = 10 ) {
  char buff[ len ];
  str.toCharArray( buff, len );
  return atol( buff );
}


//--- COMMANDS ---

/**
 * Runs an action command sent to the controller.
 * 
 * \param action {String} The action command to run.
 *        Must take the form of run[<command>, <param 1>, ...]
 */
int runAction( String action ) {
  action.trim();
  String elements[ 10 ];
  int actionSize = parseAction( action, elements );
  String command = elements[ 0 ];
  String response = "";
  boolean success = true;
  int id = -1;

  // commands
   if ( actionSize < 0 ) {
    // invalid
    success = false;
  }
  else if ( command == "termination_char" ) {
    response = TERMINATION_CHAR;
  }
  else if ( command == "echo" && actionSize == 2 ) {
    response = elements[ 1 ];
  }
  else if ( command == "trigger" && actionSize == 2 ) {
    int pin;
    if( elements[ 1 ] == "idus" ) {
        pin = IDUS;
        success = true;
    }
    else if ( elements[ 1 ] == "ingaas" ) {
        pin = INGAAS;
        success = true;
    }
    else {
        success = false;
    }

    if ( success ) {
      id = trigger( pin ); 
    }
  }
  else {
    // invalid command
    success = false;
  }

  String stat = "error";
  if ( success ) {
    stat = "success";
  }
  
  // send response
  const size_t respBufferSize = JSON_OBJECT_SIZE( 4 ) *2 + 100;
  StaticJsonDocument< respBufferSize > resp;

  resp[ "status" ]    = stat;
  resp[ "command" ]   = action;

  if ( id > -1 ) {
    resp[ "id" ] = id;
  }
  if ( response.length() > 0 ) {
    resp[ "response" ] = response;
  }
  
  serializeJson( resp, Serial );
  Serial.println();

  return id;
}

/**
 * Parses an action command sent to the controller.
 * 
 * \param action {String} The action command to parse.
 *        Must take the form of run[<command>, <param 1>, ...]
 *        
 * \param elements {String[]} The array in which to store the action elements.
 * 
 * \returns {int} The size number of elements included in the action.
 *          Returns -1 on error.
 */
int parseAction( String action, String elements[] ) {  
  action.trim();
    
  // validate run[ ... ] format
  if ( action.indexOf( "run[" ) != 0 || action.charAt( action.length() -1 ) != ']' ) {
    // invalid
    return -1;
  } 

  // get delimeter positions
  int start = 4; // start of command string, after "run["
  int delimeters[ 20 ];
  int nDels = 0;
  int index;
  do {
    index = action.indexOf( ",", start );
    if ( index != -1 ) {
      // delimeter found
      delimeters[ nDels ] = index; // save position
      start = index + 1; // start from next position
      ++nDels;
    }
  } while( index != -1 );

  // add end of action delimeter
  delimeters[ nDels ] = action.length() - 1;

  // get elements
  String temp;
  start = 4;
  for ( index = 0; index <= nDels; index++ ) { 
    // get substring of element
    temp = action.substring( start, delimeters[ index ] ); 
    temp.trim(); 
    elements[ index ] = temp; 

    // start at next position
    start = delimeters[ index ] + 1; 
  }
  
  return ( nDels + 1 );
}

//--- ACTIONS ---

/**
 * Sends a trigger signal to the given pin.
 * 
 * \param channel {int} The pin to trigger.
 * 
 * \returns {int} Returns the triggered pin.
 */
int trigger( int pin ) {
  digitalWrite( pin, HIGH );
  delay( 250 );
  digitalWrite( pin, LOW );

  return pin;
}
