#define bq25792Addr 0x6B

#define DevicePartNumberMask 0b00111000
#define PrechargeCurrentLimitMask 0b00111111
#define ChargerStatus0PowerGoodStatusMask 0b00001000
#define ChargerStatus1ChargeStatusMask 0b11100000
#define ChargerStatus1VbusStatusMask 0b00011110
#define ChargerStatus1Bc12DetectionMask 0b00000001
#define ChargerControl1WatchdogMask 0b00000111
#define AdcControlEnableMask 0b10000000
#define AdcControlConversionRateMask 0b01000000
#define AdcControlResolutionMask 0b00110000
#define AdcControlAverageControlMask 0b00001000
#define AdcControlInitialAverageMask 0b00000100
#define ThermalRegulationThresholdMask 0b00000011
#define ThermalShutdownThresholdMask 0b00001100
#define ChargerStatus3IcoStatusMask 0b11000000
#define NtcControl0ChargerVoltageMask 0b11100000
#define NtcControl0ChargeCurrentMask 0b00011000
#define NtcControl1Vt2ComparatorVoltageMask 0b11000000
#define NtcControl1Vt3ComparatorVoltageMask 0b00110000
#define NtcControl1OtgHotTemperatureMask 0b00001100
#define NtcControl1OtgColdTemperatureMask 0b00000010
#define NtcControl1IgnoreTSMask 0b00000001
#define EN_ACDRV1Mask 0b01000000
#define EN_ACDRV2Mask 0b10000000
#define ChargeENMask 0b00100000
#define EN_EXTILIMMask 0b00000010

int FixedOffsetMinimalSystemVoltage = 2500;
int MaxValueMinimalSystemVoltage = 16000;
int StepMinimalSystemVoltage = 250;
int PrechargeCurrentLimitMinValue = 40;
int StepPrechargeCurrentLimit = 40;
int PrechargeCurrentLimitMaxValue = 2000;
int FixedOffsetMinimalChargeVoltageLimit = 3000;
int MaxValueChargeVoltageLimit = 18800;
int FixedOffsetMinimalChargeCurrentLimit = 50;
int MaxValueChargeCurrentLimit = 5000;
int MaxValueInputVoltageLimit = 22000;
int MinValueInputVoltageLimit = 3600;
int FixedOffsetMinimalInputCurrentLimit = 100;
int MaxValueInputCurrentLimit = 3300;

int VbusAdcStep = 1;
int ChargeVoltageStep = 10;
int ChargeInputCurrentStep = 10;
int InputVoltageStep = 100;
float TdieStemp = 0.5f;

#define REG00_Minimal_System_Voltage 0x01
#define REG01_Charge_Voltage_Limit 0x01
#define REG03_Charge_Current_Limit 0x03
#define REG05_Input_Voltage_Limit 0x05
#define REG06_Input_Current_Limit 0x06
#define REG08_Precharge_Control 0x08
#define REG0F_Charger_Control_0 0x0F
#define REG10_Charger_Control_1 0x10
#define REG13_Charger_Control_4 0x13
#define REG14_Charger_Control_5 0x14
#define REG16_Temperature_Control 0x16
#define REG17_NTC_Control_0 0x17
#define REG18_NTC_Control_1 0x18

#define REG1B_Charger_Status_0 0x1B
#define REG1C_Charger_Status_1 0x1C
#define REG1D_Charger_Status_2 0x1D
#define REG1E_Charger_Status_3 0x1E
#define REG1F_Charger_Status_4 0x1F
#define REG20_FAULT_Status_0 0x20
#define REG21_FAULT_Status_1 0x21
#define REG22_Charger_Flag_0 0x22
#define REG23_Charger_Flag_1 0x23
#define REG24_Charger_Flag_2 0x24
#define REG25_Charger_Flag_3 0x25
#define REG26_FAULT_Flag_0 0x26
#define REG27_FAULT_Flag_1 0x27

