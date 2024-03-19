#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "MQ7.h"

#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
#define A_PIN 32
#define VOLTAGE 5
//#define RELAY_PIN 18

MQ7 mq7(A_PIN, VOLTAGE);


float temperatureC;
float temperatureF;
float humidity;
float coConcentration;
float voltage;
int mq7Value;


LiquidCrystal_I2C lcd(0x27, 16, 2);

const unsigned long BOT_MTBS = 1000; // mean time between scan messages
unsigned long bot_lasttime;

// Wifi network station credentials
#define ssid "KAWAKI@007"
#define password "kawaki@07"

// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "6278672734:AAH1GCnEchR7xxJoF687QiIyiKgF-6XIekA"
#define CHAT_ID "965875133"
#define GROUP_ID "-930638267"

//const char* ssid = "KAWAKI@007";
//const char* password = "kawaki@07";
//const char* botToken = "5898529093:AAEoOgL4Q1GDB-_uZTilSjP9riknCIamOCg";
//const long chatId = 965875133;

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

void scrollText(int row, String message, int delayTime, int lcdColumns) {
  for (int i = 0; i < 16; i++) {
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
  lcd.init();
  lcd.backlight();
  dht.begin();
  //  pinMode(MQ7_PIN, INPUT);
  mq7.calibrate();
  pinMode(18, OUTPUT);

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  Serial.println("\nWiFi IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  temperatureC = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  temperatureF = dht.readTemperature(true);

  mq7Value = mq7.readPpm();

  //  if (mq7Value > 8) {
  //    // Code to turn on the relay
  //    digitalWrite(RELAY_PIN, HIGH); // Assuming you have defined the relay pin
  //  } else {
  //    // Code to turn off the relay
  //    digitalWrite(RELAY_PIN, LOW); // Assuming you have defined the relay pin
  //  }



  lcd.clear();

  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print(" %   ");

  // Scroll the temperature and CO concentration values
  String tempStr = "Temp:" + String(temperatureC) + " C   ";
  String coStr = "CO:" + String(mq7Value) + " ppm   ";
  scrollText(0, tempStr + coStr, 500, 16);


  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }

  delay(1000);
}




void handleNewMessages(int numNewMessages)
{
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);
  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id == CHAT_ID || chat_id == GROUP_ID )
    {
      String text = bot.messages[i].text;
      String from_name = bot.messages[i].from_name;
      if (from_name == "")
        from_name = "Guest";
      if (text.equals("/tempc")) {
        String msg = "Temperature is ";
        msg += msg.concat(temperatureC);
        msg += "C";
        bot.sendMessage(chat_id, msg, "");
      } else if (text.equals("/tempf")) {
        String msg = "Temperature is ";
        msg += msg.concat(temperatureF);
        msg += "F";
        bot.sendMessage(chat_id, msg, "");
      } else if (text.equals("/humidity")) {
        String msg = "Humidity is ";
        msg += msg.concat(humidity);
        msg += "%";
        bot.sendMessage(chat_id, msg, "");
      } else if (text.equals("/co")) {
        String msg = "CO in ppm is ";
        msg += msg.concat(coConcentration);
        msg += "ppm";
        bot.sendMessage(chat_id, msg, "");
      } else if (text.equals("/tonionizer")) {
        digitalWrite(18, HIGH);
        lcd.setCursor(0, 1);
        lcd.print("Relay: ON");
        String msg = "Ionizer Turned on\n";
        bot.sendMessage(chat_id, msg);
      } else if (text.equals("/toffionizer")) {
        digitalWrite(18, LOW);
        lcd.setCursor(0, 1);
        lcd.print("Relay: OFF");
        String msg = "Ionizer turned off\n";
        bot.sendMessage(chat_id, msg);
      } else if (text.equals("/start")) {
        String welcome = "Sensor readings.\n";
        welcome += "/tempc : Temperature in celcius \n";
        welcome += "/tempf : Temperature in faranthit\n";
        welcome += "/humidity : Humidity\n";
        welcome += "/co\n";
        welcome += "/tonionizer : Turn On Ionizer\n";
        welcome += "/toffionizer : Turn Off Ionizer\n";
        bot.sendMessage(chat_id, welcome, "Markdown");
      } else {
        // Handle invalid input
        bot.sendMessage(chat_id, "Invalid Command", "");
      }
    }
    else
    {
      bot.sendMessage(chat_id, "Unauthorized user", "");
    }
  }
}
