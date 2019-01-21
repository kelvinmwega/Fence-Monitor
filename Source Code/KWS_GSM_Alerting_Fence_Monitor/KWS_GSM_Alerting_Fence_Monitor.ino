

#include <mqtt.h>
#include <avr/wdt.h>
#include <MemoryFree.h>;

const char server[] PROGMEM = "159.8.169.212"; // uk
const char clientId[] PROGMEM = "d:0fvk13:FenceMonitor:868345034730333";
const char topic[] PROGMEM = "iot-2/evt/status/fmt/format_string";
const char token[] PROGMEM = "devmonitor";

char serv[20], client[45], top[35], buffer[6], dataToSend[100], imeiBuf[20];

const char* const string_table[] PROGMEM = {server, clientId, topic, token};

long failures = 0;
long count = 0;
int defaultLightOn = 500;
String quality;
long previousMillis = 0;
String imei = "";
String batt = "0";
uint32_t period = 10000L;   
int maxValue = 0;
int valToCheck = 0;
int valToSend = 0;

unsigned long startT = 999000;
unsigned long interval = 120000;

unsigned long batstartT = 999000;
unsigned long batinterval = 120000;

unsigned long heartstartT = 999000;
unsigned long heartinterval = 300000;

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

#define LED13 13 
#define GSM_IGN 6 

void setup() {

  Serial.begin(19200);

  wdt_disable();
  wdt_enable(WDTO_8S);
  wdt_reset();

  setUpBoard();
  delay(1000);
  blinkLED(13,2,500);
}

void loop() {

  wdt_reset();
  
  unsigned long loopT = millis();
  unsigned long battLoopT = millis();
  unsigned long heartloopT = millis();

  if ((unsigned long)(loopT - startT) >= interval){
   checkFence();
   startT = loopT;
  }

//  if ((unsigned long)(heartloopT - heartstartT) >= heartinterval)
//  {
//    sendDataMsg("00");
//    heartstartT = heartloopT;
//  }
//
//  if ((unsigned long)(battLoopT - batstartT) >= batinterval)
//  {
//    batt = powerManager();
//    blinkLED(13,3,500);
//    batstartT = battLoopT;
//  }
  
  delay(5000);
}

void setUpBoard(){
  wdt_reset();
  //sensors.begin();
  blinkLED(13,10,100);
  initHWPins();
  blinkLED(13,1,defaultLightOn);
  gsm_is_ready_MODULE();
  blinkLED(13,2,defaultLightOn);
  blinkLED(13,3,defaultLightOn);
  gsm_is_ready_TCP();
  blinkLED(13,4,defaultLightOn);

  delay(500);
  imei = gsmGetIMEI();
//  imei.toCharArray(clientId, 20);
  delay(500);
  batt = powerManager();
  
//  sendHelloMsg();
}

void sendDataMsg(String msgType)
{
  wdt_reset();
  
  msgType.toCharArray(buffer, 10);
  strcpy(dataToSend, buffer);
  strcat(dataToSend, ",");
  
  imei.toCharArray(imeiBuf, 20);
  strcat(dataToSend, imeiBuf);
  strcat(dataToSend, ",");

  batt.toCharArray(buffer, 10);
  strcat(dataToSend, buffer);
  strcat(dataToSend, ",");

  ltoa(failures, buffer, 10);
  strcat(dataToSend, buffer);
  strcat(dataToSend, ",");

  ltoa(freeMemory(), buffer, 10);
  strcat(dataToSend, buffer);
  strcat(dataToSend, ",");

  ltoa(count, buffer, 10);
  strcat(dataToSend, buffer);
  strcat(dataToSend, ",");

  ltoa(valToSend, buffer, 10);
  strcat(dataToSend, buffer);
  strcat(dataToSend, ",");

  quality = gsmGetSignalQuality();
  quality.toCharArray(buffer, 10);
  strcat(dataToSend, buffer);

  procMsgHandler();
}

