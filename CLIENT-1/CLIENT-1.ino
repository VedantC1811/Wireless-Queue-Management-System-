/*
  This code is for Client 1 of the Wireless Queue Management System.
  It uses an ESP8266 to receive data from an IR remote and communicates
  with the ESP32 server to send the combined value (queue token).
  A second endpoint (/getCounter1) is used for Client 1 to send its data.

  Components:
  1. ESP8266: Acts as the client, handling IR inputs and communicating with the server.
  2. EEPROM: Stores token numbers persistently between reboots.
  3. IR remote: Used to input token numbers.
  4. LCD Display: Shows the current token and connection status.
  5. HTTPClient: Sends the combined token value to the ESP32 server at /getCounter1.

  // The code now sends data to the /getCounter1 endpoint on the ESP32 server
  // (http://192.168.4.1/getCounter1?value=<token_value>).
 // The logic remains the same as the original client (Client 1), but the endpoint
 // for the HTTP request is changed to match Client 1's needs.

  */
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <ESP8266HTTPClient.h>
#include <LiquidCrystal_I2C.h>



LiquidCrystal_I2C lcd(0x27, 16, 2); // Initialize the LCD display

//const char *wifiSSID = "TP-Link_91F4"; // WiFi SSID
//const char *wifiPassword = "27296936";// WiFi password


//const char *wifiSSID = "DIR-825";
//const char *wifiPassword = "88728278";

const char *wifiSSID = "ESP32";
const char *wifiPassword = "12345678";

const char *serverAddress = "192.168.4.1"; // Server address

const int RECV_PIN = 16;
String combinedValue;
const int statusled = 5;
const int buzz = 0;

WiFiClient client;

enum State {
  IDLE,
  RECEIVING_VALUE_3,
  RECEIVING_VALUE_2,
  RECEIVING_VALUE_1
};

State currentState = IDLE;
IRrecv irReceiver(RECV_PIN);
decode_results results;

enum Buttons {
  NONE,
  BUTTON_0,
  BUTTON_1,
  BUTTON_2,
  BUTTON_3,
  BUTTON_4,
  BUTTON_5,
  BUTTON_6,
  BUTTON_7,
  BUTTON_8,
  BUTTON_9,
  BUTTON_NEXT,
  BUTTON_PREV,
  BUTTON_CLEAR,
  BUTTON_SEND
};

struct code_button_pair {
  unsigned long code;
  int button;
};

const code_button_pair code_mapping[] = {
  {0xFF6897, BUTTON_0},
  {0xFF30CF, BUTTON_1},
  {0xFF18E7, BUTTON_2},
  {0xFF7A85, BUTTON_3},
  {0xFF10EF, BUTTON_4},
  {0xFF38C7, BUTTON_5},
  {0xFF5AA5, BUTTON_6},
  {0xFF42BD, BUTTON_7},
  {0xFF4AB5, BUTTON_8},
  {0xFF52AD, BUTTON_9},
  {0xFFC23D, BUTTON_NEXT},
  {0xFF22DD, BUTTON_PREV},
  {0xFF906F, BUTTON_NEXT},
  {0xFFE01F, BUTTON_PREV},
  {0xFFA25D, BUTTON_CLEAR},
  {0xFF9867, BUTTON_SEND}
};

int button = NONE;
String value1 = " ";
String value2 = " ";
String value3 = " ";
String tempVal;

const int EEPROM_ADDRESS_1 = 0;
const int EEPROM_ADDRESS_2 = 4;
const int EEPROM_ADDRESS_3 = 8;
const int EEPROM_ADDRESS_COMBINED = 12;

void EEPROMWriteLong(int address, long value) {
  for (int i = 0; i < sizeof(value); ++i) {
    EEPROM.write(address + i, (value >> (8 * i)) & 0xFF);
  }
}

long EEPROMReadLong(int address) {
  long value;
  for (int i = 0; i < sizeof(value); ++i) {
    value |= (static_cast<long>(EEPROM.read(address + i)) << (8 * i));
  }
  return value;
}

void sendValueToServer(String val) {
  WiFiClient client;
  HTTPClient httpClient;
  int value = val.toInt();
  httpClient.begin(client, String("http://") + serverAddress + "/getCounter1?value=" + String(value));

  int httpCode = httpClient.GET();

  if (httpCode > 0) {
    Serial.println("Value sent to server: " + String(value));
    EEPROMWriteLong(EEPROM_ADDRESS_COMBINED,value);
    
       delay(1000);
       digitalWrite(buzz, HIGH);
       delay(500);
       digitalWrite(buzz, LOW);
       
       
  } else {
    //Serial.println("Error sending value to server");
  }

  httpClient.end();
}

void sendCombinedValueToServer() {

  combinedValue = value3 + value2 + value1;
  Serial.println("Combined value is " + combinedValue);
  sendValueToServer(combinedValue);
}

