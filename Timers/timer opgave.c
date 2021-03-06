/*

*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <math.h>

/*---------------- Globals, Macros ----------------------------*/
static long freq_s = 0;
static long interval_display;
static struct itimerspec itv;
static timer_t timerid;
static struct timespec tm_start, tm_end;

#define MIN_START_SEC	1
#define MAX_START_SEC	5
#define CLOCKID CLOCK_REALTIME

/*---------------- Functions ----------------------------------*/


void show_tmvalue(struct timespec value) {

	printf("sec = %ld nsec = %ld\n", value.tv_sec, value.tv_nsec);

}


void timerdiff2(struct timespec *a, struct timespec *b, struct timespec *result) {

    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;                             
    (result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec;                          
    /*
    if ((result)->tv_nsec < 0) {                                              
      --(result)->tv_sec;                                                     
      (result)->tv_nsec += 1000000000;
    }
    */

}

/* 
 * The realtime signal (SIGRTMIN) - timer expiry - handler.
 * WARNING! Using the printf(3) in a signal handler is unsafe - but we do it anyways here
 * #FEATURE 2
 */
static void timer_handler(int sig, siginfo_t * si, void *uc)
{
    struct timespec tmp;    
    struct timespec res;
    
	//----------------------------
	//FEATURE #2
    //SLÅ OP HVAD KLOKKEN ER NU OG GEM I TMP

    if (clock_gettime(CLOCK_REALTIME, &tmp) < 0)
        printf("clock_gettime (tm_end) failed\n");


	//FEATURE #2
	//SKRIV UD LØBENDE HVAD SAMLEDE TID ER 
    res.tv_sec = tmp.tv_sec - tm_start.tv_sec;
    // Convert result to string
    char str[32];
    sprintf(str, "Elapsed time: %ld sec\r",res.tv_sec);
    write(STDERR_FILENO, str, strlen(str));
	//------------------------------
}

/*
 * The signal handler for SIGINT; arrival here implies the user has
 * "reacted" - has pressed ^C.
 *
 * WARNING! Using the [f]printf(3) in a signal handler is unsafe!
 * We do so for the purposes of this demo app only; do not
 * use in production.
 */
static void userpress(int sig)
{
	struct timespec res;
	long long diff_ns = 0;
	long diff_s = 0;

	//--------------------------

	//FEATURE #1 
	//GET THE TIMESTAMP OF THE SIGNAL

	if (clock_gettime(CLOCK_REALTIME, &tm_end) < 0)
		printf("clock_gettime (tm_end) failed\n");


	printf("\n*** PRESSED ***\n");

	//WRITE VALUE TO CONSOLE
    printf("Reacted @ ");
    show_tmvalue(tm_end);


	//------------------------------

	/* Calculate the delta; subtracting one struct timespec
		* from another takes a little work. Found something on Google which seems to work ;)
    */
	timerdiff2(&tm_end, &tm_start, &res);

    printf("Timespend: ");

    show_tmvalue(res);

	//--------------------------------------------
    //FEATURE #1
    //FUNKTION DER TRÆKKER DIT TARGET ARGUMENT (FRA FEATURE #0) FRA DEN BRUGTE TID 
    //AFSLUT MED SHOW_TMVALUE

	printf("You came this close, time: ");

	diff_s = res.tv_sec - freq_s;
	diff_ns = (res.tv_nsec + (res.tv_sec*1000000000)) - (freq_s*1000000000);

    printf("sec = %ld  nsec = %lld\n",diff_s,diff_ns);


	//--------------------------

    exit(EXIT_SUCCESS);

}

static void arm_timer(timer_t tmrid, struct itimerspec *itmspec)
{
	printf("Arming timer now\n");

	if (timer_settime(tmrid, 0, itmspec, NULL) == -1)
		printf("timer_settime failed\n");
	
}

/*
 * startoff
 * The signal handler for SIGALRM; arrival here implies the app has
 * "started" - we shall arm the interval timer here, it will start
 * running immediately. Take a timestamp now.
 */
static void startoff()
{
	char press_msg[] = "\n*** GET READY! !!! ***\n";

	//----------------------------
	//FEATURE #2
	if (interval_display > 0)
	{
		arm_timer(timerid, &itv);
	}

	//---------------------
	write(STDERR_FILENO, press_msg, strlen(press_msg));

	//--- timestamp it: start time
	if (clock_gettime(CLOCK_REALTIME, &tm_start) < 0)
		printf("clock_gettime (tm_start) failed\n");

    printf("Started @ time: ");
    show_tmvalue(tm_start);
    
}


static int init(void)
{
	struct sigevent sev;
	struct rlimit rlim;
	struct sigaction act;

	//-------------------------------------------------------------
    //KONFIGURATION AF INTERVAL TIMER SIGNAL HANDLER - FEATURE #2
	//OPGAVE: Trap SIGRTMIN : delivered on (interval) timer expiry

	memset(&act, 0, sizeof(act));
	
    act.sa_flags = SA_SIGINFO | SA_RESTART;
	act.sa_sigaction = timer_handler;
	
    if (sigaction(SIGRTMIN, &act, NULL) == -1) {
		printf("sigaction SIGRTMIN failed\n");
    }


	//KONFIGURATION AF CLK TIL INTERVAL TIMER DER SMIDER SIGRTMIN
	//FEATURE #2

	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGRTMIN; //make it send SIGRTMIN when done - keep this for now
	sev.sigev_value.sival_ptr = &timerid;
	
    if (timer_create(CLOCKID, &sev, &timerid) == -1)
		printf("timer_create failed\n");

	//START TIMER MED GIVNE INTERVAL
	//FEATURE #2

	memset(&itv, 0, sizeof(struct itimerspec));
    itv.it_interval.tv_sec = interval_display;
    itv.it_value.tv_sec = interval_display;
   	//-----------------------------------------

	//-------------------------------------------------------------
    //KONFIGURATION AF SIGINT - FEATURE #1
	// Trap SIGINT : delivered on user pressing ^C
	memset(&act, 0, sizeof(act));
	act.sa_handler = userpress;

	if (sigaction(SIGINT, &act, NULL) == -1)
		printf("sigaction SIGINT failed\n");



	//----------------------------------------------

	/* Turn off core dump via resource limit;
	 * else, pressing ^\ (SIGQUIT) to quit will result in an
	 * unecessary core dump image file being created.
	 */
	memset(&rlim, 0, sizeof(rlim));
	if (prlimit(0, RLIMIT_CORE, &rlim, NULL) == -1)
		printf("prlimit:core to zero failed\n");
	return 0;
}

static void help(char *name)
{
	fprintf(stderr,
		"Usage: <freq-in-sec> <time-display-in-sec> \n"
		"\nFeeling time!?\n"
		"Once you run this app with the freq-in-sec parameter,\n"
		"a timer is started\n"
		"If run with time-display-in-sec parameter is given, a "
		"help timer will be displayed\n"
		"You must estimate the timepassed and try to "
        "Press CTRL-C at the exact time you've specified\n"
        "Your reaction time is displayed... Have fun!\n");

}

int main(int argc, char **argv)
{
	if (argc < 2) {
		help(argv[0]);
		exit(EXIT_FAILURE);
	}

	//----------------------------------------
	//FEATURE #0
	//FRA CLI SKAL DU KUNNE AFLEVERE DEN VÆRDI DU SKAL FORSØGE AT RAMME
	//LIDT BOILERPLATE TIL AT HJÆLPE DIG PÅ VEJ
	//DET ER OK AT GEMME DIT TARGET I EN GLOBAL
	
	
	freq_s = atoll(argv[1]);

    if (freq_s < 1) {
		help(argv[0]);
		exit(EXIT_FAILURE);
	}
    if (argc > 2)
    {
        interval_display = atoll(argv[2]);

        if (interval_display < 1)
        {
    		help(argv[0]);
    		exit(EXIT_FAILURE);
        }
    }

	//-----------------------------------------

	//KONFIG AF.. ALT
	init();

	//LET THE GAMES BEGIN
	startoff();

	while (1) {
		(void)pause();
		// pause() unblocked by signal delivery!
	}
	exit(EXIT_SUCCESS);
}