#define REG2E_ADC_Control 0x2E
#define REG35_VBUS_ADC 0x35
#define REG37_VAC1_ADC 0x37
#define REG39_VAC2_ADC 0x39
#define REG3B_VBAT_ADC 0x3B
#define REG3D_VSYS_ADC 0x3D
#define REG41_TDIE_ADC 0x41
#define REG48_Part_Information 0x48
#define REG0E_Timer_Control 0x0E

byte buffer[10];

void ReadFromRegister(int addr, int reg, int readByteCount) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(bq25792Addr,readByteCount,true);
  int count = 0;
  while (Wire.available()) {
    buffer[count] = Wire.read();
    count++;
  }
}

void WriteToRegister(int addr, int reg, int length) { //Writes specified number of bytes from Global buffer to register
  Wire.beginTransmission(addr);
  Wire.write(reg);
  for(int i = 0; i < length; i++){
    Wire.write(buffer[i]);
  }
  /*for(int i = length - 1; i > 0; i--){
    Wire.write(buffer[i]);
  }*/
  Wire.endTransmission();
}

void ResetWatchdog() {
  buffer[0] = 0x08;
  WriteToRegister(bq25792Addr, REG10_Charger_Control_1, 1);//0b00001000);//8);
  //WriteToRegister(bq25792Addr, bq25792Addr, Register.REG10_Charger_Control_1, 0b0000_1000);
}

void SetChargeEnabled() {
  buffer[0] = 0b00010000;
  WriteToRegister(bq25792Addr, REG0E_Timer_Control, 1);//0b00001000);//8);
  //WriteToRegister(bq25792Addr, bq25792Addr, Register.REG10_Charger_Control_1, 0b0000_1000);
}

int GetChargerStatus0() {
  ReadFromRegister(bq25792Addr, REG1C_Charger_Status_1, 1);
  return buffer[0];
}

void readAllFaultRegisters(){
  for(int i = 27; i < 39; i++){
    Serial.print("Register ");
    Serial.print(i);
    Serial.print(" = ");
    ReadFromRegister(bq25792Addr, i, 1);
    Serial.println(buffer[0]);
  }
}

void readAllRegisters(){
  for(int i = 0; i < 73; i++){
    if(i != 0x02 && i != 0x04 && i != 0x07 && i != 0x0C && i != 0x1A && i != 0x32 && i != 0x34 && i != 0x36 && i != 0x38 && i != 0x3A && i != 0x3C && i != 0x3E && i != 0x42 && i != 0x44 && i != 0x46){
      Serial.print("Register ");
      Serial.print("0x");
      Serial.print(i < 16 ? "0" : "");
      Serial.print(i, HEX);
      
      if(i == 0x01 || i == 0x03 || i == 0x06 || i == 0x0B || i == 0x19 || i == 0x31 || i == 0x33 || i == 0x35 || i == 0x37 || i == 0x39 || i == 0x3B || i == 0x3D || i == 0x3F || i == 0x41 || i == 0x43 || i == 0x45){
        ReadFromRegister(bq25792Addr, i, 2);
        int output = (buffer[0] << 8) | buffer[1];
        Serial.print(" = ");
        Serial.print("0x");
        Serial.print(output < 16 ? "0" : "");
        Serial.println(output, HEX);
      } else {
        Serial.print(" = ");
        ReadFromRegister(bq25792Addr, i, 1);
        Serial.print("0x");
        Serial.print(buffer[0] < 16 ? "0" : "");
        Serial.println(buffer[0], HEX);
      }
    }
  }
}

int GetFaultStatus0() {
  ReadFromRegister(bq25792Addr, REG20_FAULT_Status_0, 1);
  return buffer[0];
}

int GetFaultFlag0() {
  ReadFromRegister(bq25792Addr, REG26_FAULT_Flag_0, 1);
  return buffer[0];
}
int GetFaultFlag1() {
  ReadFromRegister(bq25792Addr, REG27_FAULT_Flag_1, 1);
  return buffer[0];
}