void updateState() {

  
  int code_mapping_size = sizeof(code_mapping) / sizeof(code_button_pair);
  for (int i = 0; i < code_mapping_size; ++i) {
    if (results.value == code_mapping[i].code) {
      int combValue = combinedValue.toInt();

      switch (code_mapping[i].button) {
        case BUTTON_0:
        case BUTTON_1:
        case BUTTON_2:
        case BUTTON_3:
        case BUTTON_4:
        case BUTTON_5:
        case BUTTON_6:
        case BUTTON_7:
        case BUTTON_8:
        case BUTTON_9:
         
          if (currentState == IDLE) {
            value1 = " ";
            value3 = " ";
            value3 = String(code_mapping[i].button - BUTTON_0);
            currentState = RECEIVING_VALUE_2;
            value2 = " ";
          } else if (currentState == RECEIVING_VALUE_2) {
            value2 = String(code_mapping[i].button - BUTTON_0);
            currentState = RECEIVING_VALUE_1;
            value1 = " ";
          } else if (currentState == RECEIVING_VALUE_1) {
            if (value1 == " ") {
              value1 = String(code_mapping[i].button - BUTTON_0);
            }
          }
          break;
        case BUTTON_NEXT:
          combinedValue = String(EEPROMReadLong(EEPROM_ADDRESS_COMBINED));
          combinedValue = String(combValue + 1);
          EEPROMWriteLong(EEPROM_ADDRESS_COMBINED, combinedValue.toInt());
           if (combinedValue.toInt() > 999) {
        combinedValue = "999";
    }
         // Serial.println("Combined value incremented: " + combinedValue);
          tempVal = combinedValue;
       value1 = " ";
       value2 = " ";
       value3 = " ";
       currentState = IDLE;
          break;

        case BUTTON_PREV:
          combinedValue = String(EEPROMReadLong(EEPROM_ADDRESS_COMBINED));
          combinedValue = String(combValue - 1);
           EEPROMWriteLong(EEPROM_ADDRESS_COMBINED, combinedValue.toInt());
          if (combinedValue.toInt() < 0) {
        combinedValue = "0";
    } 
         // Serial.println("Combined value decremented: " + combinedValue);
          tempVal = combinedValue;
       value1 = " ";
       value2 = " ";
       value3 = " ";
       currentState = IDLE;
          break;

           case BUTTON_SEND:
      sendCombinedValueToServer();

      
      //sendValueToServer(combinedValue);
       
      // Print EEPROM values before storing
      //Serial.print("EEPROM values before storing: ");
      //Serial.print("Value3: " + String(EEPROMReadLong(EEPROM_ADDRESS_3)) + " ");
      //Serial.print("Value2: " + String(EEPROMReadLong(EEPROM_ADDRESS_2)) + " ");
      //Serial.print("Value1: " + String(EEPROMReadLong(EEPROM_ADDRESS_1)) + " ");

      // Store the values in EEPROM
      EEPROMWriteLong(EEPROM_ADDRESS_COMBINED, value3.toInt() + value2.toInt() + value1.toInt());

      // Print EEPROM values after storing
      //Serial.print("EEPROM values after storing: ");
      //Serial.print("Value3: " + String(EEPROMReadLong(EEPROM_ADDRESS_3)) + " ");
      //Serial.print("Value2: " + String(EEPROMReadLong(EEPROM_ADDRESS_2)) + " ");
      //Serial.print("Value1: " + String(EEPROMReadLong(EEPROM_ADDRESS_1)) + " ");

      //Serial.println("Values stored in EEPROM: " + String(value3) + String(value2) + String(value1));
     
      
      currentState = IDLE; // Reset the state to IDLE

      break;
      }

      // Print EEPROM values before updating
      //Serial.print("EEPROM values before update: ");
      //Serial.print("Value3: " + String(EEPROMReadLong(EEPROM_ADDRESS_3)) + " ");
     // Serial.print("Value2: " + String(EEPROMReadLong(EEPROM_ADDRESS_2)) + " ");
     // Serial.print("Value1: " + String(EEPROMReadLong(EEPROM_ADDRESS_1)) + " ");

      EEPROMWriteLong(EEPROM_ADDRESS_1, value1.toInt());
      EEPROMWriteLong(EEPROM_ADDRESS_2, value2.toInt());
      EEPROMWriteLong(EEPROM_ADDRESS_3, value3.toInt());

      // Print EEPROM values after updating
     // Serial.print("EEPROM values after update: ");
     // Serial.print("Value3: " + String(EEPROMReadLong(EEPROM_ADDRESS_3)) + " ");
      //Serial.print("Value2: " + String(EEPROMReadLong(EEPROM_ADDRESS_2)) + " ");
      //Serial.print("Value1: " + String(EEPROMReadLong(EEPROM_ADDRESS_1)) + " ");

      button = code_mapping[i].button;
      if (button == BUTTON_NEXT || button == BUTTON_PREV ) {
    printLCD("TOKEN NO.=", combinedValue, WiFi.status() == WL_CONNECTED ? "Connected.." : "Connecting..", connectToServer() ? "Connected" : "Error...");
}
    }
  }
}

