/*
 * main.c
 *
 *  Created on: Feb 21, 2022
 *      Author: bergma
 */

#include<stdio.h>
#include<signal.h>
#include<unistd.h>

void sig_handler(int signo)
{
  if (signo == SIGHUP)
    printf("received SIGHUP\n");
}

int main(void)
{
  if (signal(SIGHUP, sig_handler) == SIG_ERR)
  printf("\ncan't catch SIGINT\n");
  // A long long wait so that we can easily issue a signal to this process
  printf("PID: %d\n",getpid());
  while(1)
    sleep(1);
  return 0;
}


