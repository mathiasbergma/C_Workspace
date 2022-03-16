/*
 * subscriber.c
 *
 *  Created on: Mar 8, 2022
 *      Author: bergma
 */

#include <stdio.h>
#include <stdlib.h>
#include "MQTTClient.h"
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include "queue.h"

#define PORT 8889
#define ADDRESS    	"localhost:1883"
#define CLIENTID   	"Subscriber"
#define TOPIC      	"ISD"
#define QOS        	1
#define TIMEOUT    	10000L

MQTTClient client;
MQTTClient_deliveryToken token;
MQTTClient_message pubmsg = MQTTClient_message_initializer;
struct MHD_Daemon *daemonizer;
char *log_text;

char todayDateStr[100];
time_t rawtime;
struct tm *timeinfo;

FILE *fp;

void handler(int sig);
int got_mail(void *context, char *topic, int topicLen, MQTTClient_message *msg);

void delivered(void *context, MQTTClient_deliveryToken tok);
int print_out_key(void *cls, enum MHD_ValueKind kind, const char *key,
		const char *value);
int answer_to_connection(void *cls, struct MHD_Connection *connection,
		const char *url, const char *method, const char *version,
		const char *upload_data, size_t *upload_data_size, void **con_cls);

int main(void)
{
	struct sigaction act;

	/*** Create queue ***/
	queue = createQueue(10);

	fp = fopen("mqtt_receive.log", "a");

	// Trap SIGINT : delivered on user pressing ^C
	memset(&act, 0, sizeof(act));
	act.sa_handler = handler;

	if (sigaction(SIGINT, &act, NULL) == -1)
		printf("sigaction SIGINT failed\n");

	/**** Create http server deamon ****/

	daemonizer = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL,
	NULL, &answer_to_connection, NULL, MHD_OPTION_END);
	if (NULL == daemonizer)
		return 1;

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
		printf("Failed to connect, return code %d, trying again in 2 sec\n",
				rc);
		sleep(2);
	}

	printf("Subscribing to topic: %s\n", TOPIC);
	rc = MQTTClient_subscribe(client, TOPIC, QOS);

	printf("Subscribed to topic: %s, with return value: %d\n", TOPIC, rc);

	while (1)
	{
		(void) pause();
	}

	return 0;
}
void handler(int sig)
{
	char str[] = "Disconnecting mqtt client....goodbye\n";
	write(STDERR_FILENO, str, strlen(str));
	/********** Disconnect & destroy *********/
	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);

	fclose(fp);
	free(queue);
	MHD_stop_daemon(daemonizer);

	exit(EXIT_SUCCESS);

}
int got_mail(void *context, char *topic, int topicLen, MQTTClient_message *msg)
{
	char combined_msg[200] = "";
	//memset(combined_msg,0,200);
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(todayDateStr, strlen("DD-MMM-YYYY HH:MM:SS") + 1,
			"%d-%b-%Y %H:%M:%S", timeinfo);
	fprintf(fp,
			"{\"timestamp\": \"%s\", \"topicid\": \"%s\", \"value\": \"%s\"} \n",
			todayDateStr, topic, (char*) msg->payload);
	strcat(combined_msg, "{\"timestamp\":\"");
	strcat(combined_msg, todayDateStr);
	strcat(combined_msg, "\",\"topic\":\"");
	strcat(combined_msg, topic);
	strcat(combined_msg, "\",\"message\":\"");
	strcat(combined_msg, (char*) msg->payload);
	strcat(combined_msg, "\"}");

	//printf("%s\n", combined_msg);
	// Put transmission into queue
	enqueue(queue, combined_msg);

	MQTTClient_freeMessage(&msg);
	MQTTClient_free(topic);

	return 1;
}
void delivered(void *context, MQTTClient_deliveryToken tok)
{
	printf("Confirmed delivery of message with token value %d\n", tok);
	return;
}

int print_out_key(void *cls, enum MHD_ValueKind kind, const char *key,
		const char *value)
{
	printf("%s: %s\n", key, value);
	return MHD_YES;
}

int answer_to_connection(void *cls, struct MHD_Connection *connection,
		const char *url, const char *method, const char *version,
		const char *upload_data, size_t *upload_data_size, void **con_cls)
{
	//const char *page = "<html><body>Hello, browser!</body></html>";
	static const char *nocando = "Log is empty\n";
	static const char *emptied = "Log is now empty\n";

	struct MHD_Response *response;
	int ret;

	MHD_get_connection_values(connection, MHD_HEADER_KIND, &print_out_key,
	NULL);

	printf("URL: %s\nMethod: %s\nVersion: %s\n\n", url, method, version);

	if (strcmp(method, "GET") == 0)
	{
		if (!isEmpty(queue))
		{
			// Get all logged elements
			log_text = traversequeue(queue, log_text, 0);
			response = MHD_create_response_from_buffer(strlen(log_text),
					(void*) log_text, MHD_RESPMEM_MUST_COPY);

			ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
			free(log_text);
		}
		else
		{
			response = MHD_create_response_from_buffer(strlen(nocando),
					(void*) nocando, MHD_RESPMEM_PERSISTENT);

			ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
		}

	}
	else if (strcmp(method, "POST") == 0)
	{
		// Clear queue
		emptyqueue(queue);
		response = MHD_create_response_from_buffer(strlen(emptied),
				(void*) emptied, MHD_RESPMEM_MUST_COPY);

		ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	}

	MHD_destroy_response(response);

	return ret;
}
