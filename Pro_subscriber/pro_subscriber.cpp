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
#include <stdint.h>
#include <linux/can.h>
#include "read_conf.h"
#include <queue>


//Please replace the following address with the address of your server
#define TOPIC 	"#"
#define QOS		1
#define CHAR_PER_LINE	150

using namespace std;

queue<string> q;

MQTTClient client;
MQTTClient_deliveryToken token;
MQTTClient_message pubmsg = MQTTClient_message_initializer;

int id_count;
struct dbc_data * dbc_array;

struct dbc_data
{
	canid_t can_id;
	float offset;
	int scale;
	char unit[20];
	char signal[20];
	int start_bit;
	int stop_bit;
};

struct can_data
{
	unsigned long long timestamp;
	canid_t can_id;
	unsigned int data[8] __attribute__((aligned(8)));
};

void convert_can_data(struct can_data *data_frame, struct dbc_data*,
		int *count);
void load_dbc_data(struct dbc_data*);
int id_count_dbc_data();

void handler(int sig)
{
	char str[] = "Disconnecting mqtt client....goodbye\n";
	write(STDERR_FILENO, str, strlen(str));
	/********** Disconnect & destroy *********/
	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);

	exit(EXIT_SUCCESS);

}
void splitmsg(char *msg)
{
	/* struct to hold temporary can_data */
	struct can_data data_frame;

	sscanf(msg, "{\"timestamp\":%llu,\"canid\":%05X,\"dataframe\":[%03X,%03X,%03X,%03X,%03X,%03X,%03X,%03X]}",
			&data_frame.timestamp, &data_frame.can_id, &data_frame.data[0],
			&data_frame.data[1], &data_frame.data[2], &data_frame.data[3],
			&data_frame.data[4], &data_frame.data[5], &data_frame.data[6],
			&data_frame.data[7]);

	convert_can_data(&data_frame, dbc_array, &id_count);


}
int got_mail(void *context, char *topic, int topicLen, MQTTClient_message *msg)
{
	//time ( &rawtime );
	//timeinfo = localtime ( &rawtime );
	//strftime(todayDateStr, strlen("DD-MMM-YYYY HH:MM:SS")+1,"%d-%b-%Y %H:%M:%S",timeinfo);
	//fprintf(fp, "{\"timestamp\": \"%s\", \"topicid\": \"%s\", \"value\": \"%s\"} \n", todayDateStr, topic, (char *)msg->payload);

	//printf("Received on topic %s with len %d: %s\n", topic, msg->payloadlen,
			//(char*) msg->payload);

	if (strcmp(topic, "gokart/CAN") == 0)
	{
		q.push((char *) msg->payload);
	}

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
	// Read configuration file
	read_configuration();

	/* count canid in csv file */
	id_count = id_count_dbc_data();

	// Allocate memory for the dbc_array
	dbc_array = (struct dbc_data *)malloc(id_count * sizeof(struct dbc_data));

	/* read dbc-data from csv file. Create array of structs */
	load_dbc_data(dbc_array);

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
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	conn_opts.ssl = &ssl_opts;
	conn_opts.ssl->enableServerCertAuth = 1;
	conn_opts.ssl->CApath = ca_path;
	conn_opts.ssl->keyStore = cert_path;
	conn_opts.ssl->privateKey = key_path;
	conn_opts.ssl->sslVersion = MQTT_SSL_VERSION_TLS_1_2;

	/*********** Necessary when using self-signed certificates *************/
	conn_opts.ssl->enableServerCertAuth = 0;

	MQTTClient_setCallbacks(client, NULL, NULL, got_mail, delivered);

	/*********** Connect MQTT Client ************/
	int rc;
	cout << "Connecting MQTT client" << endl;
	while ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		cout << "Failed to connect, return code " << rc
				<< ", trying again in 2 sec" << endl;
		sleep(2);
	}

	printf("Subscribing to topic: %s\n", TOPIC);
	rc = MQTTClient_subscribe(client, TOPIC, QOS);

	printf("Subscribed to topic: %s, with return value: %d\n", TOPIC, rc);

	while (1)
	{
		while (!q.empty())
		{
			//cout << "Size of stack before pop: " << q.size() << endl;
			/***** Split one message and pop it from the queue afterwards *****/
			splitmsg(const_cast<char*>(q.front().c_str()));
			q.pop();
			//cout << "Size of stack after pop: " << q.size() << endl;
		}
		sleep(5);
		//(void) pause();
	}

	return 0;
}
int id_count_dbc_data()
{
	/*
	 * read dbc-data from text file. Create array of structs
	 */

	FILE *pfile;
	pfile = fopen("DBC.csv", "r");

	if (pfile == NULL)
	{
		perror("file_open_read");
	}

	int count = 0;

	/* antal karaktere der kan være på en linje */
	char line[CHAR_PER_LINE];

	fgets(line, CHAR_PER_LINE, pfile); // læs føsrte linje i fil, skal ikke bruges til noget

	while (!feof(pfile))
	{
		fgets(line, CHAR_PER_LINE, pfile); // læs en linje i fil
		count++;
	}

	fclose(pfile);
	return count;
}
void load_dbc_data(struct dbc_data *ptr)
{
	/*
	 * read dbc-data from csv/text file. Create array of structs
	 */

	FILE *pfile;
	pfile = fopen("DBC.csv", "r");

	if (pfile == NULL)
	{
		perror("file_open_read");
	}

	/* antal karaktere der kan være på en linje */
	char line[CHAR_PER_LINE];

	fgets(line, CHAR_PER_LINE, pfile); // læs føsrte linje i fil, skal ikke bruges til noget

	while (!feof(pfile))
	{
		fgets(line, CHAR_PER_LINE, pfile); // læs en linje i fil

		sscanf(line, "%03X, %f, %d, %[^,], %[^,], %d, %d", &ptr->can_id,
				&ptr->offset, &ptr->scale, ptr->unit, ptr->signal,
				&ptr->start_bit, &ptr->stop_bit);
		ptr++;
	}

	fclose(pfile);

}

void convert_can_data(struct can_data *data_frame, struct dbc_data *dbc_array,
		int *count)
{

	//int count1 = *count;

	int i;
	for (i = 0; i < *count; i++)
	{

		if (data_frame->can_id == dbc_array->can_id)
		{
			/* casting hex to decimal */
			uint64_t raw_value_decimal = 0;

			int num = 0;
			int ii;
			for (ii = dbc_array->start_bit; ii < dbc_array->stop_bit + 1; ii++)
			{
				raw_value_decimal |= data_frame->data[ii] << num * 8;
				num++;
			}

			/* timestamp to reable clock/time  */
			int ms = data_frame->timestamp % 1000;
			int sec = data_frame->timestamp / 1000;

			time_t time = sec;

			struct tm timeinfo;
			timeinfo = *localtime(&time);
			char buf[80];

			strftime(buf, sizeof(buf), "%A %d %B %Y %H:%M:%S", &timeinfo);

			float physical_value = dbc_array->offset
					+ dbc_array->scale * raw_value_decimal;

			/* print data */
			printf("%s.%d	%s: %.3f %s\n", buf, ms, dbc_array->signal,
					physical_value, dbc_array->unit);

			printf("\n");

		}

		/* increment pointer in array */
		dbc_array++;
	}
}
