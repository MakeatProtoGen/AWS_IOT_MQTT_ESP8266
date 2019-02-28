#include "ProtoKit.h"

#include <ESP8266WiFi.h>
#include <ESP8266AWSIoTMQTTWS.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <string.h>

// Initialize ProtoKit IoT Node
ProtoKit P(IoT_Node);

// Set these to run example
#define thing_name  "thing_name"

const char *ssid = "ssid";
const char *password = "password";

char *region = (char *) "region";
char *endpoint = (char *) "endpoint_sequence";
char *mqtt_Host = (char *) "endpoint_sequence.iot.region.amazonaws.com";
int mqtt_Port = 443;
char *iam_Key_Id = (char *) "iam_user_Key_Id";
char *iam_Secret_Key = (char *) "iam_user_Secret_Key";

char publish_payload[MQTT_MAX_PACKET_SIZE];
int led_state = 1;
static int flag = 0;

ESP8266DateTimeProvider dtp;
AwsIotSigv4 sigv4(&dtp, region, endpoint, mqtt_Host, mqtt_Port, iam_Key_Id, iam_Secret_Key);
AWSConnectionParams cp(sigv4);
AWSWebSocketClientAdapter adapter(cp);
AWSMqttClient client(adapter, cp);

void setup() {

  Serial.begin(115200);
  while (!Serial) {
    yield();
  }

  // connect to wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("Connected to...... ");
  Serial.println(WiFi.localIP());

  // connect to MQTT host
  int result = client.connect();
  Serial.printf("mqtt connect = %d\n", result);

  pinMode(P.Led_2, OUTPUT); //To configure LED pin as OUTPUT pin
  digitalWrite(P.Led_2, led_state);

  if (result == 0)
  {
    sprintf(publish_payload, "{\"state\":{\"reported\":{\"led\":%d}}}", led_state);
    Shadow_Update(publish_payload);
    Shadow_Get_Accepted();
  }
}

//To update data in AWS shadow
void Shadow_Update(char* publish_payload)
{
  const char* shadow_update = "$aws/things/"thing_name"/shadow/update";
  client.publish(shadow_update, publish_payload, 0, false);
  flag = 1;
}

//To send request message to get data from AWS shadow
void Shadow_Get()
{
  const char* shadow_get = "$aws/things/"thing_name"/shadow/get";
  char *publish_payload = "{\"state\":{\"reported\":{}}}";
  client.publish(shadow_get, publish_payload, 0, false);
}

//Response message obtained if the get request is accepted
void Shadow_Get_Accepted()
{
  const char* shadow_get_accepted = "$aws/things/"thing_name"/shadow/get/accepted";
  client.subscribe(shadow_get_accepted, 0,
                   [](const char* topic, const char* msg)
  {
    int desired_led_state;
    char *pch = strstr(msg, "\"led");
    char jsonString[MQTT_MAX_PACKET_SIZE] = "{";
    for (int i = 0; i < strlen(pch) - 2; i++) {
      jsonString[i + 1] = (char)pch[i] ;
    }

    //To obtain LED state using json commands
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(jsonString);

    for (JsonObject::iterator it = root.begin(); it != root.end(); ++it) {
      desired_led_state = it->value.as<int>();
      if (desired_led_state != led_state) {
        update_led_state(desired_led_state);
      }
    }
  }
                  );
}

//To update LED state from AWS shadow
void update_led_state(int desired_led_state)
{
  led_state = desired_led_state;
  digitalWrite(P.Led_2, led_state);
  sprintf(publish_payload, "{\"state\":{\"reported\":{\"led\":%d}}}", led_state);
  Shadow_Update(publish_payload);
}

void loop() {

  if (client.isConnected()) {
    if (flag == 1) {
      Shadow_Get();
      Shadow_Get_Accepted();
    }
    client.yield();
  }
  else {
    Serial.println("Not connected to MQTT Host...");
    delay(2000);
  }
}
