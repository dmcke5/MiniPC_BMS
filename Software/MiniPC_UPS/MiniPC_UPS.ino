#include <Wire.h>
#include <HIDPowerDevice.h>
//#include <LowPower.h>

//User Config
#define powerOffDelay 10000 //Time in milliseconds to hold power button for hard shut off
#define shutdownDelay 2000  //Time in milliseconds to hold power button to initiate Shutdown
#define reportDelay 5000    //Time in milliseconds. How often to Update UPS info
int batteryCapacity = 5000; //Battery Design Capacity in mAh
bool disableAutoShutoff = false; //Used to disable power sense shutdown for testing
bool debug = false;         //Set to True to disable UPS functionality and enable serial logging.
bool forceFuelGaugeReset = false; //Set to True to reset fuel gauge on startup. Learnt battery information will be lost!

//Pin configuration
int boostEnable = 4; //PD4 // Enables 19v Boost Output
int pcSwitch = 6; //PD7  // High to trigger PC power button press
//int pcPower = 12; //PD6 // Input to sense if PC is powered on
int pwrButton = 7; //PE6 // Power button

//bool systemAwake = true;
int StatusPOR = 0; //This Variable is set to 1 by the Fuel gauge on first power up
bool updateBQ = false;

//Timers
unsigned long shutdownTimer = 0;
unsigned long powerBtnTimer = 0;
unsigned long reportTimer = 0;

//UPS
int batterySOC;
bool bCharging = false;
bool bACPresent = false;
bool bDischarging = false;
byte iRemaining =0, iPrevRemaining=0;
bool lastACState = false;
bool lastChargeState = false;

int powerSupplyState = 0; //0 = Active, 1 = Sleep, 2 = Powered Down

void setup() {
  TXLED0;
  RXLED0;
  if(debug){
    Serial.begin(57600);
    while(!Serial){}
  }
  Wire.begin();

  max17261Init(); //Set up fuel gauge

  //Configure IO pins
  pinMode(boostEnable, OUTPUT);
  pinMode(pcSwitch, OUTPUT);
  //pinMode(pcPower, INPUT);
  pinMode(pwrButton, INPUT);

  //Enable Boost 
  digitalWrite(boostEnable, HIGH);

  //Setup UPS
  if(!debug){
    UPS_Setup();
  }

  //Setup BQ25792
  if(updateBQ){
    bqConfig();
  }
  //digitalWrite(pcSwitch, HIGH);
  //readAllRegisters();
}

void loop() {
  if(digitalRead(pwrButton) == 1){ //Power button pressed
    if(powerBtnTimer == 0){ //If power button timer isn't started
      powerBtnTimer = millis(); //Start timer
      if(debug){
        Serial.println("Button Pressed");
      }
    }
    if(powerBtnTimer + powerOffDelay < millis()){ //If ten seconds has elapsed
      shutDownPSU(); // Trigger shutdown sequence
    }
  } else { //power button released
    if(powerBtnTimer != 0){ //power button timer is set
      if(powerBtnTimer + shutdownDelay < millis()){ //power button was held for more than shutdownDelay
        //Initiate PC Shutdown
        if(debug){
          Serial.println("Shutdown PC");
        }
        shutdownPC();
      } else { //power button wasn't held for more than one second
        if(debug){
          Serial.println("Sleep PC");
        }
        sleepPC(); //Initiate PC Sleep
      }
      powerBtnTimer = 0; //Reset power button timer
    }
  }
  
  /*if(shutdownTimer != 0 && shutdownTimer + 3000 < millis()){ //Shutdown timer is active and expired
    shutDownPSU();
  }*/
  iRemaining = getSOC();
  int temp = GetChargeStatus();
  if(temp != 0 && temp != 7){ //0 = Not Charging, 7 = Charge Termination Done
    bCharging = true;
  } else {
    bCharging = false;
  }
  if(GetACDRV1Status() || GetACDRV2Status()){
    bACPresent = true;
  } else {
    bACPresent = false;
  }

  //Force instant UPS report when charger disconnected/connected
  if(lastACState != bACPresent){
    lastACState = bACPresent;
    reportTimer = 0;
  }
  if(lastChargeState != bCharging){
    lastChargeState = bCharging;
    reportTimer = 0;
  }

  if(reportTimer + reportDelay < millis()){
    if(debug){
      Serial.print("SOC = ");
      Serial.println(getSOC());
      Serial.print("Remaining Capacity = ");
      Serial.println(getRemainingCapacity());
      Serial.print("Pack Voltage = ");
      Serial.println(getInstantaneousVoltage() * 4);
      Serial.print("V Empty = ");
      Serial.println(getVEmpty());
      Serial.print("Pack Capacity = ");
      Serial.println(getCapacity());
      Serial.print("Calculated Pack Capacity = ");
      Serial.println(getRealCapacity());
      Serial.print("iRemaining = ");
      Serial.println(iRemaining);
      Serial.print("Current = ");
      Serial.println(getInstantaneousCurrent());
      Serial.print("Charging = ");
      Serial.println(bCharging);
      Serial.print("AC Present = ");
      Serial.println(bACPresent);

      readAllFaultRegisters();
      if(bCharging){
        Serial.print("TTF = ");
        Serial.println(getTimeToFull() * 60);
      } else {
        Serial.print("TTE = ");
        Serial.println(getTimeToEmpty() * 60);
      }
      
      Serial.println();
    } else {
      UPS_Report();
    }
    reportTimer = millis();
  }
}

void sleepPC(){
  digitalWrite(pcSwitch, HIGH);
  delay(100);
  digitalWrite(pcSwitch, LOW);
}

void shutdownPC(){
  digitalWrite(pcSwitch, HIGH);
  delay(2000);
  digitalWrite(pcSwitch, LOW);
}

void shutDownPSU(){
  digitalWrite(boostEnable, LOW); //Disable Boost Converter
}

void bqConfig(){ //Call to configure BQ register settings after shutdown
  SetWatchdogTimerSetting(0);
  SetInputCurrentLimit(3000); //Set input current to maximum
  SetEN_EXTILIM(false);
  SetChargeCurrentLimit(3000); //Set charge current to 3A
}
