#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define MQ7_PIN 32

LiquidCrystal_I2C lcd(0x27, 16, 2);

const char* ssid = "KAWAKI@007";
const char* password = "kawaki@007";
const char* botToken = "5757374159:AAGbm0ukUtP9abDb2Go-xVuZjs0mHUfMqCc";
const char* chatId = "965875133";
WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

void scrollText(int row, String message, int delayTime, int lcdColumns) {
  for (int i=0; i < 16; i++) {
    message = " " + message;  
  } 
  message = message + " "; 
  for (int pos = 0; pos < message.length(); pos++) {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
}


void setup() {

  Serial.begin(9600);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  
  Serial.println("Connected to WiFi network");

  Serial.println("Connected to WiFi");


  lcd.init();
  lcd.backlight();
  dht.begin();
  pinMode(MQ7_PIN, INPUT);
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  int mq7Value = analogRead(MQ7_PIN);
  float voltage = (mq7Value / 1023.0) * 5.0;
  float coConcentration = (voltage - 0.1) * 10000.0 / 0.8;

  lcd.clear();

  lcd.setCursor(0,1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print(" %   ");
  
  // Scroll the temperature and CO concentration values
  String tempStr = "Temp:" + String(temperature) + " C   ";
  String coStr = "CO:" + String(coConcentration) + " ppm   ";
  scrollText(0, tempStr + coStr, 500, 16);

  // Display the humidity value
  
  String message = "Humidity: " + String(humidity) + "%\n" + 
                   "Temperature: " + String(temperature) + "°C\n" +
                   "CO Concentration: " + String(coConcentration) + "ppm";

  if (bot.sendMessage(chatId, message, "Markdown")) {
    Serial.println("Message sent successfully");
  } else {
    Serial.println("Error sending message");
  }

  delay(5000);
}
