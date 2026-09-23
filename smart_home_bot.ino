#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "config.h"

String buttons[] = {"Light"};
int pins[] = {2};

bool protectionEnabled = true;
int allowedChatIds[] = {};

const String onSymbol = "✅ ";
const String offSymbol = "☑ ";

const int ldrPin = A0;
const int pirPin = D5;
const int sosLedPin = D6;

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

int deviceCount = 0;
const unsigned long botPollIntervalMs = 3000;
unsigned long lastBotPoll = 0;
String keyboardJson;
String keyboardStart;

void sendMorseSOS() {
  const char* morse = "... --- ...";

  for (const char c : String(morse)) {
    if (c == '.') {
      digitalWrite(sosLedPin, HIGH);
      delay(200);
    } else if (c == '-') {
      digitalWrite(sosLedPin, HIGH);
      delay(600);
    } else {
      delay(800);
    }

    digitalWrite(sosLedPin, LOW);
    delay(200);
  }
}

bool isAuthorized(const String& chatId) {
  if (!protectionEnabled) {
    return true;
  }

  const size_t allowedCount = sizeof(allowedChatIds) / sizeof(allowedChatIds[0]);
  for (size_t i = 0; i < allowedCount; ++i) {
    if (String(allowedChatIds[i]) == chatId) {
      return true;
    }
  }

  return false;
}

String getSwitchStatus() {
  String status;

  for (int i = 0; i < deviceCount; ++i) {
    status += digitalRead(pins[i]) ? onSymbol : offSymbol;
    status += buttons[i];
    status += '\n';
  }

  return status;
}

void handleNewMessages(int messageCount) {
  for (int i = 0; i < messageCount; ++i) {
    const String chatId = bot.messages[i].chat_id;
    const String messageText = bot.messages[i].text;
    const String senderName = bot.messages[i].from_name.length()
        ? bot.messages[i].from_name
        : "Guest";

    if (bot.messages[i].type == "callback_query") {
      for (int deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex) {
        if (messageText == buttons[deviceIndex]) {
          digitalWrite(pins[deviceIndex], !digitalRead(pins[deviceIndex]));
        }
      }

      bot.sendMessageWithInlineKeyboard(chatId, getSwitchStatus(), "", keyboardJson);
      continue;
    }

    if (!isAuthorized(chatId)) {
      if (messageText == "/start") {
        bot.sendMessage(chatId, "Access denied.", "");
      }
      continue;
    }

    if (messageText == "/switch") {
      bot.sendMessageWithInlineKeyboard(chatId, getSwitchStatus(), "", keyboardJson);
    }

    if (messageText == "/status") {
      const int lightLevel = analogRead(ldrPin);
      const bool motionDetected = digitalRead(pirPin);

      String status = "🌞 Light level: " + String(lightLevel) + "\n";
      status += motionDetected ? "🚶 Motion detected\n" : "🛑 No motion\n";

      bot.sendMessage(chatId, status, "");
    }

    if (messageText == "/sos") {
      bot.sendMessage(chatId, "🚨 Sending SOS signal...", "");
      sendMorseSOS();
    }

    if (messageText == "/start") {
      String welcome = "Welcome " + senderName + ".\n";
      welcome += "ESP8266 smart-home controller.\n\n";
      welcome += "/switch - control connected devices.\n";
      welcome += "/status - view light and motion sensors.\n";
      welcome += "/sos - trigger the Morse SOS LED signal.\n";

      bot.sendMessageWithInlineKeyboard(chatId, welcome, "", keyboardStart);
    }
  }
}

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }

  Serial.println();
  Serial.println("Wi-Fi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);

  connectToWiFi();

  deviceCount = sizeof(pins) / sizeof(pins[0]);

  for (int i = 0; i < deviceCount; ++i) {
    pinMode(pins[i], OUTPUT);
  }

  pinMode(pirPin, INPUT);
  pinMode(sosLedPin, OUTPUT);

  keyboardJson = "[";
  for (int i = 0; i < deviceCount; ++i) {
    if (i > 0) {
      keyboardJson += ",";
    }

    keyboardJson += "[{ \"text\": \"" + buttons[i] +
                    "\", \"callback_data\": \"" + buttons[i] + "\" }]";
  }
  keyboardJson += "]";

  keyboardStart = "[[{ \"text\": \"Switch Board\", \"callback_data\": \"/switch\" }]]";

  // For a production deployment, replace this with certificate validation.
  client.setInsecure();
}

void loop() {
  if (millis() - lastBotPoll < botPollIntervalMs) {
    return;
  }

  int messageCount = bot.getUpdates(bot.last_message_received + 1);

  while (messageCount > 0) {
    handleNewMessages(messageCount);
    messageCount = bot.getUpdates(bot.last_message_received + 1);
  }

  lastBotPoll = millis();
}
