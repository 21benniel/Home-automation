
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"





/************************* Pin Definition *********************************/

//Relays for switching appliances
#define Relay1            D6
#define Relay2            D2
#define Relay3            D1
#define Relay4            D5

//DHT11 for reading temperature and humidity value
#define DHTPIN            D7

//buzzer to know the status of MQTT connections and can be used for any other purpose according to your project need.
#define buzzer            D0

//Selection pins for multiplexer module to switch between different sensors and give data on a single analog pin.
#define S0                D3
#define S1                D4

//Analog pin to read the incoming analog value from different sensors.
#define analogpin         A0

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SSID_NAME"
#define WLAN_PASS       "PASSWORD"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "AIO_USERNAME"//"techiesms"
#define AIO_KEY         "AIO_KEY"//"912b30c900574034a653f41e2b4df838"


#define DHTTYPE           DHT11     // DHT 11 




DHT dht(DHTPIN, DHTTYPE);
uint32_t delayMS;


/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();
Servo myservo;
void setup() {
  Serial.begin(115200);

  delay(10);

  pinMode(buzzer, OUTPUT);
  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  pinMode(Relay3, OUTPUT);
  pinMode(Relay4, OUTPUT);
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(A0, INPUT);

  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  
  //Setting up DHT sensor
  dht.begin();



uint32_t x = 0;

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(20000))) {
    if (subscription == &Light1) {
      Serial.print(F("Got: "));
      Serial.println((char *)Light1.lastread);
      int Light1_State = atoi((char *)Light1.lastread);
      digitalWrite(Relay1, Light1_State);
      
    }
    if (subscription == &Light2) {
      Serial.print(F("Got: "));
      Serial.println((char *)Light2.lastread);
      int Light2_State = atoi((char *)Light2.lastread);
      digitalWrite(Relay2, Light2_State);
    }
    if (subscription == &Fan1) {
      Serial.print(F("Got: "));
      Serial.println((char *)Fan1.lastread);
      int Fan1_State = atoi((char *)Fan1.lastread);
      digitalWrite(Relay3, Fan1_State);
    }
    if (subscription == &Fan2) {
      Serial.print(F("Got: "));
      Serial.println((char *)Fan2.lastread);
      int Fan2_State = atoi((char *)Fan2.lastread);
      digitalWrite(Relay4, Fan2_State);
      
    }
  }

  // Now we can publish stuff!
  digitalWrite(S0, LOW);
  digitalWrite(S1, LOW);
  Serial.print("Motion "); Serial.println(analogRead(analogpin));
  Serial.print("...");
  int Value = analogRead(analogpin);
  if(Value>400)
  Value=1;
  else
  Value=0;
  if (! Motion.publish(Value)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);
  Serial.print("C02 "); Serial.println(analogRead(analogpin));
  Serial.print("...");
  Value = analogRead(analogpin);
  if (! CO2.publish(Value)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  digitalWrite(S0, LOW);
  digitalWrite(S1, HIGH);
  Serial.print("Sound "); Serial.println(analogRead(analogpin));
  Serial.print("...");
  int raw_sound = analogRead(analogpin);
  Value = map(raw_sound,0,1024,0,100);
  if (! Sound.publish(Value)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  digitalWrite(S0, HIGH);
  digitalWrite(S1, HIGH);
  Serial.print("Light "); Serial.println(analogRead(analogpin));
  Serial.print("...");
  int raw_light = analogRead(analogpin);
  Value = map(raw_light,1024,0,0,100);
  if (! Light.publish(Value)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }


  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  if (! Humidity.publish(h)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  if (! Temperature.publish(t)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
    if(! mqtt.ping()) {
    mqtt.disconnect();
    }
  */
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);
  delay(200);
  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);
  delay(200);
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
  digitalWrite(buzzer, HIGH);
  delay(2000);
  digitalWrite(buzzer, LOW);
}
