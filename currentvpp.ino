//Arduino to NodeMCU Lib
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

//Initialise Arduino to NodeMCU (5=Rx & 6=Tx) for serial communicaton
SoftwareSerial nodemcu(5, 6);


int mVperAmp = 66;     // this the 5A version of the ACS712 -use 100 for 20A Module and 66 for 30A Module
int Watt = 0;
double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;

//declearing and initializing variables to save sensor data
double sensorValue1 = 0;
double sensorValue2 = 0;
int crosscount = 0;
int climb_flag = 0;
int val[100];
int max_v = 0;
double VmaxD = 0;
double VeffD = 0;
double Veff = 0;

//initializing and setting up arduino 
void setup() {
  Serial.begin (9600); 
  nodemcu.begin(9600);
  Serial.println ("Program started"); 
}

//looping contineously to get data and sending those data as json format to NodeMcu through serial communication
void loop() {
  
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  
  data["Theft"] = false; 
  Serial.println ("");

  //getting gas Sensor Value 
  const float gasValue = analogRead(A2);
  if(gasValue > 450){
    data["Gas_status"] = 1;
    Serial.print("Gas Detected :");
    Serial.println(gasValue);
    }
   else{
    data["Gas_status"] = 0;
    }

  //getting current from sensor 1 
  const float sensor1 = returnAmpRMS(A0);
  Serial.print("current sensor one value : ");
  Serial.println(sensor1);
  data["MainSupply_current"] = sensor1;
  
  //getting current from sensor 1 
  const float sensor2 = returnAmpRMS(A1);
  Serial.print("current sensor two value : ");
  Serial.println(sensor2);
  data["HomeConsumption_current"] = sensor2;

  //detecting theft
  if(sensor1-sensor2 > 0.1){
      Serial.println("theft detected");
      data["Theft"] = true;
    }

  //getting voltage
  const float voltage = getVoltage(A3);
  Serial.print("voltage value : ");
  Serial.println(voltage);
  data["voltage"] = voltage;
    
  //Send data to NodeMCU
  data.printTo(nodemcu);
  jsonBuffer.clear();
  delay(2000);

  
}



 
// ******************** function calls **********************

//compute vpp from analog input from corresponding analog pins
float getVPP(char sensorIn)
{
  float result;
  int readValue;                // value read from the sensor
  int maxValue = 0;             // store max value here
  int minValue = 4096;          // store min value here ESP32 ADC resolution
  
   uint32_t start_time = millis();
   while((millis()-start_time) < 1000) //sample for 1 Sec
   {
       readValue = analogRead(sensorIn);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           /*record the minimum sensor value*/
           minValue = readValue;
       }
   }
   
   // Subtract min from max
   result = ((maxValue - minValue) * 5)/4096.0; //ESP32 ADC resolution 4096
      
   return result;
 }

 
//function that returns current and watt from certain input pin through sensorIn parameter
 float returnAmpRMS(char sensorIn ){
  Voltage = getVPP(sensorIn);
  VRMS = (Voltage/2.0) *0.707;   //root 2 is 0.707
  AmpsRMS = ((VRMS * 1000)/mVperAmp)-0.05; //0.3 is the error I got for my sensor
  Watt = (AmpsRMS*240/1.2);
  // note: 1.2 is my own empirically established calibration factor
  delay (200);
  return AmpsRMS;
  }

  float getVoltage(char sensorIn){
    for ( int i = 0; i < 100; i++ ) {
      sensorValue1 = analogRead(sensorIn);
      if (analogRead(sensorIn) > 511) {
        val[i] = sensorValue1;
      }
      else {
        val[i] = 0;
      }
      delay(1);
    }

    max_v = 0;

    for ( int i = 0; i < 100; i++ )
    {
      if ( val[i] > max_v )
      {
        max_v = val[i];
      }
      val[i] = 0;
    }
    if (max_v != 0) {
    VmaxD = max_v;
    VeffD = VmaxD / sqrt(2);
    Veff = (((VeffD - 420.76) / -90.24) * -210.2) + 210.2;
    }
    else {
    Veff = 0;
    }
    
    //Serial.print("Voltage: ");
    //Serial.println(Veff);
    VmaxD = 0;
    delay(100);
    return Veff;
    }