int GetFaultStatus1() {
  ReadFromRegister(bq25792Addr, REG21_FAULT_Status_1, 1);
  return buffer[0];
}

int GetChargerStatus3() {
  ReadFromRegister(bq25792Addr, REG1E_Charger_Status_3, 2);
  return (buffer[1] << 8) | buffer[0];
}

int GetChargeControl4() {
  ReadFromRegister(bq25792Addr, REG13_Charger_Control_4, 1);

  return buffer[0];
}

bool GetACDRV1Status(){
  ReadFromRegister(bq25792Addr, REG13_Charger_Control_4, 1);
  return (buffer[0] & EN_ACDRV1Mask) != 0;
}

bool GetACDRV2Status(){
  ReadFromRegister(bq25792Addr, REG13_Charger_Control_4, 1);
  return (buffer[0] & EN_ACDRV2Mask) != 0;
}

int GetChargeStatus() {
  ReadFromRegister(bq25792Addr, REG1C_Charger_Status_1, 1);

  return buffer[0];// & ChargerStatus1ChargeStatusMask;
}

int GetVbusStatus() {
  ReadFromRegister(bq25792Addr, REG1C_Charger_Status_1, 1);

  return buffer[0] & ChargerStatus1VbusStatusMask;
}

bool GetBc12Detection() {
  ReadFromRegister(bq25792Addr, REG1C_Charger_Status_1, 1);

  return (buffer[0] & ChargerStatus1Bc12DetectionMask) == 1;
}

int GetChargerStatus2() {
  ReadFromRegister(bq25792Addr, REG1D_Charger_Status_2, 1);

  return buffer[0];
}

int GetIcoStatus() {
  ReadFromRegister(bq25792Addr, REG1D_Charger_Status_2, 1);

  return buffer[0] & ChargerStatus3IcoStatusMask;
}

int GetChargeVoltage() { //Not Charge voltage?
  ReadFromRegister(bq25792Addr, REG17_NTC_Control_0, 1);

  return buffer[0] & NtcControl0ChargerVoltageMask;
}

int GetChargeVoltageLimit() {
  ReadFromRegister(bq25792Addr, REG01_Charge_Voltage_Limit, 2);

  int vbus = (buffer[0] << 8) | buffer[1];

  return vbus * ChargeVoltageStep; //Millivolts
}


int GetChargeCurrentLowTempRange() {
  ReadFromRegister(bq25792Addr, REG17_NTC_Control_0, 1);

  // need to shift 1 position to the right to get the value
  return (buffer[0] & NtcControl0ChargeCurrentMask) >> 1;
}

int GetChargeCurrentLimit() {
  ReadFromRegister(bq25792Addr, REG03_Charge_Current_Limit, 2);

  int vbus = (buffer[0] << 8) | buffer[1];

  return vbus * ChargeInputCurrentStep; //Milliamps
}

int GetInputCurrentLimit() {
  ReadFromRegister(bq25792Addr, REG06_Input_Current_Limit, 2);

  int vbus = (buffer[0] << 8) | buffer[1];

  return vbus * ChargeInputCurrentStep; //Milliamps
}

int GetThresholdFastCharge() {
  ReadFromRegister(bq25792Addr, REG08_Precharge_Control, 1);

  return buffer[0] >> 6;
}

int GetInputVoltageLimit() {
  ReadFromRegister(bq25792Addr, REG05_Input_Voltage_Limit, 1);

  return buffer[0] * InputVoltageStep; //Millivolts
}

bool GetIgnoreTempSensor() {
  ReadFromRegister(bq25792Addr, REG18_NTC_Control_1, 1);

  return (buffer[0] & NtcControl1IgnoreTSMask) == 1;
}

int GetOtgColdTempThreshold() {
  ReadFromRegister(bq25792Addr, REG18_NTC_Control_1, 1);

  return buffer[0] & NtcControl1OtgColdTemperatureMask;
}

