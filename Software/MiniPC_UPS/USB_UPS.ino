// String constants 
const char STRING_DEVICECHEMISTRY[] PROGMEM = "LiOn";//"PbAc";
const char STRING_OEMVENDOR[] PROGMEM = "CNCDAN";
const char STRING_SERIAL[] PROGMEM = "MiniPC_UPS"; 

const byte bDeviceChemistry = IDEVICECHEMISTRY;
const byte bOEMVendor = IOEMVENDOR;

uint16_t iPresentStatus = 0, iPreviousStatus = 0;

byte bRechargable = 1;
byte bCapacityMode = 2;  // 0 = mah, 1 = mwh, 2 = %, bool

// Physical parameters
const uint16_t iConfigVoltage = 1380;
uint16_t iVoltage =1300, iPrevVoltage = 0;
uint16_t iRunTimeToEmpty = 7200, iPrevRunTimeToEmpty = 0;
uint16_t iAvgTimeToFull = 7200;
uint16_t iAvgTimeToEmpty = 7200;
uint16_t iRemainTimeLimit = 600;
int16_t  iDelayBe4Reboot = 60;
int16_t  iDelayBe4ShutDown = 60;

byte iAudibleAlarmCtrl = 1; // 1 - Disabled, 2 - Enabled, 3 - Muted

// Parameters for ACPI compliancy
const byte iDesignCapacity = 100;
byte iWarnCapacityLimit = 10; // warning at 10% 
byte iRemnCapacityLimit = 5; // low at 5% 
const byte bCapacityGranularity1 = 1;
const byte bCapacityGranularity2 = 1;
byte iFullChargeCapacity = 100;



//int iRes=0;

void UPS_Setup(){
  PowerDevice.begin();
  PowerDevice.setSerial(STRING_SERIAL); 
  PowerDevice.setOutput(Serial);

  PowerDevice.setFeature(HID_PD_PRESENTSTATUS, &iPresentStatus, sizeof(iPresentStatus));
  
  PowerDevice.setFeature(HID_PD_RUNTIMETOEMPTY, &iRunTimeToEmpty, sizeof(iRunTimeToEmpty));
  PowerDevice.setFeature(HID_PD_AVERAGETIME2FULL, &iAvgTimeToFull, sizeof(iAvgTimeToFull));
  PowerDevice.setFeature(HID_PD_AVERAGETIME2EMPTY, &iAvgTimeToEmpty, sizeof(iAvgTimeToEmpty));
  PowerDevice.setFeature(HID_PD_REMAINTIMELIMIT, &iRemainTimeLimit, sizeof(iRemainTimeLimit));
  PowerDevice.setFeature(HID_PD_DELAYBE4REBOOT, &iDelayBe4Reboot, sizeof(iDelayBe4Reboot));
  PowerDevice.setFeature(HID_PD_DELAYBE4SHUTDOWN, &iDelayBe4ShutDown, sizeof(iDelayBe4ShutDown));
  
  PowerDevice.setFeature(HID_PD_RECHARGEABLE, &bRechargable, sizeof(bRechargable));
  PowerDevice.setFeature(HID_PD_CAPACITYMODE, &bCapacityMode, sizeof(bCapacityMode));
  PowerDevice.setFeature(HID_PD_CONFIGVOLTAGE, &iConfigVoltage, sizeof(iConfigVoltage));
  PowerDevice.setFeature(HID_PD_VOLTAGE, &iVoltage, sizeof(iVoltage));

  PowerDevice.setStringFeature(HID_PD_IDEVICECHEMISTRY, &bDeviceChemistry, STRING_DEVICECHEMISTRY);
  PowerDevice.setStringFeature(HID_PD_IOEMINFORMATION, &bOEMVendor, STRING_OEMVENDOR);

  PowerDevice.setFeature(HID_PD_AUDIBLEALARMCTRL, &iAudibleAlarmCtrl, sizeof(iAudibleAlarmCtrl));

  PowerDevice.setFeature(HID_PD_DESIGNCAPACITY, &iDesignCapacity, sizeof(iDesignCapacity));
  PowerDevice.setFeature(HID_PD_FULLCHRGECAPACITY, &iFullChargeCapacity, sizeof(iFullChargeCapacity));
  PowerDevice.setFeature(HID_PD_REMAININGCAPACITY, &iRemaining, sizeof(iRemaining));
  PowerDevice.setFeature(HID_PD_WARNCAPACITYLIMIT, &iWarnCapacityLimit, sizeof(iWarnCapacityLimit));
  PowerDevice.setFeature(HID_PD_REMNCAPACITYLIMIT, &iRemnCapacityLimit, sizeof(iRemnCapacityLimit));
  PowerDevice.setFeature(HID_PD_CPCTYGRANULARITY1, &bCapacityGranularity1, sizeof(bCapacityGranularity1));
  PowerDevice.setFeature(HID_PD_CPCTYGRANULARITY2, &bCapacityGranularity2, sizeof(bCapacityGranularity2));

  //PowerDevice.sendReport(
}

