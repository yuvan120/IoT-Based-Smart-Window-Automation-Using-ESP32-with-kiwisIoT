#include <WiFi.h>

#include <DHT.h>

#include <ESP32Servo.h>

#include <KiwisIoT.h>

// WiFi credentials

const char* ssid = "YOUR_WIFI_NAME";

const char* password = "YOUR_WIFI_PASSWORD";

// Replace with your actual KiwisIoT panel topic

const char* topic = "dash_YOUR_PANEL_ID";

// GPIO connections

#define DHT_PIN 4

#define RAIN_PIN 27

#define LDR_PIN 34

#define SERVO_PIN 18

#define OPEN_LIMIT 32

#define CLOSE_LIMIT 33

#define DHTTYPE DHT22

DHT dht(DHT_PIN, DHTTYPE);

Servo windowServo;

KiwisIoT kiwisiot(ssid, password, topic);

// Window positions

const int CLOSED_ANGLE = 0;

const int OPEN_ANGLE = 90;

// Temperature thresholds

const float OPEN_TEMP = 30.0;

const float CLOSE_TEMP = 27.0;

// Set according to your rain sensor module

#define RAIN_ACTIVE LOW

bool autoMode = true;

bool windowOpen = false;

bool rainDetected = false;

unsigned long lastSend = 0;

// Open and close functions

void openWindow() {

if (digitalRead(OPEN_LIMIT) == LOW) {

windowOpen = true;

return;

}

windowServo.write(OPEN_ANGLE);

windowOpen = true;

Serial.println("Window Opening");

}

void closeWindow() {

if (digitalRead(CLOSE_LIMIT) == LOW) {

windowOpen = false;

return;

}

windowServo.write(CLOSED_ANGLE);

windowOpen = false;

Serial.println("Window Closing");

}

// Dashboard commands

void onControl(String ch, String val) {

// Channel 4: Window command

if (ch == "4") {

if (val == "1") {

openWindow();

}

if (val == "0") {

closeWindow();

}

}

// Channel 6: Operating mode

if (ch == "6") {

autoMode = (val == "1");

}

}

void setup() {

Serial.begin(115200);

pinMode(RAIN_PIN, INPUT);

pinMode(OPEN_LIMIT, INPUT_PULLUP);

pinMode(CLOSE_LIMIT, INPUT_PULLUP);

dht.begin();

windowServo.setPeriodHertz(50);

windowServo.attach(SERVO_PIN, 500, 2400);

// Start closed

windowServo.write(CLOSED_ANGLE);

kiwisiot.begin();

kiwisiot.onReceive(onControl);

Serial.println("Smart Window Automation Started");

}

void loop() {

// Read sensors

float temperature = dht.readTemperature();

float humidity = dht.readHumidity();

rainDetected =

(digitalRead(RAIN_PIN) == RAIN_ACTIVE);

int lightValue = analogRead(LDR_PIN);

// Rain has priority over automatic and manual opening

if (rainDetected) {

closeWindow();

}

// Automatic mode

else if (autoMode && !isnan(temperature)) {

if (temperature >= OPEN_TEMP) {

openWindow();

}

if (temperature <= CLOSE_TEMP) {

closeWindow();

}

}

// Send data to KiwisIoT every 2 seconds

if (millis() - lastSend >= 2000) {

lastSend = millis();

if (!isnan(temperature)) {

kiwisiot.send("0", temperature);

}

if (!isnan(humidity)) {

kiwisiot.send("1", humidity);

}

kiwisiot.send("2", rainDetected ? 1 : 0);

kiwisiot.send("3", lightValue);

kiwisiot.send("5", windowOpen ? 1 : 0);

}

// Cloud communication

kiwisiot.run();

delay(100);

};: