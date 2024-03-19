#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "DHT.h"
//#include "wire.h"
#include <LiquidCrystal_I2C.h>

#define DHTPIN 4 

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Wifi network station credentials
#define WIFI_SSID "KAWAKI@007"
#define WIFI_PASSWORD "kawaki@07"

// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "5757374159:AAHN7ZNQnO3pjP7ALFYa9D8Tv94e1YwbNow"
#define CHAT_ID "965875133"

WiFiClientSecure secured_client;

UniversalTelegramBot bot(BOT_TOKEN, secured_client);

DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27, 16, 2); // Initialize the LCD with I2C address and size


const unsigned long BOT_MTBS = 1000; // mean time between scan messages
unsigned long bot_lasttime; // last time messages' scan has been done
float temperatureC;
float temperatureF;
float humidity;

void setup()
{
  Serial.begin(9600);
  Serial.println(F("DHTxx test!"));

  dht.begin();

  // attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
   temperatureC = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  temperatureF = dht.readTemperature(true);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.print(temperatureC);
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print("%");
  
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
}

void handleNewMessages(int numNewMessages)
{
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);
  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID )
    {
      bot.sendMessage(chat_id, "Unauthorized user", "");
    }
    else
    {
      String text = bot.messages[i].text;
      String from_name = bot.messages[i].from_name;
      if (from_name == "")
        from_name = "Guest";
      if (text == "/tempC")
      {   String msg = "Temperature is ";
          msg += msg.concat(temperatureC);
          msg += "C";
          bot.sendMessage(chat_id,msg, ""); 
      }
      if (text == "/tempF")
      {  
          String msg = "Temperature is ";
          msg += msg.concat(temperatureF);
          msg += "F";
          bot.sendMessage(chat_id,msg, ""); 
      }
      if (text == "/humidity")
      {  
          String msg = "Humidity is ";
          msg += msg.concat(humidity);
          msg += "%"; 
          bot.sendMessage(chat_id,msg, ""); 
      }
      if (text == "/start")
      {
        String welcome = "DHTxx sensor readings.\n";
        welcome += "/tempC : Temperature in celcius \n";
        welcome += "/tempF : Temperature in faranthit\n";
        welcome += "/humidity : Humidity\n";
        bot.sendMessage(chat_id, welcome, "Markdown");
      }
    }
  }
}
