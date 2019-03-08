#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "preempt.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

struct sigaction signal_action;

unsigned int alarm(void){
	struct itimerval old, new;
	new.it_interval.tv_usec = HZ * 100;
	new.it_interval.tv_sec = 0;
	// Specifies how long until next signal is sent (in microseconds -- every 10000 usec = every 0.01 sec)
	new.it_value.tv_usec = HZ * 100;
	new.it_value.tv_sec = 0;
	// setitimer returns -1 on failure
	if (setitimer(ITIMER_VIRTUAL, &new, &old) < 0)
		return 0;
	else
		return old.it_value.tv_sec;
}

void signal_handler(int signal) {
        // Need to reset the alarm
	uthread_yield();       
}


void preempt_disable(void)
{       
	sigprocmask(SIG_BLOCK, &signal_action.sa_mask, NULL);
}

void preempt_enable(void)
{
	// NULL because we don't care about previous value of signal mask
	sigprocmask(SIG_UNBLOCK, &signal_action.sa_mask, NULL);
}

void preempt_start(void){       
	sigemptyset (&signal_action.sa_mask); // not blocked anymore
	// Adding SIGVTALRM to our mask in order to block it
        sigaddset(&signal_action.sa_mask, SIGVTALRM);
	// Set our signal_action struct's sighandler to be our function, signal_handler
        signal_action.sa_handler = signal_handler;
	// On receipt of SIGVTALRM, we're handling that signal with our signal handler defined in signal_action
        sigaction (SIGVTALRM, &signal_action, NULL);
	alarm();
}
