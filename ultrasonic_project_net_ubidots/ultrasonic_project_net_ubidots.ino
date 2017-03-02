#include <LBattery.h>
#include <LGPRS.h>
#include <LGPRSClient.h>
#include <Wire.h>
#include "DHT.h"  
#include <ADXL345.h>

/*
//DHT Library can be downloaded from:
https://github.com/Seeed-Studio/Grove_Starter_Kit_For_LinkIt/tree/master/libraries/Humidity_Temperature_Sensor
*/

/*
//Accelerometer ADXL345 Library can be downloaded from:
http://www.seeedstudio.com/wiki/File:DigitalAccelerometer_ADXL345.zip
*/

//code for testing Ultrasonic Module
#define ECHOPIN 11        // Digital Pin 11 to receive echo pulse 
#define TRIGPIN 12        // Digital Pin 12 to send trigger pulse

//Ubidots account data
#define URL    "things.ubidots.com"
#define TOKEN  ""     // replace with your Ubidots token generated in your profile tab
#define VARID1 ""    // create a Trash level variable in Ubidots and put its ID here
#define VARID2 ""   //temperature of trash can
#define VARID3 ""  //activity tracking
#define VARID4 ""  //battery level tracking
#define VARID5 ""  //RFID card number variable

// Reading temperature or humidity takes about 250 milliseconds!
// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)  
#define DHTPIN 2     // Digital pin 2 where DHT 22 is connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);

//Accelerometer connected to I2C of LinkIT One
ADXL345 accel; //variable accel is an instance of the accel345 library

char buff[256];  //Info about battery level and charging

//Temperature and Humidity sensor readings from DHT 22
float t = 0.0;
float h = 0.0;

//Location where the prototype is placed!!
String loclat="19.21833";
String loclng="72.978088";
String Location;

//RFID card data 
uint8_t count=0;               //No. of characters of RFID card number
char RFID_input[12];          // character array of size 12 to store the Tag ID being presented
char tagOk[] ="1500239EA50D"; // Authorised RFID card number of garbage collector


unsigned long ReportingInterval = 20000;  // How often do you want to update the IOT site in milliseconds (in this case 20 seconds)
unsigned long LastReport = 0;      // When was the last time you reported


//Accelerometer adxl345 outputs    
int x_initial,y_initial,z_initial;
double xyz_initial[3];
double ax_initial,ay_initial,az_initial;

LGPRSClient client;  //GPRS Client

void setup()
{
  Serial1.begin(9600);  //RFID reader module tx is connected to D0 (Serial1)
  Serial.begin(9600);    //For serial debugging on Laptop/computer
  Serial.println("Smart Trash Can Project by Er.Amol");
  accel.powerOn();       //Powering Accelerometer
  dht.begin();           //Initializing DHT 22 sensor

  //Initializing Ultrasonic sensor pins
  pinMode(ECHOPIN, INPUT);   
  pinMode(TRIGPIN, OUTPUT);  

  axis_initialize(); //Initializing Accelerometer
  
  //Location String containing lat and long data as required by Ubidots API
  //For more info: http://ubidots.com/docs/get_started/quickstart/tutorial_geopoint.html
  Location="{\"lat\":";
  Location=Location+loclat;
  Location += " ,\"lng\":";
  Location=Location+loclng+ "}";
  
  Serial.println("Attach to GPRS network by APN setting");
  while (!LGPRS.attachGPRS("APN name","UserName","Password")) //Enter correct Access Point Name (APN) according to your GSM provider. Username and password if not present then keep it empty.
  {
    delay(500);
    Serial.println("Smart Trash Can Project by Er.Amol");
  }
  
  LGPRSClient globalclient ; //Client has to be initiated after GPRS is established with the correct APN settings:
  client = globalclient;     //Again this is a temporary solution described in support forums
  
  Serial.println("Smart Trash Can Project by Er.Amol");
  delay(2000);
}

