/*
 * main.cpp
 *
 *  Created on: Mar 4, 2022
 *      Author: bergma
 */

#include <iostream>
#include <fstream>
//#include <string>
#include "read_conf.h"

using namespace std;

/*
#define SERVER_H_LOOKUP "server_hostname"
#define CA_LOOKUP		"server_ca_path"
#define CERT_LOOKUP		"client_cert_path"
#define KEY_LOOKUP		"client_key_path"
*/

int main(void) {

	read_configuration();

	cout << host << endl;
	cout << ca_path << endl;
	cout << cert_path << endl;
	cout << key_path << endl;
	cout << client_id << endl;

	return 0;
}