int GetVt2ComparatorVoltage() {
  ReadFromRegister(bq25792Addr, REG18_NTC_Control_1, 1);

  return buffer[0] & NtcControl1Vt2ComparatorVoltageMask;
}

int GetChargeCurrentHighTempRange() {
  ReadFromRegister(bq25792Addr, REG17_NTC_Control_0, 1);
  
  // need to shift 3 positions to the right to get the value
  return (buffer[0] & NtcControl0ChargeCurrentMask) >> 3;
}

int GetVt3ComparatorVoltage() {
  ReadFromRegister(bq25792Addr, REG18_NTC_Control_1, 1);

  return buffer[0] & NtcControl1Vt3ComparatorVoltageMask;
}

int GetOtgHotTempThreshold() {
  ReadFromRegister(bq25792Addr, REG18_NTC_Control_1, 1);

  return buffer[0] & NtcControl1OtgHotTemperatureMask;
}

int GetMinimalSystemVoltage() {
  ReadFromRegister(bq25792Addr, REG00_Minimal_System_Voltage, 1);

  return (buffer[0] * StepMinimalSystemVoltage) + FixedOffsetMinimalSystemVoltage; //Millivolts
}

int GetPrechargeCurrentLimit() {
  ReadFromRegister(bq25792Addr, REG08_Precharge_Control, 1);

  return buffer[0] & PrechargeCurrentLimitMask * StepPrechargeCurrentLimit; //Milliamps
}

int GetWatchdogTimerSetting() {
  ReadFromRegister(bq25792Addr, REG10_Charger_Control_1, 1);

  return buffer[0] & ChargerControl1WatchdogMask;
}

bool GetAdcEnable() {
  ReadFromRegister(bq25792Addr, REG2E_ADC_Control, 1);

  return (buffer[0] & AdcControlEnableMask) != 0;
}

int GetAdcConversionRate() {
  ReadFromRegister(bq25792Addr, REG2E_ADC_Control, 1);

  return buffer[0] & AdcControlConversionRateMask;
}

int GetAdcResolution() {
  ReadFromRegister(bq25792Addr, REG2E_ADC_Control, 1);

  return buffer[0] & AdcControlResolutionMask;
}

int GetAdcAveraging() {
  ReadFromRegister(bq25792Addr, REG2E_ADC_Control, 1);

  return buffer[0] & AdcControlAverageControlMask;
}

int GetAdcInitialAverage() {
  ReadFromRegister(bq25792Addr, REG2E_ADC_Control, 1);

  return buffer[0] & AdcControlInitialAverageMask;
}

int GetThermalRegulationThreshold() {
  ReadFromRegister(bq25792Addr, REG16_Temperature_Control, 1);

  return buffer[0] & ThermalRegulationThresholdMask;
}

int GetThermalShutdownThreshold() {
  ReadFromRegister(bq25792Addr, REG16_Temperature_Control, 1);

  return (buffer[0] & ThermalShutdownThresholdMask) >> 2;
}

int GetDieTemperature() {
  ReadFromRegister(bq25792Addr, REG41_TDIE_ADC, 2);

  int ascReading = (buffer[0] << 8) | buffer[1];

  return ascReading * TdieStemp; //Degrees C
}

int GetAdcVBUS(){
  ReadFromRegister(bq25792Addr, REG35_VBUS_ADC, 2);

  int adcVBUS = (buffer[0] << 8) | buffer[1];

  return adcVBUS * VbusAdcStep; //Millivolts
}

int GetAdcVAC1(){
  ReadFromRegister(bq25792Addr, REG37_VAC1_ADC, 2);

  int adcVBUS = (buffer[0] << 8) | buffer[1];

  return adcVBUS * VbusAdcStep; //Millivolts
}

int GetAdcVAC2(){
  ReadFromRegister(bq25792Addr, REG39_VAC2_ADC, 2);

  int adcVBUS = (buffer[0] << 8) | buffer[1];

  return adcVBUS * VbusAdcStep; //Millivolts
}

