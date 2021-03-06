/*
 * subscriber.cpp
 *
 *  Created on: Mar 9, 2022
 *      Author: bergma
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "MQTTClient.h"
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include "json.hpp"

using json = nlohmann::json;
using namespace std;

#define ADDRESS    	"localhost:1883"
#define CLIENTID   	"Subscriber"
#define TOPIC      	"ISD"
#define QOS        	1
#define TIMEOUT    	10000L
#define SIZE 		10
MQTTClient client;
MQTTClient_deliveryToken token;
MQTTClient_message pubmsg = MQTTClient_message_initializer;

char   todayDateStr[100];
time_t rawtime;
struct tm *timeinfo;

int index;
FILE * fp;
json data = json::array({});

void write_buff(char * timestamp, char * topic, char * msg)
{
	if (index > SIZE-1)
	{
		index = 0;
	}
	data[index] = {topic,{timestamp,msg}};
	index++;
}

void handler(int sig)
{
	char str[] = "Disconnecting mqtt client....goodbye\n";
	write(STDERR_FILENO, str, strlen(str));
	/********** Disconnect & destroy *********/
	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);
	fclose(fp);

    exit(EXIT_SUCCESS);

}
int got_mail(void * context, char * topic, int topicLen, MQTTClient_message *msg)
{
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	strftime(todayDateStr, strlen("DD-MMM-YYYY HH:MM:SS")+1,"%d-%b-%Y %H:%M:%S",timeinfo);
	fprintf(fp, "%s\t Topic = %s\t\t %s \n", todayDateStr, topic, (char *)msg->payload);

	write_buff(todayDateStr, topic, (char*)msg->payload);
	cout << data << endl;
	//printf("Received on topic %s: %s\n",topic,(char*)msg->payload);


	MQTTClient_freeMessage(&msg);
	MQTTClient_free(topic);

	return 1;
}
void delivered(void *context, MQTTClient_deliveryToken tok)
{
	printf("Confirmed delivery of message with token value %d\n", tok);
	return;
}

int main(void)
{
	struct sigaction act;


	fp = fopen("mqtt_receive.log","a");


	// Trap SIGINT : delivered on user pressing ^C
	memset(&act, 0, sizeof(act));
	act.sa_handler = handler;

	if (sigaction(SIGINT, &act, NULL) == -1)
		printf("sigaction SIGINT failed\n");


	/*********** Create MQTT Client ************/
	printf("Attempting to create MQTT Client\n");

	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

	MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE,
	NULL);
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;

	MQTTClient_setCallbacks(client, NULL, NULL, got_mail, delivered);

	/*********** Connect MQTT Client ************/
	int rc;
	printf("Connecting MQTT client\n");
	while ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d, trying again in 2 sec\n",rc);
		sleep(2);
	}

	printf("Subscribing to topic: %s\n", TOPIC);
	rc = MQTTClient_subscribe(client, TOPIC, QOS);

	printf("Subscribed to topic: %s, with return value: %d\n",TOPIC, rc);



	while (1)
	{
		(void)pause();
	}

	return 0;
}

