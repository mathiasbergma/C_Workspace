/*
 * main.cpp
 *
 *  Created on: Mar 4, 2022
 *      Author: bergma
 */

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#define SERVER_H_LOOKUP "server_hostname"
#define CA_LOOKUP		"server_ca_path"
#define CERT_LOOKUP		"client_cert_path"
#define KEY_LOOKUP		"client_key_path"
string host;
string ca_path;
string cert_path;
string key_path;

void splitString(string str, string delimiter = "	") {
	int start = 0;
	int end = str.find(delimiter);
	while (end != -1) {
		cout << str.substr(start, end - start) << endl;
		start = end + delimiter.size();
		end = str.find(delimiter, start);
	}
	cout << str.substr(start, end - start);
}

int main(void) {
	ifstream conf_file;

	string line, str;
	string delimiter = "	";

	conf_file.open("/home/bergma/Gokart_CAN_API/Power_off/server.conf");
	/*
	 while (std::getline(conf_file, str)) {
	 std::cout << str << "\n";
	 }
	 */

	while (std::getline(conf_file, line)) {
		int start = 0;
		int end = line.find(delimiter);
		// Split String by space in C++

		str = line.substr(start, end - start);

		start = end + delimiter.size();
		end = str.find(delimiter, start);

		if (str == SERVER_H_LOOKUP) {
			host = line.substr(start, end - start);
		}
		else if (str == CA_LOOKUP)
		{
			ca_path = line.substr(start, end - start);
		}
		else if (str == CERT_LOOKUP)
		{
			cert_path = line.substr(start, end - start);
		}
		else if (str == KEY_LOOKUP)
		{
			key_path = line.substr(start, end - start);
		}



	}
	cout << host << endl;
	cout << ca_path << endl;
	cout << cert_path << endl;
	cout << key_path << endl;

	return 0;
}