void procMsgHandler(){
  wdt_reset();
  
  strcpy_P(serv, (char*)pgm_read_word(&(string_table[0])));
  strcpy_P(client, (char*)pgm_read_word(&(string_table[1])));
  strcpy_P(top, (char*)pgm_read_word(&(string_table[2])));
  
  sendMQTTMessage(client, serv, top, dataToSend);
}

void checkFence(){
  maxValue = 0;
  
  for( uint32_t tStart = millis();  (millis()-tStart) < period;){
        int hvReading = analogRead(A0);

       if ( hvReading > 300 ) {
          for (int i = 0; i < 10; i++ ) {
            hvReading = analogRead(A0);
            if (maxValue < hvReading) {
              maxValue = hvReading;
            }
            valToCheck = maxValue;
          }
          digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
          delay(100);               // wait for a second
          digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
       } 
    }

   valToCheck = maxValue;
   procAlert();
}

void procAlert(){
  
  if (valToCheck > 300){
    Serial.print("All is Good !!... :-)  ");
    Serial.println(valToCheck);
    valToSend = valToCheck;
    valToCheck = 0;
  } else {
    Serial.print("Send Alert Now !!... :-( ");
    Serial.println(valToCheck);
    valToSend = valToCheck;
    valToCheck = 0;
    sendDataMsg("01");
  }
}

String powerManager(){
  return String(gsmGetBattery());
}

void initHWPins() {
  wdt_reset();
  pinMode(LED13, OUTPUT);
  pinMode(GSM_IGN, OUTPUT);
  digitalWrite(GSM_IGN, HIGH);
}

void blinkLED(int ledID, int repeat, int wait) {
  wdt_reset();
  if (repeat == 999) {
    digitalWrite(ledID, HIGH);
    return;
  }
  if (repeat == 0) {
    digitalWrite(ledID, LOW);
    return;
  }
  for (int i = 0; i < repeat; i++) {
    digitalWrite(ledID, HIGH);
    delay(wait);
    digitalWrite(ledID, LOW);
    delay(wait);
  }
  delay(1000);
}

void sendMQTTMessage(char* clientId, char* brokerUrl,  char* topic, char* message){  

  byte    mqttMessage[127];
  int     mqttMessageLength = 0;
  
  blinkLED(13,999,defaultLightOn);
  
  mqttMessageLength=mqtt_connect_message(mqttMessage, clientId, "use-token-auth", "devmonitor");   //prepare MQTTConnect Message String & calculate length
  //mqttMessageLength=mqtt_connect_message(mqttMessage, clientId, "", "");
  gsm_send_tcp_MQTT_byte(brokerUrl,mqttMessage,mqttMessageLength);                                         // Send MQTT formatted connect string to IoT Server
  // Publish to MQTT Server
  mqtt_publish_message(mqttMessage, topic, message);                                                       // prepare MQTTPublish Message Message String
  mqttMessageLength = 4 + strlen(topic) + strlen(message);                                                 // calculate the message length
  gsm_send_tcp_MQTT_byte(brokerUrl,mqttMessage, mqttMessageLength);                                        // Send MQTT formatted Message string to IoT Server
  // disconnect from MQTT Server
  mqttMessageLength = mqtt_disconnect_message(mqttMessage);                                                 // prepare MQTT disconnect Message Message String and length
  gsm_send_tcp_MQTT_byte(brokerUrl,mqttMessage, mqttMessageLength);  
  blinkLED(13,0,100);
}


///    *************************************************************************
///    *************************************************************************
///    ****             GSM/TCP FUNCTIONS   GSM/TCP FUNCTIONS               ****
///    ****             GSM/TCP FUNCTIONS   GSM/TCP FUNCTIONS               ****
///    ****             GSM/TCP FUNCTIONS   GSM/TCP FUNCTIONS               ****
///    *************************************************************************
///    *************************************************************************

