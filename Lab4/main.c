#include <stdio.h>
#include <stdlib.h>
#include <cmsis_os2.h>
#include <LPC17xx.h>
#include "random.h"

#define MSG_COUNT 10

typedef struct {
	uint8_t msg;
} MQData_t;

typedef struct {
	int sent;
	int received;
	int overflows;
} MQMonitor_t;

osMessageQueueId_t mq1;
osMessageQueueId_t mq2;

MQMonitor_t mq1Monitor;
MQMonitor_t mq2Monitor;

int serverIndex;
uint32_t prevTime;

void client(void *arg)
{
	while (1)
	{
		MQData_t data;
		osStatus_t status;
	
		uint32_t randVal = next_event();
		uint32_t ticks = osKernelGetTickFreq() * randVal;
		uint32_t delayVal = (ticks / 9) >> 16;
		
		osDelay(delayVal);
		
		data.msg = serverIndex == 0 ? 1U : 2U;
		status = osMessageQueuePut(serverIndex == 0 ? mq1 : mq2, &data, 0U, 0U);
		
		if (serverIndex == 0)
		{
			if (status == osOK)
			{
				mq1Monitor.sent++;
			}
			else
			{
				mq1Monitor.overflows++;
			}
		}
		else
		{
			if (status == osOK)
			{
				mq2Monitor.sent++;
			}
			else
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
	while (1)
	{
		MQData_t data;
		osStatus_t status;
		uint32_t mq = *(uint32_t *)arg;
		
		uint32_t randVal = next_event();
		uint32_t ticks = osKernelGetTickFreq() * randVal;
		uint32_t delayVal = (ticks / 10) >> 16;
			
		osDelay(delayVal);
		
		status = osMessageQueueGet(mq == 1 ? mq1 : mq2, &data, NULL, osWaitForever);
		
		if (mq == 1)
		{
			if (status == osOK)
			{
				mq1Monitor.received++;
			}
		}
		else
		{
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
		uint32_t time = osKernelGetTickCount() / osKernelGetTickFreq();
		
		if (time % 1 == 0 && time >= 1 && time != prevTime)
		{
			prevTime = time;
			printf("%d s: (%d, %d, %d), (%d, %d, %d)\n", time, mq1Monitor.sent, mq1Monitor.received, mq1Monitor.overflows, mq2Monitor.sent, mq2Monitor.received, mq2Monitor.overflows);
		}
		
		osThreadYield();
	}
}

int main()
{
	osKernelInitialize();
	
	mq1 = osMessageQueueNew(MSG_COUNT, sizeof(MQData_t), NULL);
	mq2 = osMessageQueueNew(MSG_COUNT, sizeof(MQData_t), NULL);
	
	serverIndex = 0;
	prevTime = 0;
	
	osThreadNew(client, NULL, NULL);
	
	uint32_t serverOneMQ = 1;
	uint32_t serverTwoMQ = 2;
	
	osThreadNew(server, &serverOneMQ, NULL);
	osThreadNew(server, &serverTwoMQ, NULL);
	
	mq1Monitor.sent = 0;
	mq1Monitor.received = 0;
	mq1Monitor.overflows = 0;
	
	mq2Monitor.sent = 0;
	mq2Monitor.received = 0;
	mq2Monitor.overflows = 0;
	
	osThreadNew(systemMonitor, NULL, NULL);
	
	osKernelStart();
}
