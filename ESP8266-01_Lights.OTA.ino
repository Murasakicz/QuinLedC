#include "ESPHelper.h"
#include "logBuffer.h"


netInfo homeNet = {  .mqttHost = "192.168.100.20",     //can be blank if not using MQTT
                     .mqttUser = "",   //can be blank
                     .mqttPass = "",   //can be blank
                     .mqttPort = 1883,         //default port for MQTT is 1883 - only chance if needed.
                     .ssid = "",
                     .pass = ""
                  };

ESPHelper myESP(&homeNet);
ESP8266WebServer server(80);


#define MQTT_ID "/QuinLed_1"
#define PIN_LED1 0 // Pin with 0
#define PIN_LED2 2 // Pin with 1


int led1Value = 0, led2Value = 0;
int led1SetValue = 0, led2SetValue = 0;
uint32_t ledFadeTime = 0;


void ledFade() {
  if (millis() - ledFadeTime < 4ul) return;
  ledFadeTime = millis();

  if (led1Value < led1SetValue) {
    led1Value++;
  }
  if (led1Value > led1SetValue) {
    led1Value--;
  }

  if (led2Value < led2SetValue) {
    led2Value++;
  }
  if (led2Value > led2SetValue) {
    led2Value--;
  }

  analogWrite(PIN_LED1, led1Value);
  analogWrite(PIN_LED2, led2Value);
}

void setup() {
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  analogWrite(PIN_LED1, 0);
  analogWrite(PIN_LED2, 0);

  Serial.begin(115200); //start the serial line
  delay(500);

  logInfo("Starting Up, Please Wait...");

  myESP.OTA_enable();
  myESP.OTA_setPassword("Mura153624saki");
  myESP.OTA_setHostnameWithVersion("Lights.v1");

  myESP.addSubscription(MQTT_ID "/Led1/value");
  myESP.addSubscription(MQTT_ID "/Led2/value");
  myESP.addSubscription(MQTT_ID "/Led1/switch");
  myESP.addSubscription(MQTT_ID "/Led2/switch");

  myESP.setMQTTCallback(callback);
  myESP.begin();


  server.on("/log", HTTP_GET, []() {
    logBuffer.dumpTo(&server);
  });
  server.begin();


  logInfo("Initialization Finished.");
}

void loop() {
  myESP.loop();  //run the loop() method as often as possible - this keeps the network services running
  server.handleClient();
  ledFade();
}

void callback(char* topic, uint8_t* payload, unsigned int length) {
  logValue("Message arrived to topic: ", topic);

  if (length > 4) {
    logInfo("Message too long, ignored");
    return;
  }

  char valueRaw[5] = {0, 0, 0, 0, 0};
  strncpy(valueRaw, (char*)payload, length);
  int valueInt = String(valueRaw).toInt();

  bool topicLed1 = strstr(topic, "Led1") != NULL;
  bool topicLed2 = strstr(topic, "Led2") != NULL;
  bool topicSwitch = strstr(topic, "switch") != NULL;
  bool topicValue = strstr(topic, "value") != NULL;

  if (topicLed1) {
    if (topicValue) {
      logValue("Setting value to LED1: ", valueInt);
      led1SetValue = valueInt;
    }

    if (topicSwitch) {
      if (valueInt) {
        if (led1Value > 0){
          logInfo("Switch already on LED1");
        }else{
          led1SetValue = 1023;
          logInfo("Switch on LED1");
        }
      } else {
        led1SetValue = 0;
        logInfo("Switch off LED1");
      }
    }

    myESP.publish(MQTT_ID "/Led1/status", led1SetValue ? "1" : "0");
    myESP.publish(MQTT_ID "/Led1/rvalue", String(led1SetValue).c_str());
  }


  if (topicLed2) {
    if (topicValue) {
      logValue("Setting value to LED2: ", valueInt);
      led2SetValue = valueInt;
    }
    if (topicSwitch) {
      if (valueInt) {
        if (led2Value > 0){
          logInfo("Switch already on LED2");
        }else{
          led2SetValue = 1023;
          logInfo("Switch on LED2");
        }
      } else {
        led2SetValue = 0;
        logInfo("Switch off LED2");
      }
    }
    myESP.publish(MQTT_ID "/Led2/status", led2SetValue ? "1" : "0");
    myESP.publish(MQTT_ID "/Led2/rvalue", String(led2SetValue).c_str());
  }
  logInfo("Message processing finished");
}