boolean gsm_send_tcp_MQTT_byte( char* mqttServer, byte* message, int lenMessage) {

x:
  wdt_reset();
  if (gsm_is_ready_TCPSERVER(mqttServer, "1883") == false) goto x;
  gsm_send("AT+CIPSEND");
  delay(300);
  if ( gsm_response_check(">") == false) return false;
  for (int j = 0; j < lenMessage; j++)  Serial.write(message[j]);
  Serial.write(byte(26));
  delay(2000);
  gsm_response_check("SEND OK");     //dont use response.. because Data Transmit Check is counts!
  gsm_send("AT+CIPACK");
  delay(500);

  if ( gsm_response_check("Data Transmit Check") == false) {
    goto x;
  }
  return true;
}


boolean gsm_send_tcp( char* message) {
x:
  wdt_reset();
  if (gsm_is_ready_TCPSERVER("198.100.31.2", "19940") == false) goto x;       //VPS
  gsm_send("AT+CIPSEND");
  delay(300);
  if ( gsm_response_check(">") == false) return false;
  gsm_send(message);
  Serial.write(byte(26));
  delay(2000);
  gsm_response_check("SEND OK");     //dont use response.. because Data Transmit Check is counts!
  gsm_send("AT+CIPACK");
  delay(500);
  if ( gsm_response_check("Data Transmit Check") == false) {
    goto x;
  }
  return true;
}


boolean gsm_is_ready_TCPSERVER(char* server_ip, char* server_port) {
  int cnt = 0;
  char at_command[50];
  strcpy(at_command, "AT+CIPSTART=\"TCP\",\"" );
  strcat(at_command, server_ip);
  strcat(at_command, "\",\"");
  strcat(at_command, server_port);
  strcat(at_command, "\"");
x:
  wdt_reset();
  gsm_send("AT+CIPSTATUS");
  delay(300);
  if ( gsm_response_check("STATE: CONNECT OK") == true) return true;
  if (cnt > 7) {
    cnt = 0;
    gsm_startup();
    delay(500);
    gsm_startup();
    gsm_is_ready_MODULE();
  }
  cnt++;
  gsm_is_ready_TCP();
  gsm_send(at_command);
  delay(300);
  gsm_response_check("OK");    // not important... does not helps !!
  goto x;
}

boolean gsm_is_ready_TCP() {
x:
  wdt_reset();
  digitalWrite(LED13, LOW);
  gsm_send("AT+CIPSTATUS");
  delay(300);
  if ( gsm_response_check("STATE: IP STATUS") == true) {
    digitalWrite(LED13, HIGH);
    return true;
  }
  gsm_is_ready_GPRS();
  gsm_send("AT+CIPSHUT");
  delay(300);
  if ( gsm_response_check("SHUT OK") == false ) goto x;
  gsm_send("AT+CSTT=\"internet\"");
//  gsm_send("AT+CSTT=\"bicsapn\"");
  delay(1000);
  if ( gsm_response_check("OK") == false ) goto x;
  gsm_send("AT+CIICR");
  delay(1000);
  if ( gsm_response_check("OK") == false ) goto x;
  gsm_send("AT+CIFSR");
  delay(300);
  if ( gsm_response_check(".") == false ) goto x;
  goto x;
}

boolean gsm_is_ready_GPRS() {
  int reps = 0;
x:
  wdt_reset();
  gsm_send("AT+CGATT=1");
  delay(300);
  gsm_response_check("OK");
  gsm_send("AT+CGATT?");
  delay(300);
  if ( gsm_response_check("+CGATT: 1") == true) {
    return true;
  }
  if (reps == 7)
  {
    gsm_is_ready_NETWORK();
    reps = 0;
    goto x;
  }
  reps++;
  goto x;
}


boolean gsm_is_ready_NETWORK() {
  int noOfTry = 0;
x:
  wdt_reset();
  noOfTry++;
  gsm_send("AT+CREG?");
  delay(1000);
  if ( gsm_response_check("+CREG: 0,1") == true) return true;
  if (noOfTry == 3) {
    noOfTry = 0;
    gsm_is_ready_MODULE();
  }
  goto x;
}

