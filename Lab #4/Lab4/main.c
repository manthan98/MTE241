#include <stdio.h>
#include <stdlib.h>
#include <cmsis_os2.h>
#include <LPC17xx.h>
#include "random.h"

#define MSG_COUNT 10

// To send messages to queues
typedef struct {
	uint8_t msg;
} MQData_t;

// To monitor each queue
typedef struct {
	int sent;
	int received;
	int overflows;
} MQMonitor_t;

osMessageQueueId_t mq1;
osMessageQueueId_t mq2;

uint32_t serverOneMQ;
uint32_t serverTwoMQ;

MQMonitor_t mq1Monitor;
MQMonitor_t mq2Monitor;

int serverIndex;
uint32_t prevTime;

void client(void *arg)
{
	MQData_t data;
	osStatus_t status;
	
	while (1)
	{
		uint32_t randVal = next_event();
		uint32_t ticks = osKernelGetTickFreq() * randVal;
		uint32_t delayVal = (ticks / 9) >> 16;
		
		osDelay(delayVal);
		
		if (serverIndex == 0)
		{
			data.msg = 1U;
			status = osMessageQueuePut(mq1, &data, 0U, 0U);
			
			if (status == osOK)
			{
				mq1Monitor.sent++;
			}
			else if (status == osErrorResource)
			{
				mq1Monitor.overflows++;
			}
		}
		else
		{
			data.msg = 2U;
			status = osMessageQueuePut(mq2, &data, 0U, 0U);
			
			if (status == osOK)
			{
				mq2Monitor.sent++;
			}
			else if (status == osErrorResource)
			{
				mq2Monitor.overflows++;
			}
		}
		
		serverIndex = (serverIndex + 1) % 2;
		osThreadYield();
	}
}

void server(void *arg)
{
	MQData_t data;
	osStatus_t status;
	uint32_t mq = *(uint32_t *)arg;
	
	while (1)
	{
		uint32_t randVal = next_event();
		uint32_t ticks = osKernelGetTickFreq() * randVal;
		uint32_t seconds = ticks / 10;
		uint32_t delayVal = seconds >> 16;
		
		osDelay(delayVal);
		
		if (mq == 1)
		{
			status = osMessageQueueGet(mq1, &data, NULL, osWaitForever);
			
			if (status == osOK)
			{
				mq1Monitor.received++;
			}
		}
		else
		{
			status = osMessageQueueGet(mq2, &data, NULL, osWaitForever);
			
			if (status == osOK)
			{
				mq2Monitor.received++;
			}
		}
			
		osThreadYield();
	}
}

void systemMonitor(void *arg)
{
	while (1)
	{
		uint32_t time = osKernelGetTickCount() / osKernelGetTickFreq(); // Time in seconds
		
		if (time % 1 == 0 && time >= 1 && time != prevTime)
		{
			prevTime = time;
			printf("%d s: (%d, %d, %d), (%d, %d, %d)\n", time, mq1Monitor.sent, mq1Monitor.received, mq1Monitor.overflows, mq2Monitor.sent, mq2Monitor.received, mq2Monitor.overflows);
		}
		
		osThreadYield();
	}
}

void setup()
{
	mq1 = osMessageQueueNew(MSG_COUNT, sizeof(MQData_t), NULL);
	mq2 = osMessageQueueNew(MSG_COUNT, sizeof(MQData_t), NULL);
	
	serverIndex = 0;
	prevTime = 0;
	
	serverOneMQ = 1;
	serverTwoMQ = 2;
	
	mq1Monitor.sent = 0;
	mq1Monitor.received = 0;
	mq1Monitor.overflows = 0;
	
	mq2Monitor.sent = 0;
	mq2Monitor.received = 0;
	mq2Monitor.overflows = 0;
}

int main()
{
	osKernelInitialize();
	setup();
	osThreadNew(client, NULL, NULL);
	osThreadNew(server, &serverOneMQ, NULL);
	osThreadNew(server, &serverTwoMQ, NULL);
	osThreadNew(systemMonitor, NULL, NULL);
	osKernelStart();
}
