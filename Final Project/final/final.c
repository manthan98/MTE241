#include <cmsis_os2.h>
#include "general.h"

typedef struct {
	uint8_t n;
	bool *loyal;
	uint8_t reporter;
	char command;
	uint8_t sender;
} test_t;

bool loyal0[] = { true, true, false };
bool loyal1[] = { true, true, true };
bool loyal2[] = { true, true, true };
bool loyal3[] = { true, false, true, true };
bool loyal4[] = { true, false, true, true };
bool loyal5[] = { false, true, false, true, true, true, true };

#define N_TEST 6

test_t tests[N_TEST] = {
	{ sizeof(loyal0)/sizeof(bool), loyal0, 1, 'R', 0 },
	{ sizeof(loyal1)/sizeof(bool), loyal1, 2, 'R', 0 },
	{ sizeof(loyal2)/sizeof(bool), loyal2, 2, 'A', 1 },
	{ sizeof(loyal3)/sizeof(bool), loyal3, 2, 'R', 0 },
	{ sizeof(loyal4)/sizeof(bool), loyal4, 2, 'A', 1 },
	{ sizeof(loyal5)/sizeof(bool), loyal5, 3, 'R', 0 }
};

#define MAX_GENERALS 7
uint8_t ids[MAX_GENERALS] = { 0, 1, 2, 3, 4, 5, 6 };
osThreadId_t generals[MAX_GENERALS];
uint8_t nGeneral;

void startGenerals(uint8_t n) {
	nGeneral = n;
	for(uint8_t i=0; i<nGeneral; i++) {
		generals[i] = osThreadNew(general, ids + i, NULL);
		if(generals[i] == NULL) {
			printf("failed to create general[%d]\n", i);
		}
	}
}

void stopGenerals(void) {
	for(uint8_t i=0; i<nGeneral; i++) {
		osThreadTerminate(generals[i]);
	}
}

void testCases(void *arguments) {
	for(int i=0; i<6; i++) {
		printf("\ntest case %d\n", i);
		if(setup(tests[i].n, tests[i].loyal, tests[i].reporter)) {
			startGenerals(tests[i].n);
			broadcast(tests[i].command, tests[i].sender);
			cleanup();
			stopGenerals();
		} else {
			printf(" setup failed\n");
		}
	}
	printf("\ndone\n");
}

/* main */
int main(void) {
	osKernelInitialize();
  osThreadNew(testCases, NULL, NULL);
	osKernelStart();
	
	for( ; ; ) ;
}