boolean gsm_is_ready_MODULE() {
  int noOfTry = 0;
  Serial.begin(19200);
x:
  wdt_reset();
  noOfTry++;
  gsm_send("AT");
  delay(500);
  if ( gsm_response_check("OK") == true) return true;
  if (noOfTry == 3) {
    noOfTry = 0;
    failures++;
    gsm_startup();
  }

  goto x;
}


boolean gsm_response_check(String expected_response) {
  wdt_reset();
  String  gprs_resp_str;

  Serial.flush();
  gprs_resp_str = gsm_read();

  if ( expected_response == "Data Transmit Check") {     //Speciall check the response !!
    int indexOfComma;
    char checkChar = '0';
    indexOfComma = gprs_resp_str.lastIndexOf(',');   // , sign before number char left for Tx.. Should be 0 !
    if ( gprs_resp_str.charAt(indexOfComma + 1) == checkChar) {
      return true;
    }
    else return false;  //still there are chars to send... NOT POSSIBLE ! reTry to SEND !
    return true;
  }

  if ( expected_response == "GET_DATE_TIME") {     //not response check.. just GET DATE/TIME
    int indexOfComma;
    char checkChar = '0';
    indexOfComma = gprs_resp_str.lastIndexOf(',');   // , sign before number char left for Tx.. Should be 0 !
    if ( gprs_resp_str.charAt(indexOfComma + 1) == checkChar) {
      return true;
    }
    else return false;  //still there are chars to send... NOT POSSIBLE ! reTry to SEND !
    return true;
  }
  if (gprs_resp_str.indexOf(expected_response) > -1) {
    return true;
  }
  else {
    return false;
  }
  return false;   //should never come here !
}

String gsm_read() {
  wdt_reset();
  String gsmString;
  while (Serial.available()) gsmString = gsmString + (char)Serial.read();
  return gsmString;
}


void gsm_send(char* command) {
  wdt_reset();
  Serial.println(command);
  delay(2500);
}

void gsm_startup() {
  wdt_reset();

  gsm_toggle();

}


void gsm_toggle() {
  wdt_reset();
  digitalWrite(GSM_IGN, HIGH);
  delay(1000);
  digitalWrite(GSM_IGN, LOW);
  delay(1500);
  digitalWrite(GSM_IGN, HIGH);
  delay(5000);
}

void gsm_shutdown() {
  wdt_reset();
  Serial.begin(19200);
  Serial.flush();
  Serial.println("AT+CPOWD=0");
  delay(2000);
  Serial.flush();
  Serial.println("AT");
  delay(500);
  Serial.end();
}

String  gsmGetIMEI() {
  wdt_reset();
  int tmp;
  String IMEI;

  gsm_send("\nAT+CGSN");  //ask  +CCGSN: "8679123519452673"
  delay(2000);
  IMEI = gsm_read();
  return IMEI.substring(tmp + 10, tmp + 10 + 15);
}


int gsmGetBattery() {
  wdt_reset();
  String voltageBattery;
  int tmp;
  gsm_send("\nAT+CBC");    // +CBC: 0,60,3790
  delay(1000);
  voltageBattery = gsm_read();
  tmp = voltageBattery.lastIndexOf(",");
  return voltageBattery.substring(tmp + 1, tmp + 5).toInt();
}


String  gsmGetSignalQuality() {
  wdt_reset();
  String signalQuality;
  int tmp, len;
  gsm_send("\nAT+CSQ");    // +CSQ: 18,0
  delay(300);
  signalQuality = gsm_read();
  tmp = signalQuality.indexOf("+CSQ:");
  len = signalQuality.length();
  return signalQuality.substring(tmp + 6, tmp + 8);
}

char readFromProgmem(int i){
  char myChar;
  char buffer[30];

  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[i])));
  
  Serial.println(buffer);
  return buffer;
}
