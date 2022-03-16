/*
 * read_GPIO.cpp
 *
 *  Created on: Nov 25, 2021
 *      Author: bergma
 */

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <chrono>
#include <stdio.h>
#include <sstream>
#include <thread>
#include "MQTTClient.h"

using namespace std;

//Please replace the following address with the address of your server

#define ADDRESS    	"ssl://ec2-3-122-253-158.eu-central-1.compute.amazonaws.com:8883"
#define CLIENTID   	"Beagle1"

#define USER		"test"
#define PASSWD		"lgiekGLQ!drbn_lir439"
#define CA_PATH		"/home/bergma/Gokart_CAN_API/client_certs/ca.crt"
#define CERT_PATH	"/home/bergma/Gokart_CAN_API/client_certs/client.crt"
#define KEY_PATH	"/home/bergma/Gokart_CAN_API/client_certs/client.key"

#define TOPIC      	"gokart/power"
#define QOS        	1
#define TIMEOUT    	10000L

char ONmsg[] = "Power goes ON";
char LOSTmsg[] = "Power was lost. Waiting";
char OFFmsg[] = "Power still off, shutting down";
char REGAINEDmsg[] = "Power restored";

MQTTClient_deliveryToken token;
MQTTClient_message pubmsg = MQTTClient_message_initializer;

void cli_reconnect(MQTTClient * cli, MQTTClient_connectOptions * conn_opts);

int main(int argc, char **argv)
{
	sleep(5);
	/*********** Create MQTT Client ************/
	MQTTClient client;
	cout << "Attempting to create MQTT Client" << endl;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;

	MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE,
	NULL);
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	//opts.username = USER;
	//opts.password = PASSWD;
	conn_opts.ssl = &ssl_opts;
	conn_opts.ssl->enableServerCertAuth = 1;
	// conn_opts.ssl->struct_version = 1;
	conn_opts.ssl->CApath = CA_PATH;
	conn_opts.ssl->keyStore = CERT_PATH;
	//conn_opts.ssl->trustStore = CERT_PATH;
	conn_opts.ssl->privateKey = KEY_PATH;
	conn_opts.ssl->sslVersion = MQTT_SSL_VERSION_TLS_1_2;
	
	/*********** Necessary when using self-signed certificates *************/
	conn_opts.ssl->enableServerCertAuth = 0;

	/*********** Connect MQTT Client ************/
	int rc;
	cout << "Connecting MQTT client" << endl;
	while ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		cout << "Failed to connect, return code " << rc
				<< ", trying again in 2 sec" << endl;
		sleep(2);
	}
	/********** Load message to be sent *********/
	pubmsg.payload = ONmsg;
	pubmsg.payloadlen = strlen(ONmsg);
	pubmsg.qos = QOS;
	pubmsg.retained = 0;
	/********** Publish message ***********/
	MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);

	/********** Waiting on acknowledge from broker *********/
	cout << "Delivering ON msg to server" << endl;
	rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
	cout << "Message with token " << (int) token << " delivered." << endl;


	/********** Program should never reach this point *********/
	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);

	return 0;
}

void cli_reconnect(MQTTClient * cli, MQTTClient_connectOptions * conn_opts)
{
	int rc;
	MQTTClient_disconnect(cli, 200);
	rc = MQTTClient_connect(&cli, conn_opts);
	cout << "Reconnected to client with return value: " << rc << endl;
	return;
}
