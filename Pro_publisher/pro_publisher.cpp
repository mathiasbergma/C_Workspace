/*
 * pro_subscriber.cpp
 *
 *  Created on: Mar 10, 2022
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
#include "read_conf.h"

//Please replace the following address with the address of your server

//#define USER		"gokart"
//#define PASSWD		"lgiekGLQ!drbn_lir439"
/*
#define CA_PATH		"/home/bergma/Gokart_CAN_API/client_certs/ca.crt"
#define CERT_PATH	"/home/bergma/Gokart_CAN_API/client_certs/client.crt"
#define KEY_PATH	"/home/bergma/Gokart_CAN_API/client_certs/client.key"
*/
#define TOPIC 	"gokart/CAN"

#define QOS		1
#define TIMEOUT    	10000L

using namespace std;

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
int got_mail(void * context, char * topic, int topicLen, MQTTClient_message *msg)
{
	//time ( &rawtime );
	//timeinfo = localtime ( &rawtime );
	//strftime(todayDateStr, strlen("DD-MMM-YYYY HH:MM:SS")+1,"%d-%b-%Y %H:%M:%S",timeinfo);
	//fprintf(fp, "{\"timestamp\": \"%s\", \"topicid\": \"%s\", \"value\": \"%s\"} \n", todayDateStr, topic, (char *)msg->payload);

	printf("Received on topic %s: %s\n",topic,(char*)msg->payload);


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
	read_configuration();

	//fp = fopen("mqtt_receive.log","a");


	// Trap SIGINT : delivered on user pressing ^C
	memset(&act, 0, sizeof(act));
	act.sa_handler = handler;

	if (sigaction(SIGINT, &act, NULL) == -1)
		printf("sigaction SIGINT failed\n");


	/*********** Create MQTT Client ************/
	MQTTClient client;
	cout << "Attempting to create MQTT Client" << endl;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;

	MQTTClient_create(&client, host, client_id, MQTTCLIENT_PERSISTENCE_NONE,
	NULL);
	conn_opts.keepAliveInterval = 200;
	conn_opts.cleansession = 1;
	//opts.username = USER;
	//opts.password = PASSWD;
	conn_opts.ssl = &ssl_opts;
	conn_opts.ssl->enableServerCertAuth = 1;
	// conn_opts.ssl->struct_version = 1;
	conn_opts.ssl->CApath = ca_path;
	conn_opts.ssl->keyStore = cert_path;
	//conn_opts.ssl->trustStore = CERT_PATH;
	conn_opts.ssl->privateKey = key_path;
	conn_opts.ssl->sslVersion = MQTT_SSL_VERSION_TLS_1_2;


	/*********** Necessary when using self-signed certificates *************/
	conn_opts.ssl->enableServerCertAuth = 0;

	//MQTTClient_setCallbacks(client, NULL, NULL, got_mail, delivered);

	/*********** Connect MQTT Client ************/
	int rc;
	cout << "Connecting MQTT client" << endl;
	while ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		cout << "Failed to connect, return code " << rc
				<< ", trying again in 2 sec" << endl;
		sleep(2);
	}
	printf("Client connected\n");

	char msg[100];

	scanf("%s",msg);
	/********** Load message to be sent *********/
	pubmsg.payload = msg;
	pubmsg.payloadlen = strlen(msg);
	pubmsg.qos = QOS;
	pubmsg.retained = 0;

	while (1)
	{
		/********** Publish message ***********/
		MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);

		/********** Waiting on acknowledge from broker *********/
		printf("Delivering msg to server\n");
		rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
		printf("Message with token %d delivered.\n", token);

		char msg[300];

		scanf("%s",msg);
		/********** Load message to be sent *********/
		pubmsg.payload = msg;
		pubmsg.payloadlen = strlen(msg);
		pubmsg.qos = QOS;
		pubmsg.retained = 0;
	}

	return 0;
}


