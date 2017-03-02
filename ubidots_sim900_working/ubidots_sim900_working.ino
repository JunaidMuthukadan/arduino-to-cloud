//This code sends the distance measured by the untrasonic sensor to ubidots. Make sure you properly connect the pins.
#include <UbidotsArduinoGPRS.h>
#include <SoftwareSerial.h>
#include <JeeLib.h>
#include <String.h>
SoftwareSerial mySerial(7, 8);//pins to serial communication
ISR(WDT_vect) { Sleepy::watchdogEvent(); } // Setup the watchdog

int valor;
const int trigPin = 3;
const int echoPin = 5;
String loclat="12.991889";
String loclng="80.246272";
String Location="{\"lat\":";
  

//-------------------------------------------------------------
//---------------------Ubidots Configuration-------------------
//-------------------------------------------------------------
String TOKEN = "wWcudFMmbtCK3QBoPwoQUlKYNMRMm7";                                    //your token to post a value
String VARID1  = "575e73b076254201f7b46502";                                     //ID of your variable
void setup()
{

  mySerial.begin(19200); //the GPRS baud rate
Serial.begin(19200);  //the serial communication baud rate
   Serial.println("Starting");
  delay(2000);
   Location=Location+loclat;
   Location += " ,\"lng\":";
   Location=Location+loclng+ "}";
   Serial.println("JUnd");
    Serial.println("JUnd");

 Serial.println("JUnd");



}

void loop()
{
     long duration, value;
     pinMode(trigPin, OUTPUT);
     digitalWrite(trigPin, LOW);
     delayMicroseconds(2);
     digitalWrite(trigPin, HIGH);
     delayMicroseconds(10);
     digitalWrite(trigPin, LOW);
     pinMode(echoPin, INPUT);
     duration = pulseIn(echoPin, HIGH);
     value = duration / 29 / 2;
     
     
     
      String mystr="";
  int i = 0;
mySerial.println("AT+CLTS=1");
   delay(2000);
  ShowSerialData();
  mySerial.println("AT+CCLK?");
   
  delay (500);
  while (mySerial.available()>0) {
    mystr += char(mySerial.read());
  }
  Serial.println("My String |"+mystr+"|");
  String clockString = "";
  int w = mystr.indexOf(String('"'))+1;   // Find the first occurance of an open quotation.  This is where we begin to read from
  int x = mystr.lastIndexOf(String('"'));
 
  
 String Rtime=mystr.substring(w,x);
 String time=mystr.substring(29,34);// Find the last occurance of an open quotation. This is where we end.
 String hour=mystr.substring(29,31);

  Serial.println(Rtime);
 Serial.println(time); // This is the time string yy/mm/dd,hh:mm:ss-tz (tz in qtrs)
 Serial.println(hour);
 
 

     Serial.println("Sending to Ubidots Value="+ value);
     
     save_value(String(value));                                                      //call the save_value function
     if (mySerial.available())
     Serial.write(mySerial.read());
     
     
     
      if(hour =="06"){
  // activate sleep mode
mySerial.println("AT+CSCLK=2"); //2:Module decides itself if go to sleep mode if no data on serial port.

delay(500);
ShowSerialData();
   Sleepy::loseSomeTime(54000000);
   Serial.println("Wake up (1) GSM by senging AT");
mySerial.println("AT");
delay(500);
ShowSerialData();
Serial.println("Wake up (2) GSM by senging AT");
mySerial.println("AT");
ShowSerialData();
 
 }
 //confirm wake up from the sleep mode

 // while (Serial.read()<1); // Sit and do nothing*/
  
}

//this function is to send the sensor data to Ubidots, you can see the new value in Ubidots after running this function
void save_value(String value)
{
  int num;
  String le;
  String var;
  //[{"variable":"575e73b076254201f7b46502","value":115,"context":{.246272}}]



  
String payload = "[{\"variable\":\""+ VARID1+ "\",\"value\":"+ String(value)+",\"context\":"+Location+"}]";
Serial.println(payload);
  num=var.length();
  le=String(num);
  for(int i = 0;i<7;i++)
  {
    mySerial.println("AT+CGATT?");                                                   //this is better made repeatedly because it is unstable
    delay(2000);
    ShowSerialData();
  }
  mySerial.println("AT+CSTT=\"internet\","",""");                                    //start task and set the APN
  delay(1000);
  ShowSerialData();
  mySerial.println("AT+CIICR");                                                      //bring up wireless connection
  delay(3000);
  ShowSerialData();
  mySerial.println("AT+CIFSR");                                                      //get local IP adress
  delay(2000);
  ShowSerialData();
  mySerial.println("AT+CIPSPRT=0");

  delay(3000);
  ShowSerialData();
  mySerial.println("AT+CIPSTART=\"tcp\",\"things.ubidots.com\",\"80\"");             //start up connection
  delay(3000);
  ShowSerialData();
  mySerial.println("AT+CIPSEND");                                                    //begin sendiing data to the remote server
  delay(3000);
  ShowSerialData();
  mySerial.print("POST /api/v1.6/collections/values/?token=");
  delay(100);
  ShowSerialData();
  mySerial.println(TOKEN);
  delay(100);
  ShowSerialData();
  mySerial.println(" HTTP/1.1");                                                        
  delay(100);
  ShowSerialData();                                                                     
  mySerial.println("Content-Type: application/json");
  delay(100);
  ShowSerialData();
  mySerial.println("Content-Length: "+le);
  delay(100);
  ShowSerialData();
/*  mySerial.print("X-Auth-Token: ");
  delay(100);
  ShowSerialData();*/

  mySerial.println("Host: things.ubidots.com");
  delay(100);
  ShowSerialData();
  mySerial.println();
  delay(100);
  ShowSerialData();
  Serial.println("hghghg");
  mySerial.print("{\"variable\":\""+ VARID1+ "\",\"value\":"+ String(value)+"}");
  //delay(300);
 // ShowSerialData();
   // mySerial.print(payload);
  delay(100);
  ShowSerialData();
  mySerial.println();
  delay(100);
  ShowSerialData();
  mySerial.println((char)26);
  delay(7000);
  mySerial.println();
  ShowSerialData();
  mySerial.println("AT+CIPCLOSE");                                                //close communication
  delay(1000);
  ShowSerialData();
}

void ShowSerialData()
{
  while(mySerial.available()!=0)
  Serial.write(mySerial.read());
}