int GetAdcVBAT(){
  ReadFromRegister(bq25792Addr, REG3B_VBAT_ADC, 2);

  int adcVBUS = (buffer[0] << 8) | buffer[1];

  return adcVBUS * VbusAdcStep; //Millivolts
}

bool GetChargeEN(){
  ReadFromRegister(bq25792Addr, REG0F_Charger_Control_0, 1);

  return (buffer[0] & ChargeENMask) != 0;
}

void SetEN_EXTILIM(bool value){
  ReadFromRegister(bq25792Addr, REG14_Charger_Control_5, 1);
  //Serial.println(buffer[0]);
  if(value){
    buffer[0] = buffer[0] | EN_EXTILIMMask;
  } else {
    buffer[0] = buffer[0] & ~EN_EXTILIMMask;
  }
  
  //Serial.println(buffer[0]);
  //buffer[0] |= (byte)value;

  //Serial.println(buffer[0]);

  WriteToRegister(bq25792Addr, REG14_Charger_Control_5, 1); //EN_EXTILIMMask)

}

void SetAdcEnable(bool value) {
  // read existing content
  ReadFromRegister(bq25792Addr, REG2E_ADC_Control, 1);

  // clear bit
  buffer[0] = (byte)(buffer[0] & ~AdcControlEnableMask);

  // set bit if needed
  if (value) {
      buffer[0] |= AdcControlEnableMask;
  }

  WriteToRegister(bq25792Addr, REG2E_ADC_Control, 1);
}

void SetChargeVoltage(int value) {
  // read existing content
  ReadFromRegister(bq25792Addr, REG17_NTC_Control_0, 1);

  // clear bits
  buffer[0] = (byte)(buffer[0] & ~NtcControl0ChargerVoltageMask);

  // set value
  buffer[0] |= (byte)value;

  WriteToRegister(bq25792Addr, REG17_NTC_Control_0, 1);
}

void SetChargeCurrentHighTempRange(int value) {
  // read existing content
  ReadFromRegister(bq25792Addr, REG17_NTC_Control_0, 1);

  // clear bits
  buffer[0] = (byte)(buffer[0] & ~NtcControl0ChargeCurrentMask);

  // set value
  // need to shift 3 positions to the left to get the value into the correct position
  buffer[0] |= (byte)((byte)value << 3);

  WriteToRegister(bq25792Addr, REG17_NTC_Control_0, 1);
}

void SetChargeCurrentLowTempRange(int value) {
  // read existing content
  ReadFromRegister(bq25792Addr, REG17_NTC_Control_0, 1);

  // clear bits
  buffer[0] = (byte)(buffer[0] & ~NtcControl0ChargeCurrentMask);

  // set value
  // need to shift 1 position to the left to get the value into the correct position
  buffer[0] |= (byte)((byte)value << 1);

  WriteToRegister(bq25792Addr, REG17_NTC_Control_0, 1);
}

void SetVt2ComparatorVoltage(int value) {
  // read existing content
  ReadFromRegister(bq25792Addr, REG18_NTC_Control_1, 1);

  // clear bits
  buffer[0] = (byte)(buffer[0] & ~NtcControl1Vt2ComparatorVoltageMask);

  // set value
  buffer[0] |= (byte)value;

  WriteToRegister(bq25792Addr, REG18_NTC_Control_1, 1);
}

void SetVt3ComparatorVoltage(int value) {
  // read existing content
  ReadFromRegister(bq25792Addr, REG18_NTC_Control_1, 1);

  // clear bits
  buffer[0] = (byte)(buffer[0] & ~NtcControl1Vt3ComparatorVoltageMask);

  // set value
  buffer[0] |= (byte)value;

  WriteToRegister(bq25792Addr, REG18_NTC_Control_1, 1);
}

