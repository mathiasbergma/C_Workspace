/*
 * read_conf.h
 *
 *  Created on: Mar 9, 2022
 *      Author: bergma
 */

#ifndef READ_CONF_H_
#define READ_CONF_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

using namespace std;

#define SERVER_H_LOOKUP		"server_hostname"
#define CA_LOOKUP			"server_ca_path"
#define CERT_LOOKUP			"client_cert_path"
#define KEY_LOOKUP			"client_key_path"
#define CLIENT_ID_LOOKUP	"friendly_name"

extern char host[100];
extern char ca_path[60];
extern char cert_path[60];
extern char key_path[60];
extern char client_id[60];

void read_configuration();
void remove_trailing(char * buffer);



#endif /* READ_CONF_H_ */
