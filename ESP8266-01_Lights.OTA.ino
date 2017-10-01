#include "ESPHelper.h"

netInfo homeNet = {  .mqttHost = "192.168.100.20",     //can be blank if not using MQTT
          .mqttUser = "",   //can be blank
          .mqttPass = "",   //can be blank
          .mqttPort = 1883,         //default port for MQTT is 1883 - only chance if needed.
          .ssid = "", 
          .pass = ""};

ESPHelper myESP(&homeNet);

const char* mqtt_id = "/QuinLed_1";
const int pwm_startup = 5;
const int fadeout = 5;
int led1Value, led2Value;
int led1SetValue, led2SetValue;

const byte ledPin1 = 0; // Pin with 0
const byte ledPin2 = 2; // Pin with 1

char* join(const char* a, char* b)
{
  int lenA = strlen(a);
  int lenB = strlen(b);
  char* solution = (char*) malloc(lenA + lenB + 1);
  memcpy(solution, a, lenA);
  memcpy(solution + lenA, b, lenB);
  solution[lenA + lenB] = '\0';
 
  return solution;
}

char* inttochar(int i){
  int n = log10(i) + 1;
  char * str = (char*) malloc(n+1);
  itoa(i, str, 10);
  return str;
  }

void ledFade(){
  if (led1Value < led1SetValue){ led1Value++; }
  if (led1Value > led1SetValue){ led1Value--; }

  if (led2Value < led2SetValue){ led2Value++; }
  if (led2Value > led2SetValue){ led2Value--; }

  analogWrite(ledPin1, led1Value);
  analogWrite(ledPin2, led2Value);
  delay(fadeout);
  }

void setup() {
  
  Serial.begin(115200); //start the serial line
  delay(500);

  Serial.println("Starting Up, Please Wait...");

  myESP.OTA_enable();
  myESP.OTA_setPassword("Mura153624saki");
  myESP.OTA_setHostnameWithVersion("Lights.v1");

  myESP.addSubscription(join(mqtt_id,"/Led1/value"));
  myESP.addSubscription(join(mqtt_id,"/Led2/value"));
  myESP.addSubscription(join(mqtt_id,"/Led1/switch"));
  myESP.addSubscription(join(mqtt_id,"/Led2/switch"));

  myESP.setMQTTCallback(callback);
  myESP.begin();

  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  analogWrite(ledPin1, pwm_startup);
  analogWrite(ledPin2, pwm_startup);
  
  Serial.println("Initialization Finished.");
}

void loop(){
  myESP.loop();  //run the loop() method as often as possible - this keeps the network services running

  //Put application code here
  ledFade();
  yield();
}

void callback(char* topic, uint8_t* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  char value[5] = {""};
  for (int i = 0; i < length; i++) {
    value[i]=(char)payload[i];
  }

  char* _led1 = strstr(topic,"Led1");
  char* _led2 = strstr(topic,"Led2");
  char* _switch = strstr(topic,"switch");
  char* _value = strstr(topic,"value");

  if (_led1){
    if (_value){
      led1SetValue= atoi( value );
      Serial.print("setting value to:");
      Serial.print(atoi( value ));
      if (led1SetValue  == 0) { myESP.publish(join(mqtt_id,"/Led1/status"),"0"); }
      else {myESP.publish(join(mqtt_id,"/Led1/status"),"1");}
    }
    if (_switch){
      for (int i=0;i<length;i++) {
        char receivedChar = (char)payload[i];
        if (receivedChar == '1'){
          led1SetValue= 1023;
          strncpy(value, "1023",4);
          Serial.print("Switch on");
          myESP.publish(join(mqtt_id,"/Led1/status"),"1");
        }
        if (receivedChar == '0'){
          led1SetValue= 0;
          strncpy(value, "0",1);
          Serial.print("Switch off");
          myESP.publish(join(mqtt_id,"/Led1/status"),"0");
        }
      }
    }
    myESP.publish(join(mqtt_id,"/Led1/rvalue"),value);
    Serial.println();
    Serial.println("Returning status");
  }
  if(_led2){
    if (_value){
      led2SetValue = atoi( value );
      Serial.print("setting value to:");
      Serial.print(atoi( value ));
      if (led2SetValue  == 0) {myESP.publish(join(mqtt_id,"/Led2/status"),"0"); }
      else {myESP.publish(join(mqtt_id,"/Led2/status"),"1");}
    }
    if (_switch){
      for (int i=0;i<length;i++) {
        char receivedChar = (char)payload[i];
        if (receivedChar == '1'){
          led2SetValue= 1023;
          strncpy(value, "1023",4);
          Serial.print("Switch on");
          myESP.publish(join(mqtt_id,"/Led2/status"),"1");
        }
        if (receivedChar == '0'){
          led2SetValue = 0;
          strncpy(value, "0",1);
          Serial.print("Switch off");
          myESP.publish(join(mqtt_id,"/Led2/status"),"0");
        }
      }
    }
    myESP.publish(join(mqtt_id,"/Led2/rvalue"),value);
    Serial.println("Returning status");
  }
  Serial.println();
}