void SetOtgHotTempThreshold(int value) {
  // read existing content
  ReadFromRegister(bq25792Addr, REG18_NTC_Control_1, 1);

  // clear bits
  buffer[0] = (byte)(buffer[0] & ~NtcControl1OtgHotTemperatureMask);

  // set value
  buffer[0] |= (byte)value;

  WriteToRegister(bq25792Addr, REG18_NTC_Control_1, 1);
}

void SetOtgColdTempThreshold(int value) {
  // read existing content
  ReadFromRegister(bq25792Addr, REG18_NTC_Control_1, 1);

  // clear bits
  buffer[0] = (byte)(buffer[0] & ~NtcControl1OtgColdTemperatureMask);

  // set value
  buffer[0] |= (byte)value;

  WriteToRegister(bq25792Addr, REG18_NTC_Control_1, 1);
}

void SetIgnoreTempSensor(bool value) {
  // read existing content
  ReadFromRegister(bq25792Addr, REG18_NTC_Control_1, 1);

  // clear bits
  buffer[0] = (byte)(buffer[0] & ~NtcControl1IgnoreTSMask);

  // set value
  buffer[0] |= (byte)(value ? 0b0001 : 0b0000);

  WriteToRegister(bq25792Addr, REG18_NTC_Control_1, 1);
}

bool SetMinimalSystemVoltage(int value) {
  // sanity check
  if (value < FixedOffsetMinimalSystemVoltage || value > MaxValueMinimalSystemVoltage) {
      return false;
  }

  // read existing content
  //ReadFromRegister(bq25792Addr, REG00_Minimal_System_Voltage, 1);

  // divide by step value, as the register takes the value as 240mV steps
  int newValue = value / StepMinimalSystemVoltage;

  // process value to replace VSYSMIN_5:0
  // no need to mask as the value has to be already 6 bits wide
  buffer[0] = newValue;

  WriteToRegister(bq25792Addr, REG00_Minimal_System_Voltage, 1);
  return true;
}

bool SetChargeVoltageLimit(int value) {
  // sanity check
  if (value < FixedOffsetMinimalChargeVoltageLimit || value > MaxValueChargeVoltageLimit) {
      return false;
  }

  // divide by step value, as the register takes the value as 10mV steps
  int newValue = value / ChargeVoltageStep;

  // process value 
  buffer[0] = lowByte(newValue);
  buffer[1] = highByte(newValue);

  WriteToRegister(bq25792Addr, REG01_Charge_Voltage_Limit, 2);
  return true;
}

bool SetChargeCurrentLimit(int value) {
  // sanity check
  if (value < FixedOffsetMinimalChargeCurrentLimit || value > MaxValueChargeCurrentLimit) {
      return false;
  }

  // divide by step value, as the register takes the value as 10mA steps
  int newValue = value / ChargeInputCurrentStep;

  buffer[1] = lowByte(newValue);
  buffer[0] = highByte(newValue);

  Serial.print("buffer[0] = ");
  Serial.print("0x");
  Serial.print(buffer[0] < 16 ? "0" : "");
  Serial.println(buffer[0], HEX);
  //Serial.println(buffer[0]);
  Serial.print("buffer[1] = ");
  Serial.print("0x");
  Serial.print(buffer[1] < 16 ? "0" : "");
  Serial.println(buffer[1], HEX);

  WriteToRegister(bq25792Addr, REG03_Charge_Current_Limit, 2);
  return true;
}

bool SetInputVoltageLimit(int value) {
  // sanity check
  if (value < MinValueInputVoltageLimit || value > MaxValueInputVoltageLimit) {
      return false;
  }

  // divide by step value, as the register takes the value as 100mV steps
  int newValue = value / InputVoltageStep;
  buffer[0] = newValue;

  WriteToRegister(bq25792Addr, REG05_Input_Voltage_Limit, 1);
  return true;
}

