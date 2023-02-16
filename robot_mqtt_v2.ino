

/*
Source:
- https://randomnerdtutorials.com/esp8266-nodemcu-mqtt-publish-bme680-arduino/
*/

#include <ESP8266WiFi.h>
#include <AsyncMqttClient.h>
#include <Ticker.h>


#define WIFI_SSID "iSim2"
#define WIFI_PASSWORD "WIFI_PASSWORD"

#define MQTT_HOST "broker.emqx.io"
#define MQTT_PORT 1883
#define TOPIC_CMD "aasem/robotcar/cmd"
#define TOPIC_STATUS "aasem/robotcar/status"

AsyncMqttClient mqttClient;


WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;
unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 10000;        // Interval at which to publish sensor readings

int LED = LED_BUILTIN;//D3;
int right_forward = D1;//IN1
int right_reverse = D2;//IN2
int left_forward = D5;//IN3
int left_reverse = D6;//IN4
int delay_time_on = 5000; // how long should each wheel turn?
int delay_time_off = 5000; // delay between tests


void direction(int v_dir=0){
  String status;
  digitalWrite(LED, HIGH);
    if (v_dir==0){
      status="Stop"; Serial.println(status);
      digitalWrite(right_forward, LOW);
      digitalWrite(right_reverse, LOW);

      digitalWrite(left_forward, LOW);
      digitalWrite(left_reverse, LOW);
    }
    else if (v_dir==1){
      status="Move Forward"; Serial.println(status);
      digitalWrite(right_forward, HIGH);
      digitalWrite(right_reverse, LOW);

      digitalWrite(left_forward, HIGH);
      digitalWrite(left_reverse, LOW);
    }
      else if (v_dir==-1){
      status="Move backword"; Serial.println(status);
      digitalWrite(right_forward, LOW);
      digitalWrite(right_reverse, HIGH);
      digitalWrite(left_forward, LOW);
      digitalWrite(left_reverse, HIGH);
    }
      else if (v_dir==2){
      status="Turn Right"; Serial.println(status);
      digitalWrite(right_forward, HIGH);
      digitalWrite(right_reverse, LOW);
      digitalWrite(left_forward, LOW);
      digitalWrite(left_reverse, LOW);
    }
      else if (v_dir==-2){
      status="Turn Left"; Serial.println(status);
      digitalWrite(right_forward, LOW);
      digitalWrite(right_reverse, LOW);
      digitalWrite(left_forward, HIGH);
      digitalWrite(left_reverse, LOW);
    }

      else if (v_dir==3){
      status="Turn sharp left"; Serial.println(status);
      digitalWrite(right_forward, LOW);
      digitalWrite(right_reverse, HIGH);
      digitalWrite(left_forward, HIGH);
      digitalWrite(left_reverse, LOW);
    }
          else if (v_dir==3){
      status="Turn sharp right"; Serial.println(status);
      digitalWrite(right_forward, HIGH);
      digitalWrite(right_reverse, LOW);
      digitalWrite(left_forward, LOW);
      digitalWrite(left_reverse, HIGH);
    }
   
    uint16_t packetIdPub1 = mqttClient.publish(TOPIC_STATUS, 1, true, status.c_str());
    digitalWrite(LED, LOW);

  }
  
void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe(TOPIC_CMD, 1);
  Serial.print("\nSubscribed : ");
  Serial.println(TOPIC_CMD );
  direction(0);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttPublish(uint16_t packetId) {
  /*
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  */
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print(" packetId: ");
  Serial.println(packetId);
  Serial.print(" qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print(" packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.print("\n\t CMD:");
  char new_payload[len+1];
  new_payload[len] = '\0';
  strncpy(new_payload, payload, len);
  Serial.print("**new_payload: ");
  Serial.println(new_payload);
  if((String(topic) == TOPIC_CMD) ){
    Serial.println("...topic confirmed..");
    if (strcmp(new_payload,"1")==0) {Serial.println("one received\n");direction(1);}
    else if (strcmp(new_payload,"-1")==0) direction(-1);
    else if (strcmp(new_payload,"2")==0)direction(2);
    else if (strcmp(new_payload,"-2")==0) direction(-2);
    else if (strcmp(new_payload,"0")==0) direction(0);
    else if (strcmp(new_payload,"3")==0) direction(3);
    else if (strcmp(new_payload,"-3")==0) direction(-3);     
  }

}


void setup() {
  pinMode(right_forward, OUTPUT);
  pinMode(right_reverse, OUTPUT); 
  pinMode(left_forward, OUTPUT); 
  pinMode(left_reverse, OUTPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  direction(0);
  Serial.begin(9600);      
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  //mqttClient.setCredentials(mqtt_user, mqtt_pass);
  connectToWifi();
 
}


void loop() {
  unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;

    }
  }
