/*
 * client.c
 *
 *  Created on: Mar 7, 2022
 *      Author: bergma
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "MQTTClient.h"
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define ADDRESS    	"localhost:1883"
#define CLIENTID   	"Publisher"
#define TOPIC      	"ISD"
#define QOS        	1
#define TIMEOUT    	10000L

MQTTClient client;
MQTTClient_deliveryToken token;
MQTTClient_message pubmsg = MQTTClient_message_initializer;

void handler(int sig)
{
	char str[] = "Disconnecting mqtt client....goodbye\n";
	write(STDERR_FILENO, str, strlen(str));
	/********** Disconnect & destroy *********/
	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);

    exit(EXIT_SUCCESS);

}

int main(void)
{
	struct sigaction act;

	// Trap SIGINT : delivered on user pressing ^C
	memset(&act, 0, sizeof(act));
	act.sa_handler = handler;

	if (sigaction(SIGINT, &act, NULL) == -1)
		printf("sigaction SIGINT failed\n");


	/*********** Create MQTT Client ************/
	printf("Attempting to create MQTT Client\n");

	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	//MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;

	MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE,
	NULL);
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	//opts.username = USER;
	//opts.password = PASSWD;
	//conn_opts.ssl = &ssl_opts;
	//conn_opts.ssl->enableServerCertAuth = 1;
	// conn_opts.ssl->struct_version = 1;
	//conn_opts.ssl->CApath = CA_PATH;
	//conn_opts.ssl->keyStore = CERT_PATH;
	//conn_opts.ssl->trustStore = CERT_PATH;
	//conn_opts.ssl->privateKey = KEY_PATH;
	//conn_opts.ssl->sslVersion = MQTT_SSL_VERSION_TLS_1_2;

	/*********** Necessary when using self-signed certificates *************/
	//conn_opts.ssl->enableServerCertAuth = 0;

	/*********** Connect MQTT Client ************/
	int rc;
	printf("Connecting MQTT client\n");
	while ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d, trying again in 2 sec\n",rc);
		sleep(2);
	}
	char msg[] = "Hi from publisher";
	/********** Load message to be sent *********/
	pubmsg.payload = msg;
	pubmsg.payloadlen = strlen(msg);
	pubmsg.qos = QOS;
	pubmsg.retained = 0;

	while (1)
	{
		sleep(5);
		/********** Publish message ***********/
		MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);

		/********** Waiting on acknowledge from broker *********/
		printf("Delivering ON msg to server\n");
		rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
		printf("Message with token %d delivered.\n", token);
	}



	return 0;
}
