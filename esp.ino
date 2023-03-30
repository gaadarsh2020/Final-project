//including library for nodemcu
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
//#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

//setting up api ID and Access token to connect and authenticate to blynk server  
#define BLYNK_TEMPLATE_ID "TMPL-hJxWqoU"
#define BLYNK_DEVICE_NAME "Theft detection monitoring and automation IOT"
#define BLYNK_AUTH_TOKEN "qmgf5PVT0y8H9NLNfHwvUshTMmV6Sf2f"

//setting up timer to run function on setInterval or on certain interval
BlynkTimer timer;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "........";//Enter your WIFI name
char pass[] = "........";//Enter your WIFI password

//initializing variables to store data from arduino
double GridMainSupply = 0.0;
double HomeSupply = 0.0;
boolean theft = 0;
double voltage = 230;
double temperature = 24;

//D6 = Rx & D5 = Tx for serial communication with nodeMCU
SoftwareSerial nodemcu(D6, D5);

//Define the relay pins for automation
#define relay1 D0
#define relay2 D1
#define relay3 D2
#define relay4 D3



//Get the button values from blynk app using V0 pin and change D0 or relay1 pin accordingly
BLYNK_WRITE(V0) {
  bool value1 = param.asInt();
  // Check these values and turn the relay1 ON and OFF
  if (value1 == 1) {
    digitalWrite(relay1, LOW);
  } else {
    digitalWrite(relay1, HIGH);
  }
}

//Get the button values from blynk app using V1 pin and change D1 or relay2 pin accordingly
BLYNK_WRITE(V1) {
  bool value2 = param.asInt();
  // Check these values and turn the relay2 ON and OFF
  if (value2 == 1) {
    digitalWrite(relay2, LOW);
  } else {
    digitalWrite(relay2, HIGH);
  }
}

//Get the button values from blynk app using V2 pin and change D2 or relay3 pin accordingly
BLYNK_WRITE(V2) {
  bool value3 = param.asInt();
  // Check these values and turn the relay3 ON and OFF
  if (value3 == 1) {
    digitalWrite(relay3, LOW);
  } else {
    digitalWrite(relay3, HIGH);
  }
}

//Get the button values from blynk app using V3 pin and change D3 or relay4 pin accordingly
BLYNK_WRITE(V3) {
  bool value3 = param.asInt();
  // Check these values and turn the relay4 ON and OFF
  if (value3 == 1) {
    digitalWrite(relay4, LOW);
  } else {
    digitalWrite(relay4, HIGH);
  }
}


//send main grid current data to blynk vai Server V6
void SendGridCurrentValue(){
  Blynk.virtualWrite(6,GridMainSupply);
  }

//send Home current data to blynk Server via V4 pin
void SendHomeCurrentValue(){
  Blynk.virtualWrite(4,HomeSupply);
  }

//send theft Indication data to blynk Server via V5 pin
void TheftDetectionLed(){
  if(theft == 1){
    Blynk.virtualWrite(5,255);
    }
    
  else{
    Blynk.virtualWrite(5,0);
    }
  }
//send home supply voltage data to blynk Serever via V7 pin
void sendHomeSupplyVoltage(){
  Blynk.virtualWrite(7,voltage);
  }

//send  temperature data to blynk server via V8 pin
void TemperatureSensing(){
  Blynk.virtualWrite(8,temperature);
  }
//send main if theft detected to corresponding mail address configured in blynk Server or Das
void sendMail(){
  if(theft == 1){
    Blynk.logEvent("theft","Current is being Stolen !!! Theft Detected !!!");
  }  
 }

//void setup to setup nodemcu and initialize pins 
void setup() {
  
  // Initialize Serial port
  Serial.begin(9600);
  nodemcu.begin(9600);

  //initialize pins for input/output
  pinMode(A0,INPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  // Turn OFF the relay
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);

  //initializing blynk server connection with corresponding credentials
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  //setting up function to run on certain interval of time 1000L = 1sec
  timer.setInterval(5000L,sendMail);
  timer.setInterval(1000L, SendGridCurrentValue);
  timer.setInterval(1000L, SendHomeCurrentValue);
  timer.setInterval(1000L, TheftDetectionLed);
  timer.setInterval(1000L,sendHomeSupplyVoltage);
  
  while (!Serial) continue;
}


//running contineously
void loop() {
  //running blynk server 
  Blynk.run();

  //initializing static buffer to store data and parse from nodemcu valriable using serial communication 
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject(nodemcu);

  //checking if json is valid or not if not valid clear the buffer
  if (data == JsonObject::invalid()) {
    //Serial.println("Invalid Json Object");
    jsonBuffer.clear();
    return;
  }

  Serial.println("JSON Object Recieved");

  //writing Json to variable
  GridMainSupply = data["MainSupply_current"];
  HomeSupply = data["HomeConsumption_current"];
  theft = data["Theft"];
  voltage = data["voltage"];
  temperature = data["temperature"]


  //printing values in arduino IDE serial for development purpose
  Serial.print("Recieved MainSupply_current:  ");
  Serial.println(GridMainSupply);
  
  Serial.print("Recieved HomeConsumption_current:  ");
  Serial.println(HomeSupply);

  Serial.print("Recieved Household voltage:  ");
  Serial.println(voltage);
   
  Serial.print("Recieved Theft Status:  ");
  Serial.println(theft);
  Serial.println("-----------------------------------------");

  //running  timer function to contineously run the interval functions
  timer.run();
}
