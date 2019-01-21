/*
reads the pin A0 (i.e. Current Transformer+Amplifier output ) and prints the non zero value to console
14th October 2018
 */
// 


int maxValue = 0;
int valToCheck = 0;
int valToSend = 0;

unsigned long startT = 999000;
unsigned long interval = 30000;

unsigned long heartstartT = 999000;
unsigned long heartinterval = 40000;

uint32_t period = 10000L;   

void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(9600);
  delay(500);
}

void loop() {

  unsigned long loopT = millis();
  unsigned long heartloopT = millis();

  if ((unsigned long)(loopT - startT) >= interval){
    
   Serial.println("Checking Fence Now !!... X-( ");
   checkFence();
   startT = loopT;
  }

  if ((unsigned long)(heartloopT - heartstartT) >= heartinterval)
  {
    Serial.println("Send Heartbeart Now !!... :-& .. ");
    Serial.print(valToSend);
    heartstartT = heartloopT;
  }
  
  Serial.print(".");
  delay(5000);
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
  }
}
