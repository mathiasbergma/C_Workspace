/*
 * read_conf.cpp
 *
 *  Created on: Mar 9, 2022
 *      Author: bergma
 */

#include "read_conf.h"

char host[100];
char ca_path[60];
char cert_path[60];
char key_path[60];
char client_id[60];

void read_configuration()
{
	FILE *conf_file = fopen("server.conf", "r");

	char * line = NULL;
	const char *delimiter = "	";
	size_t len = 0;
	int read;

	//conf_file = fopen("~/Gokart_CAN_API/Power_off/server.conf", "r");

	while ((read = getline(&line, &len, conf_file)) != -1)
	{

		char *token = strtok(line, delimiter);

		if (strcmp(token, SERVER_H_LOOKUP) == 0)
		{
			token = strtok(NULL, delimiter);
			strcpy(host, token);

		}
		else if (strcmp(token, CA_LOOKUP) == 0)
		{
			token = strtok(NULL, delimiter);
			strcpy(ca_path, token);

		}
		else if (strcmp(token, CERT_LOOKUP) == 0)
		{
			token = strtok(NULL, delimiter);
			strcpy(cert_path, token);
		}
		else if (strcmp(token, KEY_LOOKUP) == 0)
		{
			token = strtok(NULL, delimiter);
			strcpy(key_path, token);
		}
		else if (strcmp(token, CLIENT_ID_LOOKUP) == 0)
		{
			token = strtok(NULL, delimiter);
			strcpy(client_id, token);
		}

	}

	remove_trailing(host);
	remove_trailing(ca_path);
	remove_trailing(cert_path);
	remove_trailing(key_path);
	remove_trailing(client_id);

	free(line);
}
void remove_trailing(char * buffer)
{
	int len = strlen(buffer);
	if (len > 0 && buffer[len-1] == '\n')
	    buffer[len-1] = 0;
}

