 #include <GSM.h>

        // PIN Number
        #define PINNUMBER ""

        // APN data
        #define GPRS_APN       "IDEA GPRS" // replace with your GPRS APN
        #define GPRS_LOGIN     ""    // replace with your GPRS login
        #define GPRS_PASSWORD  "" // replace with your GPRS password

        // initialize the library instance
        GSMClient client;
        GPRS gprs;
        GSM gsmAccess;
        uint16_t reset = 0;
       String loclat="12.991889";
       String loclng="80.246272";
       String Location="{\"lat\":";
  
         int distance=10;


        String VARID1  = "5784a9267625421437b4b4ed";   // replace with your Ubidots Variable ID
        String TOKEN = "wWcudFMmbtCK3QBoPwoQUlKYNMRMm7";  // replace with your Ubidots token

        void setup(){
          // initialize serial communications and wait for port to open:
          Serial.begin(9600);
          while (!Serial){
            ; // wait for serial port to connect. Needed for Leonardo only
          }
          Serial.println("Starting Arduino web client.");
          // connection state
          boolean notConnected = true;
          // After starting the modem with GSM.begin()
          // attach the shield to the GPRS network with the APN, login and password
          while (notConnected){
            if ((gsmAccess.begin(PINNUMBER) == GSM_READY) &
                (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY)){
              notConnected = false;
            }else{
              Serial.println("Not connected");
              delay(1000);
            }
          }       
        }

        void loop(){
          //int value = analogRead(A0);
          if(save_value(distance)){
          }
          else{
            reset++;
            if (reset == 10){
              asm volatile ("  jmp 0");    // reset the Arduino board if the data transmission fail
            }
          }       
          // if the server's disconnected, stop the client:
          if (!client.available() && !client.connected()){
            Serial.println();
            Serial.println("disconnecting.");
            client.stop();
            // do nothing forevermore:        
          }
        }
        boolean save_value(int distance){
         // Serial.println("connecting...");        
          //int num=0;
         // String var = "{\"value\":"+ String(distance) + "}";
         // num = var.length();
          // if you get a connection, report back via serial:
          if (client.connect("things.ubidots.com", 80)){
            Serial.println("connected");
            Location=Location+loclat;
           
               Location += " ,\"lng\":";
         Location=Location+loclng+ "}";
     

  
  String payload = "[{\"variable\":\""+ VARID1+ "\",\"value\":"+ String(distance)+",\"context\":"+Location+"}]";
  String le = String(payload.length());  // How long is the payload
  
    client.print(F("POST /api/v1.6/collections/values/?token="));
    client.print(TOKEN);
    client.println(F(" HTTP/1.1"));
    client.println(F("Content-Type: application/json"));
    client.print(F("Content-Length: "));
    client.println(le);
    client.print(F("Host: "));
    client.println("things.ubidots.com");
    client.println(); 
    client.println(payload);
    client.println(); 
    client.println((char)26); //This terminates the JSON SEND with a carriage return﻿     
          }else{
            // if you didn't get a connection to the server:
            Serial.println("connection failed");
            return false;
          } 
            while (client.available())
          {         
            char c = client.read();
            Serial.print(c);
          }
        }
