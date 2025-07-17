//Max17261 register addresses
byte max17261Addr = 0x36;
static int Status = 0x00;
static int VCell = 0x09;
//static int AvgVCell = 0x19; Unused
static int Current = 0x0A;
static int AvgCurrent = 0x0B;
static int RepSOC = 0x06;
static int RepCap = 0x05;
static int FullCapRep = 0x10;
static int TimeToEmpty = 0x11;
static int TimeToFull = 0x20;
static int DesignCap = 0x18;

//Battery Configuration
float ChargeVoltage = 4.2;        //Maximum charge voltage per cell. Used for SOC calculations, this will NOT change the actual charge voltage of the battery.
int max17261_Status = 0;          //Fuel Gauge Status stored here
int lowVoltageThreshold = 3000;  //Empty voltage threshold. Millivolts. Used for SOC calculation only
float chargeTerminationCurrent = 0.2;

//Multiplication factors for fuel gauge info
static float capacity_multiplier_mAH = (5e-3)/0.01; //0.5
static float current_multiplier_mV = 0.00015625;
static float voltage_multiplier_V = 7.8125e-5;
static float time_multiplier_Hours = 5.625/3600.0;
static float percentage_multiplier = 1.0/256.0; //0.00390625

void WriteRegister(int addr, int reg, int value){ //Write Fuel gauge i2c register
  Wire.beginTransmission(addr);
  Wire.write(reg);
  //Wire.write(highByte(value));
  Wire.write(lowByte(value));
  Wire.write(highByte(value));
  Wire.endTransmission();
}

unsigned int ReadRegister(int addr, int reg){ //Read fuel gauge i2c register
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(addr,2,true);
  byte temp[2];
  int count = 0;
  while (Wire.available()) {
    temp[count] = Wire.read();
    count++;
  }
  return(word(temp[1], temp[0]));
}

//Define vempty reg
#define MAX17261_VEMPTY_REG(ve_mv, vr_mv)\
  (((ve_mv / 10) << 7) | (vr_mv / 40))

float getSOC() { //Returns state of charge as a percentage.
  unsigned int SOC_raw = ReadRegister(max17261Addr, RepSOC);
  return SOC_raw * percentage_multiplier;
}

int getCapacity() { //Returns configured battery capacity in mAh
  unsigned int capacity_raw = ReadRegister(max17261Addr, DesignCap);
  return capacity_raw * capacity_multiplier_mAH;
}

int getRealCapacity() { //Returns configured battery capacity in mAh
  unsigned int capacity_raw = ReadRegister(max17261Addr, FullCapRep);
  return capacity_raw * capacity_multiplier_mAH;
}

int getRemainingCapacity() { //Returns remaining capacity in mAh
  unsigned int remainingCapacity_raw = ReadRegister(max17261Addr, RepCap);
  return remainingCapacity_raw * capacity_multiplier_mAH;
}

float getInstantaneousCurrent() { //Returns current in Amps
  unsigned int current_raw = ReadRegister(max17261Addr, Current);
  return current_raw * current_multiplier_mV;
}

float getAverageCurrent() { //Returns Average current in Amps
  unsigned int current_raw = ReadRegister(max17261Addr, AvgCurrent);
  return current_raw * current_multiplier_mV;
}

float getInstantaneousVoltage() { //Returns voltage in volts
  unsigned int voltage_raw = ReadRegister(max17261Addr, VCell);
  return voltage_raw * voltage_multiplier_V;
}

float getTimeToEmpty() { //Returns time till empty as a decimal. Example 1.75 = 1 hour and 45 minutes
  unsigned int TTE_raw = ReadRegister(max17261Addr, TimeToEmpty);
  return TTE_raw * time_multiplier_Hours;
}

float getTimeToFull() { //Returns time till empty as a decimal. Example 1.75 = 1 hour and 45 minutes
  unsigned int timeRaw = ReadRegister(max17261Addr, TimeToFull);
  return timeRaw * time_multiplier_Hours;
}

uint16_t getVEmpty() { //Returns configured empty voltage
  unsigned int VEmpty_raw = ReadRegister(max17261Addr, 0x3A);
  return (((VEmpty_raw >> 7) & 0x1FF) * 10);
}

void max17261Init(){ //Fuel gauge initialisation
  StatusPOR = ReadRegister(max17261Addr,0x00) & 0x0002;
  //int currentCapacity;

  //StatusPOR = 1;
  if(debug){
    Serial.print("Status POR = ");
    Serial.println(StatusPOR);
  }

  /*if(GetInputCurrentLimit() != 3000){ //Detect First start by checking if BQ25792 input current limit has been configured.
    forceFuelGaugeReset = true;
  }*/

  if(forceFuelGaugeReset){
    StatusPOR = 1;
  }
  
  if(StatusPOR == 0){
    getCapacity();
    getSOC();
  } else {
    updateBQ = true;
    while (ReadRegister(max17261Addr, 0x3D) & 1) {
      delay(10);
    }
    int HibCFG = ReadRegister(max17261Addr, 0xBA); //Store original HibCFG value
    WriteRegister(max17261Addr, 0x60, 0x90); //Exit Hibernate Mode Step 1
    WriteRegister(max17261Addr, 0xBA, 0x0); //Exit Hibernate Mode Step 2
    WriteRegister(max17261Addr, 0x60, 0x0); //Exit Hibernate Mode Step 3

    WriteRegister(max17261Addr, 0x18, (batteryCapacity / capacity_multiplier_mAH)); //Write DesignCap
    WriteRegister(max17261Addr, 0x1E, chargeTerminationCurrent / current_multiplier_mV); //0x666); //256mA); //Write IchgTerm
    WriteRegister(max17261Addr, 0x3A, MAX17261_VEMPTY_REG(lowVoltageThreshold, 3880)); //3.1V //Write VEmpty
    
    WriteRegister(max17261Addr, 0x45, (int)(batteryCapacity / capacity_multiplier_mAH) / 32); //Write dQAcc
    
    if (ChargeVoltage > 4.275) {
      WriteRegister(max17261Addr, 0x46, (int)((batteryCapacity / capacity_multiplier_mAH) / 32) * 51200 / (batteryCapacity / capacity_multiplier_mAH));
      WriteRegister(max17261Addr, 0xDB, 0x8400);
    } else {
      WriteRegister(max17261Addr, 0x46, (int)((batteryCapacity / capacity_multiplier_mAH) / 32) * 44138 / (batteryCapacity / capacity_multiplier_mAH));
      WriteRegister(max17261Addr, 0xDB, 0x8000);
    }

    while (ReadRegister(max17261Addr, 0xDB) & 0x8000) {
      delay(10);
    }

    WriteRegister(max17261Addr, 0xBA, HibCFG);

    max17261_Status = ReadRegister(max17261Addr, 0x00);
    WriteRegister(max17261Addr, 0x00, Status & 0xFFFD);
  }
}