bool SetInputCurrentLimit(int value) {
  // sanity check
  if (value < FixedOffsetMinimalInputCurrentLimit || value > MaxValueInputCurrentLimit) {
      return false;
  }

  // divide by step value, as the register takes the value as 10mA steps
  int newValue = value / ChargeInputCurrentStep;

  buffer[1] = lowByte(newValue);
  buffer[0] = highByte(newValue);

  WriteToRegister(bq25792Addr, REG06_Input_Current_Limit, 2);
  return true;
}

void SetWatchdogTimerSetting(int value) {
  // read existing content
  ReadFromRegister(bq25792Addr, REG10_Charger_Control_1, 1);

  buffer[0] = (byte)(buffer[0] & ~ChargerControl1WatchdogMask);
  buffer[0] |= (byte)value;

  WriteToRegister(bq25792Addr, REG10_Charger_Control_1, 1);
}

void SetAdcConversionRate(int value) {
  // read existing content
  ReadFromRegister(bq25792Addr, REG2E_ADC_Control, 1);

  // clear bit
  buffer[0] = (byte)(buffer[0] & ~AdcControlConversionRateMask);
  buffer[0] |= (byte)value;

  WriteToRegister(bq25792Addr, REG2E_ADC_Control, 1);
}

void SetAdcResolution(int value) {
  // read existing content
  ReadFromRegister(bq25792Addr, REG2E_ADC_Control, 1);

  // clear bits
  buffer[0] = (byte)(buffer[0] & ~AdcControlResolutionMask);
  buffer[0] |= (byte)value;

  WriteToRegister(bq25792Addr, REG2E_ADC_Control, 1);
}

void SetAdcAveraging(int value) {
  // read existing content
  ReadFromRegister(bq25792Addr, REG2E_ADC_Control, 1);

  // clear bits
  buffer[0] = (byte)(buffer[0] & ~AdcControlAverageControlMask);
  buffer[0] |= (byte)value;

  WriteToRegister(bq25792Addr, REG2E_ADC_Control, 1);
}

void SetAdcInitialAverage(int value) {
  // read existing content
  ReadFromRegister(bq25792Addr, REG2E_ADC_Control, 1);

  // clear bits
  buffer[0] = (byte)(buffer[0] & ~AdcControlInitialAverageMask);
  buffer[0] |= (byte)value;

  WriteToRegister(bq25792Addr, REG2E_ADC_Control, 1);
}

void SetThermalRegulationThreshold(int value) {
  // read existing content
  ReadFromRegister(bq25792Addr, REG16_Temperature_Control, 1);

  // clear bits
  buffer[0] = (byte)(buffer[0] & ~ThermalRegulationThresholdMask);
  buffer[0] |= (byte)value;

  WriteToRegister(bq25792Addr, REG16_Temperature_Control, 1);
}

void SetThermalShutdownThreshold(int value) {
  // read existing content
  ReadFromRegister(bq25792Addr, REG16_Temperature_Control, 1);

  // clear bits
  buffer[0] = (byte)(buffer[0] & ~ThermalShutdownThresholdMask);
  buffer[0] |= (byte)((byte)value << 2);

  WriteToRegister(bq25792Addr, REG16_Temperature_Control, 1);
}

void SetThresholdFastCharge(int value) { //Incomplete
  // read existing content
  ReadFromRegister(bq25792Addr, REG08_Precharge_Control, 1);

  // process value to replace VBAT_LOWV_1:0
  buffer[0] = (byte)(((byte)value << 6) | (byte)(buffer[0] & 63));//0b0011_1111));
}

bool SetPrechargeCurrentLimit(int value) { //Incomplete
  // sanity check
  if (value < PrechargeCurrentLimitMinValue || value > PrechargeCurrentLimitMaxValue) {
      return false;
  }

  // read existing content
  ReadFromRegister(bq25792Addr, REG08_Precharge_Control, 1);

  // divide by 40 as the register takes the value as 40mA steps
  int newValue = value / StepPrechargeCurrentLimit;

  // process value to replace IPRECHG_5:0
  // no need to mask as the value has to be already 6 bits wide
  buffer[0] |= (byte)newValue;
}