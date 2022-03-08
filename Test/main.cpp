/*
 * main.cpp
 *
 *  Created on: Dec 6, 2021
 *      Author: bergma
 */

#include <iostream>

using namespace std;

int main() {
	for (int i = 0; i < 1000; i++) {
		if (!(i%(333)))
		{
		cout << "Modulus " << i << "= " << i % (333) << endl;
		}
	}

	return 0;
}
