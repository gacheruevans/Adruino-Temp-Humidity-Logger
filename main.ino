// Include Software Serial library to communicate with GSM
#include <SoftwareSerial.h>

// Include DHT library
#include "DHT.h"

// Pin DHT is connected to
#define DHTPIN 12
  
//Using the DHT 11 sensor
#define DHTTYPE DHT11  

// Create variable to store incoming SMS characters
char Received_SMS;

// Used later it shows if there's the word "DHT"/"MLX" within the received SMS "-1" means they are not found
short DHT_OK=-1;

// Here's the SMS that we gonna send to the phone number, it may contain DHT data or MLX data
String Data_SMS;


// Create global varibales to store temperature and humidity
float t; // temperature in celcius
float f; // temperature in fahrenheit
float h; // humidity

// Configure software serial port
SoftwareSerial SIM800L(7, 8);

// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  SIM800L.begin(9600);
  Serial.begin(9600);
  dht.begin();
  
  // Give time to your GSM shield log on to network
  delay(20000);
  Serial.println("GSM Module ready...");
  ReceiveMode();
}

void loop(){
  String RSMS;
  //We add this new variable String type, and we put it in loop so everytime gets initialized
  //This is where we put the Received SMS, yes above there's Recevied_SMS variable, we use a trick below
  //To concatenate the "char Recevied_SMS" to "String RSMS" which makes the "RSMS" contains the SMS received but as a String
  //The recevied SMS cannot be stored directly as String
  
  while(SIM800L.available()>0){  //When SIM800L sends something to the Arduino... problably the SMS received... if something else it's not a problem
      
  Received_SMS=SIM800L.read();   //"char Received_SMS" is now containing the full SMS received
  Serial.print(Received_SMS);    //Show it on the serial monitor (optional)     
  RSMS.concat(Received_SMS);
  //Serial.print(RSMS.length());  //concatenate "char received_SMS" to RSMS which is "empty"
  DHT_OK=RSMS.indexOf("State");   //And this is why we changed from char to String, it's to be able to use this function "indexOf"
                                  //"indexOf function looks for the substring "x" within the String (here RSMS) and gives us its index or position
                                  //For example if found at the beginning it will give "0" after 1 character it will be "1"
                                  //If it's not found it will give "-1", so the variables are integers
        
    }
    
  if(DHT_OK!=-1) {                        //If "DHT" word is found within the SMS, it means that DHT_OK have other value than -1 so we can proceed
    Serial.println("Found State");        //Shows on the serial monitor "found DHT" (optional)
    float h = dht.readHumidity();         //Read temperature and humidity
    float t = dht.readTemperature();
    Serial.print("Temperature = "); 
    Serial.print(t); 
    Serial.println("*C");
    Serial.print("Humidity = "); 
    Serial.print(h); 
    Serial.println(" %");

    //Prepare the SMS to send, it contains some strings like "DHT" "Temperature" and then the values read.
    Data_SMS = "State\n Temperature = "+String(t,1)+" C"+" \n Humidity ="+ String(h,1)+" %";       
    
    Send_Data();      //This function set the sending SMS mode, prepare the phone number to which we gonna send, and send "Data_SMS" String
    ReceiveMode();    //Come back to Receving SMS mode and wait for other SMS
    
    DHT_OK=-1;        //If the DHT is found the variable should be reset to -1 otherwise it will be kept as !=-1 and will send SMS over and over                      
  }   
}

//This is used with ReceiveMode function, it's okay to use for tests with Serial monitor
void Serialcom() {
  delay(500);
  while(Serial.available())                                                                      
  {
    SIM800L.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(SIM800L.available())                                                                      
  {
    Serial.write(SIM800L.read());//Forward what Software Serial received to Serial Port
  }
}

//Set the SIM800L Receive mode
void ReceiveMode() {       
  
  SIM800L.println("AT\r"); //If everything is Okay it will show "OK" on the serial monitor
  Serialcom();
  SIM800L.println("AT+CMGF=1\r"); // Configuring TEXT mode
  Serialcom();
  SIM800L.println("AT+CNMI=2,2,0,0,0\r"); //Configure the SIM800L on how to manage the Received SMS... Check the SIM800L AT commands manual
  Serialcom();
}

void Send_Data() {
  Serial.println("Sending Data Readings...");    //Displays on the serial monitor...Optional
  SIM800L.println("AT+CMGF=1");                  //Set the module to SMS mode
  delay(1000);
  SIM800L.println("AT+CMGS=\"+254XXXXXXXX\"\r");  //Your phone number don't forget to include your country code example +254xxxxxxxxx"
  delay(1000);  
  SIM800L.println(Data_SMS);  //This string is sent as SMS
  delay(1000);
  SIM800L.println((char)26);  //Required to tell the module that it can send the SMS
  delay(5000);
  SIM800L.println();
  Serial.println("Data Sent.");
  delay(1000);

}
