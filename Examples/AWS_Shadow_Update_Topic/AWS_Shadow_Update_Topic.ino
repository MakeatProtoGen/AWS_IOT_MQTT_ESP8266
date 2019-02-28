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
    Shadow_Update_Accepted();
    Shadow_Update_Rejected();
  }

}

//To update data in AWS shadow
void Shadow_Update(char* publish_payload)
{
  const char* shadow_update = "$aws/things/"thing_name"/shadow/update";
  client.publish(shadow_update, publish_payload, 0, false);
}

//Response message obtained if the update request is accepted
void Shadow_Update_Accepted()
{
  const char* shadow_update_accepted = "$aws/things/"thing_name"/shadow/update/accepted";
  client.subscribe(shadow_update_accepted, 0,
                   [](const char* topic, const char* msg)
  {
    Serial.printf("Message received: '%s' on shadow topic %s\n", msg, topic);
  }
                  );
}

//Response message obtained if the update request is rejected
void Shadow_Update_Rejected()
{
  const char* shadow_update_rejected = "$aws/things/"thing_name"/shadow/update/rejected";
  client.subscribe(shadow_update_rejected, 0,
                   [](const char* topic, const char* msg)
  {
    Serial.printf("Message received: '%s' on shadow topic %s\n", msg, topic);
  }
                  );
}

void loop() {

  char *payload = "{\"state\":{\"reported\":{\"Message\": \"Welcome to AWS IoT Core\"}}}" ;  //Specify any message to display in AWS shadow

  if (client.isConnected()) {
    Shadow_Update(payload);
    client.yield();
  }

  else {
    Serial.println("Not connected to MQTT Host...");
    delay(2000);
  }
  delay(1000);   // Update after every 1000 milliSeconds
}