void loop()
{
  count=0;                     //Initialize the counter
  while(Serial1.available())   //Data available or not from RFID reader module
  {
     RFIDRead_Check();     //Reading RFID card number and checking whether it is read correctly and then it is authorised or not
   }
 
  if (millis() >= LastReport + ReportingInterval)   //Send data after about ReportingInterval (i.e.20 seconds)
  {
  /*
  For more information on Grove sensors:
  http://www.seeedstudio.com/wiki/Grove_System
  */
	  
  delay(2000);  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  dht.readHT(&t, &h);   //Reading Temperature and humidity from DHT 22
  
  Serial.println("------------------------------");
  Serial.print("Temperature = ");
  Serial.println(t);
  
  
  sprintf(buff,"Battery level = %d", LBattery.level() );
  Serial.println(buff);


  // Start Ranging -Generating a trigger of 10us burst 
  digitalWrite(TRIGPIN, LOW); 
  delayMicroseconds(2); 
  digitalWrite(TRIGPIN, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(TRIGPIN, LOW);
  // Distance Calculation
  
  float distance = pulseIn(ECHOPIN, HIGH); 
  distance= distance/58; 
  distance=(int)distance;
  distance = map(distance, 0, 500, 0, 100);
  
/* The speed of sound is 340 m/s or 29 us per cm.The Ultrasonic burst travels out & back.So to find the distance of object we divide by 58  */
  Serial.print("Garbage Level: "); 
  Serial.print(distance); 
  Serial.println(" cm");
  delay(200); 
  
  //Check whether any unauthorised activity is occuring with the device!!
  int activity=0;
  activity=compareResult();
  if(activity==1)
  {
    Serial.println("There is unauthorised activity...");
  }
  else Serial.println("No Activity"); 
  delay(500);
   
  //String containing all the sensors data according to collection endpoint API of Ubidots
  ////Build the JSON packet according to the format needed by Ubidots
  //For more info: http://ubidots.com/docs/api/v1_6/collections/post_values.html
  String payload = "[{\"variable\":\"" VARID2 "\",\"value\":" + String(valu) + "},{\"variable\":\"" VARID3 "\",\"value\":" + String(activity) + "}]";
  String le = String(payload.length());  // How long is the payload
  
  //For sending data to Ubidots: http://ubidots.com/docs/api/index.html
  // if you get a connection, report back via serial:
  Serial.print("Connecting to ");
  Serial.println(URL);
  if (client.connect(URL, 80))
  {
    // Build HTTP POST request
    Serial.println("connected");    
    client.print(F("POST /api/v1.6/collections/values/?token="));
    client.print(TOKEN);
    client.println(F(" HTTP/1.1"));
    client.println(F("Content-Type: application/json"));
    client.print(F("Content-Length: "));
    client.println(le);
    client.print(F("Host: "));
    client.println(URL);
    client.println(); 
    client.println(payload);
    client.println(); 
    client.println((char)26); //This terminates the JSON SEND with a carriage return
  }
  else
  {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
  delay(100);
  // if there are incoming bytes available
  // from the server, read them and print them:
  if (client.available())
  {
    char c = client.read();
    Serial.print(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.available() && !client.connected())
  {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }
  LastReport = millis();  
  }
}

void RFIDRead_Check()
{
    while(Serial1.available() && count < 12)          // Read 12 characters and store them in input array
      {
         RFID_input[count] = Serial1.read();
         count++;
         delay(5);
      }

  //For more info on EM18 Reader Module: https://electrosome.com/em-18-rfid-reader-arduino-uno/
  //Check if RFID card number is read correctly without any error
	  if((RFID_input[0] ^ RFID_input[2] ^ RFID_input[4] ^ RFID_input[6] ^ RFID_input[8] == RFID_input[10]) && (RFID_input[1] ^ RFID_input[3] ^ RFID_input[5] ^ RFID_input[7] ^ RFID_input[9] == RFID_input[11]))
  { 
    RFIDSend_ubidots();   //Check if authorised or not and if authorised then Send RFID card number to ubidots 
  }
  else
  {
  Serial.println("RFID read Error"); 
  }
}

void RFIDSend_ubidots()
{
  int compare = 1; 
  compare = (strncmp(tagOk, RFID_input,12)) ; // if both tags are equal strncmp returns a 0
  if (compare == 0) //If both string matches i.e.authorised rfid card is read then send data to ubidots
  {
  Serial.println("------------------------------");
  Serial.println("RFID matched!!");
  
  //String containing all the sensors data according to collection endpoint API of Ubidots
  ////Build the JSON packet according to the format needed by Ubidots
  //For more info: http://ubidots.com/docs/api/v1_6/collections/post_values.html
  String payload= "{\"RFID_Name\":";
  payload=payload +"\""+ tagOk +"\""+ "}";
  payload = "[{\"variable\":\"" VARID5 "\",\"value\":"+ String(count)+",\"context\":"+payload+"}]";
  
  String le = String(payload.length());  //Length of payload
  
  //For sending data to Ubidots: http://ubidots.com/docs/api/index.html
  // if you get a connection, report back via serial:
  Serial.print("Connect to ");
  Serial.println(URL);
  delay(2000);
  if (client.connect(URL, 80))  //Connnecting to ubidots!!
  {
    // Build HTTP POST request
    Serial.println("connected");    
    client.print(F("POST /api/v1.6/collections/values/?token="));
    client.print(TOKEN);
    client.println(F(" HTTP/1.1"));
    client.println(F("Content-Type: application/json"));
    client.print(F("Content-Length: "));
    client.println(le);
    client.print(F("Host: "));
    client.println(URL);
    client.println(); 
    client.println(payload);
    client.println();
    client.println(char(26));
  }
  while(client.available())
  {
    char c = client.read();
    Serial.print(c);
  } 
  }
}

//Function to Store initial axis value of x,y & z for comparision  
void axis_initialize(){  
  accel.readXYZ(&x_initial, &y_initial, &z_initial); //read the accelerometer values and store them in variables  x_initial,y_initial,z_initial
  accel.getAcceleration(xyz_initial);
  ax_initial = xyz_initial[0];
  ay_initial = xyz_initial[1];
  az_initial = xyz_initial[2];
  delay(500);
 }

//To compare initial values with current reading of x,y & z values for theft protection
int compareResult(){
  //Boring accelerometer stuff   
  int x,y,z,Xchange,Ychange,Zchange;  
  accel.readXYZ(&x, &y, &z); //read the accelerometer values and store them in variables  x,y,z
  
  double xyz[3];
  double ax,ay,az;
  accel.getAcceleration(xyz);
  ax = xyz[0];
  ay = xyz[1];
  az = xyz[2];

  if((int)ax!=(int)ax_initial){
      Xchange=1;   
  } 
  else {
      Xchange=0;
  }
  
  if((int)ay!=(int)ay_initial){
      Ychange=1;   
  } 
  else {
      Ychange=0;
  }

  if((int)az!=(int)az_initial){
      Zchange=1;   
  } 
  else {
      Zchange=0;
  }
  
  if((Xchange==1)||(Ychange==1)||(Zchange==1)){
    return 1;
  }
  else return 0; 
}
