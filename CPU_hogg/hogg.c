/*
 * hogg.c
 *
 *  Created on: Mar 16, 2022
 *      Author: bergma

	Application		A CPU hogger
	File			CPU_Hog.cpp
	Purpose			A CPU-intensive program which is used to demonstrate attributes of scheduler behaviour
	Author			Richard John Anthony
	Date			December 2013

	Function		Runs in a CPU-intense loop to use CPU cycles (performs no IO so never blocks, always uses its full time slice)
*/


#include "stdio.h"


int main(int argc, char* argv[])
{
	int i, j, k;
	i = 0;
	j = 1;
	k = 2;
	for(long iOuterLoop = 0; iOuterLoop < 500000; iOuterLoop++)
	{
		for(long iInnerLoop = 0; iInnerLoop < 100000; iInnerLoop++)
		{
			// Make sure that the compiler doesn't optimise the loop away
			// It is non-trivial to determine that the computation done here has
			// no overall effect because each variable is used on a LHS and a RHS
			i += j;
			j += k;
			k += i;
		}
	}
	return 0;
}



