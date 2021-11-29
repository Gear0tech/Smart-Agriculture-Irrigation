
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>

#include <DHT.h>  // Including library for dht

#include <ESP8266WiFi.h>


#define PUMP D5      // Pin No. for PUMP

#define ledPin D8    // Pin No. for LED 
#define ldrPin D7    // Pin No. for LDR

#define MOTOR D6      // Pin No. for MOTOR

#define rainDigital D1    // Pin No. for Rain Sensor

const int sensor_pin = A0;    // Pin No. for Soil Moisture Sensor

String apiKey = "ZGYAOIWYJJGP8GY7";     //  Enter your Write API key from ThingSpeak

const char *ssid =  "2k18/b8/1083";     // replace with your wifi ssid and wpa2 key
const char *pass =  "0779cace";
const char* server = "api.thingspeak.com";

char auth[] = "FLx_6ukc4Y5EuK1sh_s5SDe-tCgTSFdU";
char Ssid[] = "2k18/b8/1083";
char Pass[] = "0779cace";

#define DHTPIN 0          //pin no. D3 where the dht11 is connected

DHT dht(DHTPIN, DHT11);

WiFiClient client;

SimpleTimer timer;

WidgetLCD lcd(V2);

void setup()
{
  Serial.begin(9600);
  Serial.begin(115200);
  delay(10);
  dht.begin();

  Serial.println("Connecting to ");
  Serial.println(ssid);


  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  pinMode(rainDigital, INPUT);
  pinMode(PUMP, OUTPUT);

  pinMode(ledPin, OUTPUT);
  pinMode(ldrPin, INPUT);

  pinMode(MOTOR, OUTPUT);


  Blynk.begin(auth, Ssid, Pass);


}

void loop()
{

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float moisture_percentage;
  int rainDigitalVal = digitalRead(rainDigital);
  int ldrStatus = digitalRead(ldrPin);

  if (isnan(h) || isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  if (client.connect(server, 80))  //   "184.106.153.149" or api.thingspeak.com
  {

    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(t);
    postStr += "&field2=";
    postStr += String(h);
    postStr += "&field3=";
    postStr += String(moisture_percentage);
    postStr += "&field4=";
    postStr += String(rainDigitalVal);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" degrees Celcius, Humidity: ");
    Serial.print(h);
    Serial.println("%. Send to Thingspeak.");


    moisture_percentage = ( 100.00 - ( (analogRead(sensor_pin) / 1023.00) * 100.00 ) );

    Serial.print("Soil Moisture(in Percentage) = ");
    Serial.print(moisture_percentage);
    Serial.println("%");

    delay(1000);

    Serial.println(rainDigitalVal);

    if (rainDigitalVal) {
      lcd.print(0, 0, "Rain Status:");
      lcd.print(0, 1, "Clear");
    }
    else {
      lcd.print(0, 0, "Rain Status:");
      lcd.print(0, 1, "It's Raining");
    }


    if (ldrStatus <= 0) {

      digitalWrite(ledPin, HIGH);
      Serial.print(ldrStatus);
      Serial.println("LDR is DARK, LED is ON");

    }

    else {

      digitalWrite(ledPin, LOW);
      Serial.print(ldrStatus);
      Serial.println("LED is OFF");

    }

    if (t <= 30) {

      digitalWrite(MOTOR, LOW);

      Serial.println(" MOTOR is OFF");

    }

    else {

      digitalWrite(MOTOR, HIGH);

      Serial.println("MOTOR is ON");

    }

    if (moisture_percentage <= 40) {

      digitalWrite(PUMP, HIGH);

      Serial.println(" WATER PUMP is ON");

    }

    else {

      digitalWrite(PUMP, LOW);

      Serial.println("WATER PUMP is OFF");

    }

    Blynk.virtualWrite(V7, moisture_percentage); //V5 is for Soil Moisture
    Blynk.virtualWrite(V5, h); //V5 is for Humidity
    Blynk.virtualWrite(V6, t); //V6 is for Temperature
    Blynk.virtualWrite(V8, rainDigitalVal); //V8 is for Rainfall



    delay(200);
  }
  client.stop();

  Serial.println("Waiting...");

  // thingspeak needs minimum 15 sec delay between updates, i've set it to 30 seconds
  delay(1000);

  Blynk.run();
  timer.run();
}
