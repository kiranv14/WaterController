#if defined(ESP32)
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#elif defined(ESP8266)
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#endif
#include <InfluxDbClient.h> // load the client library
#include <InfluxDbCloud.h> // only for InfluxDB Cloud: load SSL certificate and additional method call
#include "secrets.h" // load connection credentials

#define SLEEP_S 30 // how many seconds to sleep between readings
#define DEVICE_ID "LevelMon"
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float prev_distance;
float thresh = 0.2;
float distanceInch;
const int trigPin = 5;
const int echoPin = 4;

#define TZ_INFO "Asia/Kolkata"

// InfluxDB client for InfluxDB Cloud API
InfluxDBClient client_cloud(INFLUXDB_CLOUD_URL, INFLUXDB_CLOUD_ORG, INFLUXDB_CLOUD_BUCKET, INFLUXDB_CLOUD_TOKEN, InfluxDbCloud2CACert);

void readlevel()
{
  Serial.println("Read Level");
  prev_distance = distanceCm;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  Serial.println(duration);
  // Calculate the distance
  distanceCm = duration * SOUND_VELOCITY/2;
  Serial.println(distanceCm);
}
void sendInflux()
{
  float sensor_value1 = distanceCm;
  // END: read sensor values
  Point pointDevice("irrigation_tanks"); // create a new measurement point (the same point can be used for Cloud and v1 InfluxDB)
  // add tags to the datapoints so you can filter them
  pointDevice.addTag("device", DEVICE_ID);
  pointDevice.addTag("SSID", WiFi.SSID());
  // Add data fields (values)
  pointDevice.addField("sensor3", sensor_value1);
  pointDevice.addField("uptime", millis()); // in addition send the uptime of the Arduino
  pointDevice.addField("Strength",WiFi.RSSI());
   timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client_cloud.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client_cloud.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client_cloud.getLastErrorMessage());
  }
  Serial.println("writing to InfluxDB Cloud... ");
  if (!client_cloud.writePoint(pointDevice)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client_cloud.getLastErrorMessage());
  }
  else
  {
    Serial.println("Success");
  }

}
void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  Serial.begin(115200);
  Serial.println("Starting setup");
  delay(100);
  wifiConnect();
}

void loop() {
  readlevel();
  sendInflux();
  delay(5000);
}

// try to connect to given SSID and key, loop until successful
void wifiConnect() {
   Serial.println("Scanning Wifi Networks...");
   int n = WiFi.scanNetworks();
   Serial.print(n);Serial.println("Wifi networks found. Trying to connect to 2 known networks");
   for (int i = 0; i < n; ++i) {
     Serial.println(WiFi.SSID(i));
     if (WiFi.SSID(i)== WIFI_SSID1) {
        WiFi.begin(WIFI_SSID1,WIFI_KEY1);
        break;
     }
     if (WiFi.SSID(i)== WIFI_SSID2) {
       WiFi.begin(WIFI_SSID2,WIFI_KEY2);
       break;
     }
    }
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Successfully connected to ");
  Serial.println(WiFi.SSID());
}
