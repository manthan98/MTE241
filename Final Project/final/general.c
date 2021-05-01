#include <cmsis_os2.h>
#include "general.h"

// add any #includes here
#include <stdlib.h>

// add any #defines here
#define MAX_CHARS 10

// add global variables here
uint8_t nGens;
bool *loyalGens;
uint8_t reporterGen;
uint8_t senderGen;

uint8_t omLevels;
uint8_t origLevels;
osSemaphoreId_t startSema;
osSemaphoreId_t endSema;

osMutexId_t printMutex;
osMutexId_t omMutex;
osMutexId_t generalsMutex;

bool *visited;
char *baseMsg;

/** Record parameters and set up any OS and other resources
  * needed by your general() and broadcast() functions.
  * nGeneral: number of generals
  * loyal: array representing loyalty of corresponding generals
  * reporter: general that will generate output
  * return true if setup successful and n > 3*m, false otherwise
  */
bool setup(uint8_t nGeneral, bool loyal[], uint8_t reporter) {
	nGens = nGeneral;
	loyalGens = loyal;
	reporterGen = reporter;
	
	omLevels = 0;
	for (int i = 0; i < nGens; i++) 
	{
		if (!loyalGens[i]) 
		{
			omLevels++;
		}
	}
	origLevels = omLevels;
	
	if (!c_assert(nGens > 3 *omLevels)) {
		return false;
	}
	
	startSema = osSemaphoreNew(nGens, 0, NULL);
	endSema = osSemaphoreNew(nGens, 0, NULL);
	
	printMutex = osMutexNew(NULL);
	if (printMutex == NULL) 
	{
		printf("Failed to create mutex.\n");
	}
	
	omMutex = osMutexNew(NULL);
	if (omMutex == NULL)
	{
		printf("Failed to create mutex.\n");
	}
	
	generalsMutex = osMutexNew(NULL);
	if (generalsMutex == NULL)
	{
		printf("Failed to create mutex.\n");
	}
	
	visited = malloc(nGens * sizeof(bool));
	for (int i = 0; i < nGens; i++) 
	{
		visited[i] = false;
	}
	
	return true;
}


/** Delete any OS resources created by setup() and free any memory
  * dynamically allocated by setup().
  */
void cleanup(void) {
	loyalGens = NULL;
	visited = NULL;
	baseMsg = NULL;
	
	free(loyalGens);
	free(visited);
	free(baseMsg);
	
	osStatus_t status = osSemaphoreDelete(startSema);
	if (status != osOK) {
		printf("Failed to delete semaphore: %d\n", status);
	}
	
	osSemaphoreDelete(endSema);
	
	osMutexDelete(printMutex);
	osMutexDelete(omMutex);
	osMutexDelete(generalsMutex);
}

char* replaceChar(char *msg, char newChar)
{
	char *newStr = malloc(MAX_CHARS * sizeof(char));
	for (int i = 0; i < MAX_CHARS; i++) 
	{
		if (msg[i] == 'A' || msg[i] == 'R') 
		{
			newStr[i] = newChar;
		} 
		else 
		{
			newStr[i] = msg[i];
		}
	}
	return newStr;
}

/** This function performs the initial broadcast to n-1 generals.
  * It should wait for the generals to finish before returning.
  * Note that the general sending the command does not participate
  * in the OM algorithm.
  * command: either 'A' or 'R'
  * sender: general sending the command to other n-1 generals
  */
void broadcast(char command, uint8_t sender) {
	visited[sender] = true;
	senderGen = sender;
	
	for (int i = 0; i < nGens; i++) 
	{
		if (!visited[i]) 
		{
			char *msg = malloc(3 * sizeof(char));
			sprintf(msg, "%d:%c", sender, command);
			baseMsg = msg;
			
			if (i == reporterGen && omLevels == 0) 
			{
				printf("%s\n", msg);
			}
			
			osSemaphoreRelease(startSema);
		}
	}
	
	while (osSemaphoreGetCount(endSema) != nGens);
	
	printf("Done test case!\n");
	return;
}

void OM(uint8_t omLevel, uint8_t gen, char *message) {
	if (omLevel == 0) 
	{
		return;
	}
	
	visited[gen] = true;
	
	for (int i = 0; i < nGens; i++) 
	{
		if (!visited[i]) 
		{
			char *msg = malloc(MAX_CHARS * sizeof(char));
			sprintf(msg, "%d:%s", gen, message);
			
			// If treacherous lieutenant, send 'R' to even # generals, and 'A' to odd.
			// If treacherous commander, send 'R' to even # generals, and 'A' to odd.
			if (!loyalGens[gen])
			{				
				char newChar = gen % 2 == 0 ? 'R' : 'A';
				msg = replaceChar(msg, newChar);
			} 
			else if (!loyalGens[senderGen] && omLevel == origLevels) 
			{
				char newChar = gen % 2 == 0 ? 'R' : 'A';
				msg = replaceChar(msg, newChar);
			}
			
			if (i == reporterGen && omLevel - 1 == 0) {
				osMutexAcquire(printMutex, osWaitForever);
				printf("%s\n", msg);
				osMutexRelease(printMutex);
			}
			
			OM(omLevel - 1, i, msg);
		}
	}
	
	visited[gen] = false;
}

/** Generals are created before each test and deleted after each
  * test.  The function should wait for a value from broadcast()
  * and then use the OM algorithm to solve the Byzantine General's
  * Problem.  The general designated as reporter in setup()
  * should output the messages received in OM(0).
  * idPtr: pointer to general's id number which is in [0,n-1]
  */
void general(void *idPtr) {
	uint8_t id = *(uint8_t *)idPtr;
	
	osMutexAcquire(generalsMutex, osWaitForever);
	
	if (!visited[id]) 
	{
		osSemaphoreAcquire(startSema, osWaitForever);
	} else 
	{
		osSemaphoreRelease(endSema);
		osMutexRelease(generalsMutex);
		return;
	}
	
	osMutexAcquire(omMutex, osWaitForever);
	OM(omLevels, id, baseMsg);
	osMutexRelease(omMutex);
	
	osSemaphoreRelease(endSema);
	osMutexRelease(generalsMutex);
}
