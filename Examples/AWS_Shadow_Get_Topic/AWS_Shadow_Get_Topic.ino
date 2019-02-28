#include "ProtoKit.h"

#include <ESP8266WiFi.h>
#include <ESP8266AWSIoTMQTTWS.h>

// Initialize ProtoKit IoT Node
ProtoKit P(IoT_Node);

// Set the following to run example
#define thing_name  "thing_name"

const char *ssid = "ssid";
const char *password = "password";

char *region = (char *) "region";
char *endpoint = (char *) "endpoint_sequence";
char *mqtt_Host = (char *) "endpoint_sequence.iot.region.amazonaws.com";
int mqtt_Port = 443;
char *iam_Key_Id = (char *) "iam_user_Key_Id";
char *iam_Secret_Key = (char *) "iam_user_Secret_Key";

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

  if (result == 0)
  {
    Shadow_Get_Accepted();
    Shadow_Get_Rejected();
  }
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
    Serial.printf("Message received: '%s' on shadow topic %s\n", msg, topic);
  }
                  );
}

//Response message obtained if the get request is rejected
void Shadow_Get_Rejected()
{
  const char* shadow_get_rejected = "$aws/things/"thing_name"/shadow/get/rejected";
  client.subscribe(shadow_get_rejected, 0,
                   [](const char* topic, const char* msg)
  {
    Serial.printf("Message received: '%s' on shadow topic %s\n", msg, topic);
  }
                  );
}

void loop() {

  if (client.isConnected()) {
    Shadow_Get();
    client.yield();
  }

  else {
    Serial.println("Not connected to MQTT Host...");
    delay(2000);
  }
  delay(1000);   // Update after every 1000 milliSeconds
}