void UPS_Report(){
  int temp = GetChargeStatus();
  if(temp != 0 && temp != 7){ //0 = Not Charging, 7 = Charge Termination Done
    bCharging = true;
    iAvgTimeToFull = getTimeToFull() * 3600; //Convert to seconds
  } else {
    bCharging = false;
    iRunTimeToEmpty = getTimeToEmpty() * 3600; //Convert to seconds
  }
  if(GetACDRV1Status() || GetACDRV2Status()){
    bACPresent = true;
  } else {
    bACPresent = false;
  }

  bDischarging = !bCharging; // TODO - replace with sensor

  iRemaining = getSOC();
  
  iVoltage = GetAdcVBAT();
  
    // Charging
  if(bCharging) 
    bitSet(iPresentStatus,PRESENTSTATUS_CHARGING);
  else
    bitClear(iPresentStatus,PRESENTSTATUS_CHARGING);
  if(bACPresent) 
    bitSet(iPresentStatus,PRESENTSTATUS_ACPRESENT);
  else
    bitClear(iPresentStatus,PRESENTSTATUS_ACPRESENT);
  if(iRemaining == iFullChargeCapacity) 
    bitSet(iPresentStatus,PRESENTSTATUS_FULLCHARGE);
  else 
    bitClear(iPresentStatus,PRESENTSTATUS_FULLCHARGE);
    
  // Discharging
  if(bDischarging) {
    bitSet(iPresentStatus,PRESENTSTATUS_DISCHARGING);
    if(iRemaining < iRemnCapacityLimit) bitSet(iPresentStatus,PRESENTSTATUS_BELOWRCL);
    
    if(iRunTimeToEmpty < iRemainTimeLimit) 
      bitSet(iPresentStatus, PRESENTSTATUS_RTLEXPIRED);
    else
      bitClear(iPresentStatus, PRESENTSTATUS_RTLEXPIRED);

  }
  else {
    bitClear(iPresentStatus,PRESENTSTATUS_DISCHARGING);
    bitClear(iPresentStatus, PRESENTSTATUS_RTLEXPIRED);
    bitClear(iPresentStatus, PRESENTSTATUS_SHUTDOWNIMNT);
  }

  // Shutdown requested
  /*if(iDelayBe4ShutDown > 0 ) {
      bitSet(iPresentStatus, PRESENTSTATUS_SHUTDOWNREQ);
      Serial.println("shutdown requested");
  }
  else
    bitClear(iPresentStatus, PRESENTSTATUS_SHUTDOWNREQ);*/

  // Shutdown imminent
  if((iPresentStatus & (1 << PRESENTSTATUS_SHUTDOWNREQ)) || 
     (iPresentStatus & (1 << PRESENTSTATUS_RTLEXPIRED))) {
    bitSet(iPresentStatus, PRESENTSTATUS_SHUTDOWNIMNT);
    //Serial.println("shutdown imminent");
  }
  else
    bitClear(iPresentStatus, PRESENTSTATUS_SHUTDOWNIMNT);


  
  bitSet(iPresentStatus ,PRESENTSTATUS_BATTPRESENT);

  //************ Bulk send or interrupt ***********************

  //if((iPresentStatus != iPreviousStatus) || (iRemaining != iPrevRemaining) || (iRunTimeToEmpty != iPrevRunTimeToEmpty) ) {

    PowerDevice.sendReport(HID_PD_REMAININGCAPACITY, &iRemaining, sizeof(iRemaining));
    PowerDevice.sendReport(HID_PD_PRESENTSTATUS, &iPresentStatus, sizeof(iPresentStatus)); //iRes = 
    if(bDischarging) {
      PowerDevice.sendReport(HID_PD_RUNTIMETOEMPTY, &iRunTimeToEmpty, sizeof(iRunTimeToEmpty)); //Send time till Empty
    } else {
      PowerDevice.sendReport(HID_PD_AVERAGETIME2FULL, &iAvgTimeToFull, sizeof(iAvgTimeToFull)); //Send time till Full
    }
    PowerDevice.sendReport(HID_PD_VOLTAGE, &iVoltage, sizeof(iVoltage));

    //if(iRes <0 ) {
      //digitalWrite(10, HIGH);
    //}
    //else
      //digitalWrite(10, LOW);
        
    //iPreviousStatus = iPresentStatus;
    //iPrevRemaining = iRemaining;
    //iPrevRunTimeToEmpty = iRunTimeToEmpty;
  //}
  
  //Serial.println(iRemaining);
  //Serial.println(iRunTimeToEmpty);
  //Serial.println(iRes);
}