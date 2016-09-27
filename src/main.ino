#include <MQTTSNClient.h>
#include <WSNetwork.h>
#include <VirtualTimer.h>

using namespace MQTTSN;

WSNetwork network;
Client client(network, 5000);

// Device fixed address
// For this example we are using a fixed address for simplicity
// This means that we don't need to pair the device with the gateway, it will just connect
#define ADDR 23
#define PAN 0x09

// Topic strings
const char lastWill[] = "lastwill";
const char publishTopic[] = "client/publish";
const char subscribeTopic[] = "client/subscribe";

// Timer for publishing example topic repeatedly
VirtualTimer timer;

// Function that handles the "client/subscribe" topic
void subscribeHandler(struct MQTTSN::MessageData &msg)
{
  Serial.print("Got message, Topic: ");
  Serial.println(subscribeTopic);
  Serial.print("Message: ");
  for(uint8_t i = 0; i < msg.message.payloadlen; i++)
  {
    Serial.print(((char*)msg.message.payload)[i]);
  }
  Serial.println();
}

// Function for connecting with gateway
bool connectMqtt()
{
  Serial.println("Connecting MQTT...");
  // Set MQTT last will topic and blank message
  client.setWill(lastWill, NULL, 0);
  // Setup MQTT connection
  MQTTSNPacket_connectData options = MQTTSNPacket_connectData_initializer;
  options.duration = 10; // Keep alive interval, Seconds
  options.cleansession = true;
  options.willFlag = true;
  int status = client.connect(options);

  if(status != SUCCESS) return false;
  // Clear any previous subscriptions (useful if we are reconnecting)
  client.clearSubscriptions();
  client.clearRegistrations();
  // We need to first register the topics that we may publish (a diference between this MQTT-SN implementation and MQTT)
  client.registerTopic(publishTopic, strlen(publishTopic));
  // Subscribe a function handler to a topic
  client.subscribe(subscribeTopic, QOS1, subscribeHandler);

  return true;
}

void setup()
{
  Serial.begin(9600);
  // Start with a fixed address
  network.begin(ADDR, PAN);
  // Start timer for publishing "client/publish" every 5 seconds
  timer.countdown_ms(5000);
}

void loop()
{
  // Attend network tasks
  client.loop();
  if(!client.isConnected())
  {
    Serial.println("MQTT disconnected, trying to reconnect...");
    if(!connectMqtt()) return;
  }
  // Attend timer
  if(timer.expired())
  {
    char payload[] = "Hello world";
    bool retained = false;
    // Publish "client/publish"
    client.publish(publishTopic, payload, strlen(payload), QOS1, retained);
    // Restart timer
    timer.countdown_ms(5000);
  }

}
