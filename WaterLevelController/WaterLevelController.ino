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

#define SLEEP_S 60 // how many seconds to sleep between readings
#define DEVICE_ID "LevelCon"
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm_t1;
float distanceCm_t2;
float prev_distance;
float thresh = 0.2;
float distanceInch;
float totalHeight_t1=100;
float totalHeight_t2=75;
const int trigPin_t1 = 5;
const int echoPin_t1 = 4;
const int trigPin_t2 = 14;
const int echoPin_t2 = 12;
const int pumpPin = 15;
int pump_max_sec=300;//5 min
int pump_state=0;//0=OFF;1=ON

#define TZ_INFO "Asia/Kolkata"

// InfluxDB client for InfluxDB Cloud API
InfluxDBClient client_cloud(INFLUXDB_CLOUD_URL, INFLUXDB_CLOUD_ORG, INFLUXDB_CLOUD_BUCKET, INFLUXDB_CLOUD_TOKEN, InfluxDbCloud2CACert);

void readlevel(int tp,ep)
{
  Serial.println("Read Level");
  prev_distance = distanceCm;
  digitalWrite(tp, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(tp, HIGH);
  delayMicroseconds(10);
  digitalWrite(tp, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(ep, HIGH);
  Serial.println(duration);
  // Calculate the distance
  distanceCm = duration * SOUND_VELOCITY/2;
  Serial.println(distanceCm);
}
void sendInflux()
{
  float sensor_value1 = totalHeight_t1-distanceCm_t1;
  float sensor_value2 = totalHeight_t2-distanceCm_t2;
  // END: read sensor values
  Point pointDevice("Home_WaterTanks"); // create a new measurement point (the same point can be used for Cloud and v1 InfluxDB)
  // add tags to the datapoints so you can filter them
  //pointDevice.addTag("device", DEVICE_ID);
  //pointDevice.addTag("SSID", WiFi.SSID());
  // Add data fields (values)
  pointDevice.addField("Tank1", sensor_value1);
  pointDevice.addField("Tank2", sensor_value2);
  pointDevice.addField("uptime", millis()); // in addition send the uptime of the Arduino
  pointDevice.addField("Strength",WiFi.RSSI());
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
  pinMode(trigPin_t1, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin_t1, INPUT); // Sets the echoPin as an Input
  pinMode(trigPin_t2, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin_t2, INPUT); // Sets the echoPin as an Input
  Serial.begin(115200);
  Serial.println("Starting setup");
  delay(100);
  wifiConnect(WIFI_SSID, WIFI_KEY);
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
  readlevel(trigPin_t1,echoPin_t1);
  readlevel(trigPin_t2,echoPin_t2);
  controlPump();
  sendInflux();
  // this sends the microcontroller to deepsleep until the next reading needs to be taken
  //  due to WiFi connect and sensor reading your measurement interval will always be 5~10 seconds longer than the SLEEP_S duration
  ESP.deepSleep(SLEEP_S * 1000000); // offset by the duration the program run (converted from ms to µs)
}

void loop() {
}

// try to connect to given SSID and key, loop until successful
void wifiConnect(const char* ssid, const char* key) {
  WiFi.begin(ssid, key);
  Serial.print("Waiting for WiFi connection..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(".");
  Serial.print("Successfully connected to ");
  Serial.println(WiFi.SSID());
}
void controlPump()
{
  //check the system time. 2 modes - when solar power assumed to be available; aggressive mode. when solar power not available - emergency mode
  
}
