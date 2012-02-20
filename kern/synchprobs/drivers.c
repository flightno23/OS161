/*
 * 08 Feb 2012 : GWA : Please make any changes necessary to test your code to
 * the drivers in this file. However, the automated testing suite *will
 * replace this file in its entirety* with driver code intented to stress
 * test your synchronization problem solutions.
 */

#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
#include <current.h>
#include <synch.h>

/*
 * 08 Feb 2012 : GWA : Driver code for the whalemating problem.
 */

/*
 * 08 Feb 2012 : GWA : The following functions are for you to use when each
 * whale starts and completes either mating (if it is a male or female) or
 * matchmaking. We will use the output from these functions to verify the to
 * verify the correctness of your solution. These functions may spin for
 * arbitrary periods of time or yield.
 */

void
male_start(void)
{
	kprintf("male %s starting\n", curthread->t_name);
}
void
male_end(void)
{
	kprintf("male %s ending\n", curthread->t_name);
}
void
female_start(void)
{
	kprintf("female %s starting\n", curthread->t_name);
}
void
female_end(void)
{
	kprintf("female %s ending\n", curthread->t_name);
}
void
matchmaker_start(void)
{
	kprintf("matchmaker %s starting\n", curthread->t_name);
}
void
matchmaker_end(void)
{
	kprintf("matchmaker %s ending\n", curthread->t_name);
}

/*
 * 08 Feb 2012 : GWA : The following function drives the entire whalemating
 * process. Feel free to modify at will, but make no assumptions about the
 * order or timing of threads launched by our testing suite.
 */

#define NMATING 10

struct semaphore * whalematingMenuSemaphore;

int
whalemating(int nargs, char **args)
{

	int i, j, err=0;
	
	(void)nargs;
	(void)args;

  whalematingMenuSemaphore = sem_create("Whalemating Driver Semaphore", 0);
  if (whalematingMenuSemaphore == NULL) {
    
    // 08 Feb 2012 : GWA : Probably out of memory, or you broke our
    // semaphores! Panicing might be an overreaction, but why not?
    
    panic("whalemating: sem_create failed.\n");
  }
 
  // 13 Feb 2012 : GWA : Students are smarter than me.
  whalemating_init();

	for (i = 0; i < 3; i++) {
		for (j = 0; j < NMATING; j++) {
			switch(i) {
			    case 0:
				err = thread_fork("Male Whale Thread",
						  male, whalematingMenuSemaphore, j, NULL);
				break;
			    case 1:
				err = thread_fork("Female Whale Thread",
						  female, whalematingMenuSemaphore, j, NULL);
				break;
			    case 2:
				err = thread_fork("Matchmaker Whale Thread",
						  matchmaker, whalematingMenuSemaphore, j, NULL);
				break;
			}
			if (err) {
				panic("whalemating: thread_fork failed: (%s)\n",
				      strerror(err));
			}
		}
	}
	for (i = 0; i < 3; i++) {
		for (j = 0; j < NMATING; j++) {
      P(whalematingMenuSemaphore);
    }
  }
  sem_destroy(whalematingMenuSemaphore);
  
  // 13 Feb 2012 : GWA : Students are WAY smarter than me, including Nikhil
  // Londhe.
  whalemating_cleanup();

	return 0;
}

/*
 * 08 Feb 2012 : GWA : Driver code for the stoplight problem.
 */

/*
 * 08 Feb 2012 : GWA : The following functions should be called by your
 * stoplight solution when a car is in an intersection quadrant. The
 * semantics of the problem are that once a car enters any quadrant it has to
 * be somewhere in the intersection until it call leaveIntersection(), which
 * it should call while in the final quadrant.
 *
 * As an example, let's say a car approaches the intersection and needs to
 * pass through quadrants 0, 3 and 2. Once you call inQuadrant(0), the car is
 * considered in quadrant 0 until you call inQuadrant(3). After you call
 * inQuadrant(2), the car is considered in quadrant 2 until you call
 * leaveIntersection().
 * 
 * As in the whalemating example, we will use the output from these functions
 * to verify the correctness of your solution. These functions may spin for
 * arbitrary periods of time or yield.
 */

void
inQuadrant(int quadrant)
{
	kprintf("car %s in quadrant %d\n", curthread->t_name, quadrant);
  return;
}

void leaveIntersection()
{
	kprintf("car %s left the intersection\n", curthread->t_name);
  return;
}

#define NCARS 10

struct semaphore * stoplightMenuSemaphore;

int
stoplight(int nargs, char **args)
{
  (void)nargs;
  (void)args;
  int i, direction, turn, err=0;
  char name[32];
  
  stoplightMenuSemaphore = sem_create("Stoplight Driver Semaphore", 0);
  if (stoplightMenuSemaphore == NULL) {
    
    // 08 Feb 2012 : GWA : Probably out of memory, or you broke our
    // semaphores! Panicing might be an overreaction, but why not?
    
    panic("stoplight: sem_create failed.\n");
  }
  
  // 13 Feb 2012 : GWA : Students are smarter than me.
  stoplight_init();

  for (i = 0; i < NCARS; i++) {
    
    direction = random() % 4;
    turn = random() % 3;
      
    snprintf(name, sizeof(name), "Car Thread %d", i);
    
    switch(turn) {
      case 0:
        err = thread_fork(name, gostraight, stoplightMenuSemaphore,
            direction, NULL);
        break;
      case 1:
        err = thread_fork(name, turnleft, stoplightMenuSemaphore, direction,
            NULL);
        break;
      case 2:
        err = thread_fork(name, turnright, stoplightMenuSemaphore, direction,
            NULL);
        break;
    }
  }
  
  for (i = 0; i < NCARS; i++) {
    P(stoplightMenuSemaphore);
  }
  
  sem_destroy(stoplightMenuSemaphore);
  
  // 13 Feb 2012 : GWA : Students are WAY smarter than me, including Nikhil
  // Londhe.
  stoplight_cleanup();

  return 0;
}