void decodeState() {
   int code_mapping_size = sizeof(code_mapping) / sizeof(code_button_pair);
  for (int i = 0; i < code_mapping_size; ++i) {
    if (results.value == code_mapping[i].code) {
  switch (code_mapping[i].button) {

    case BUTTON_NEXT:

    sendValueToServer(combinedValue);
     break;

    case BUTTON_PREV:

    sendValueToServer(combinedValue);
     break;
    
   
    case BUTTON_CLEAR:
      // Clear the current values
      value1 = " ";
      value2 = " ";
      value3 = " ";
      //sendValueToServer(combinedValue);

      // Print EEPROM values before clearing
      //Serial.print("EEPROM values before clearing: ");
      //Serial.print("Value3: " + String(EEPROMReadLong(EEPROM_ADDRESS_3)) + " ");
      //Serial.print("Value2: " + String(EEPROMReadLong(EEPROM_ADDRESS_2)) + " ");
      //Serial.print("Value1: " + String(EEPROMReadLong(EEPROM_ADDRESS_1)) + " ");

      EEPROMWriteLong(EEPROM_ADDRESS_1, 0);
      EEPROMWriteLong(EEPROM_ADDRESS_2, 0);
      EEPROMWriteLong(EEPROM_ADDRESS_3, 0);

      // Print EEPROM values after clearing
      //Serial.print("EEPROM values after clearing: ");
      //Serial.print("Value3: " + String(EEPROMReadLong(EEPROM_ADDRESS_3)) + " ");
      //Serial.print("Value2: " + String(EEPROMReadLong(EEPROM_ADDRESS_2)) + " ");
      //Serial.print("Value1: " + String(EEPROMReadLong(EEPROM_ADDRESS_1)) + " ");
      
      currentState = IDLE; // Reset the state to IDLE
      break;
  }
}
  }
}

bool connectToServer() {
  return client.connect(serverAddress, 80);
}

void setup() {
  EEPROM.begin(512);
  lcd.init();
  lcd.clear();
  lcd.backlight();

  pinMode(buzz, OUTPUT);  // Set the buzz pin as an OUTPUT
  digitalWrite(buzz, LOW);  // Initialize buzzer to LOW

  if (EEPROM.read(1000) != 15) {
    for (unsigned int i = 0; i < EEPROM.length(); i++)
      EEPROM.write(i, 0);

    EEPROM.write(1000, 15);
  }
  Serial.begin(9600);

  lcd.init();
  
  lcd.backlight();
  printLCD(" ", "Connecting...");
 

//  // Connect to Wi-Fi
//  WiFi.begin(wifiSSID, wifiPassword);
//  while (WiFi.status() != WL_CONNECTED) {
//    Serial.println("Connecting to WiFi...");
//    delay(100);
//  }
//
//  Serial.println("Connected to WiFi");
//
//  delay(100);

 if (connectToServer()) {
    Serial.println("Connected to server");
     
  } else {
    Serial.println("Connection to server failed");
    
 
  }
  
  irReceiver.enableIRIn(); // Start the receiver
}

void printLCD(String label, String combinedValue, String wifiStatus, String serverStatus) {
  lcd.setCursor(0, 0);
  if (serverStatus == "Error...") {
    lcd.print("SERVER: " + serverStatus);
  } else{
  lcd.print("WiFi: " + wifiStatus);
  }
  
  lcd.setCursor(0, 1);
    lcd.print(label + " " + combinedValue);
}


void loop() {
  if (irReceiver.decode(&results)) {
    Serial.println("Received IR code: " + String(results.value, HEX));
    updateState();
    decodeState();
    irReceiver.resume();
  }

  if (value1 == "" && value2 == "" && value3 == "") {
    currentState = IDLE;
  }

  if (button == BUTTON_NEXT || button == BUTTON_PREV ) {
    printLCD("TOKEN NO.=", combinedValue, WiFi.status() == WL_CONNECTED ? "Connected.." : "Connecting..", connectToServer() ? "Connected" : "Error...");
} else {
    printLCD( "TOKEN NO.=", String(value3) + String(value2) + String(value1), WiFi.status() == WL_CONNECTED ? "Connected.." : "Connecting..", connectToServer() ? "Connected" : "Error...");

}

  delay(1);
}


 
void printLCD(String values, String wifiStatus) {
  lcd.setCursor(0, 0);
  lcd.print("WiFi:" + wifiStatus);
  lcd.setCursor(0, 1);
  lcd.print(values);
}
