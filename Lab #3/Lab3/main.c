#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <LPC17xx.h>
#include <cmsis_os2.h>

void turnOffLEDs()
{
	int pinMap[] = { 6, 5, 4, 3, 2, 31, 29, 28 };
	for (int i = 0; i < (sizeof(pinMap) / sizeof(int)); i++)
	{
		if (i < 5)
		{
			LPC_GPIO2->FIOCLR |= (1 << pinMap[i]);
		}
		else
		{
			LPC_GPIO1->FIOCLR |= (1 << pinMap[i]);
		}
	}
}

void t1(void *arg)
{
	while (true)
	{
		uint32_t A = (LPC_GPIO1->FIOPIN & (1 << 23)); // North
		uint32_t B = (LPC_GPIO1->FIOPIN & (1 << 24)); // East
		uint32_t C = (LPC_GPIO1->FIOPIN & (1 << 26)); // West
		uint32_t D = (LPC_GPIO1->FIOPIN & (1 << 25)); // South
		uint32_t ctr = (LPC_GPIO1->FIOPIN & (1 << 20)); // Press/release
		
		uint32_t activeLow = 0;
		
		if (A == activeLow) 
		{
			// 0001
			LPC_GPIO1->FIOSET |= (1 << 28);
		}
		else
		{
			LPC_GPIO1->FIOCLR |= (1 << 28);
		}
		
		if (B == activeLow)
		{
			// 0010
			LPC_GPIO1->FIOSET |= (1 << 29);
		}
		else
		{
			LPC_GPIO1->FIOCLR |= (1 << 29);
		}
		
		if (C == activeLow)
		{
			// 0100
			LPC_GPIO1->FIOSET |= (1ul << 31);
		}
		else
		{
			LPC_GPIO1->FIOCLR |= (1ul << 31);
		}
		
		if (D == activeLow)
		{
			// 0011
			LPC_GPIO1->FIOSET |= ((1 << 28) | (1 << 29));
		}
		else if (B != activeLow)
		{
			LPC_GPIO1->FIOCLR |= ((1 << 28) | (1 << 29));
		}
		
		if (ctr == activeLow)
		{
			LPC_GPIO2->FIOSET |= (1 << 3);
		}
		else
		{
			LPC_GPIO2->FIOCLR |= (1 << 3);
		}
		
		osThreadYield();
	}
}

void t2(void *arg)
{
	// Turn off the left-most LED
	LPC_GPIO2->FIODIR |= (1 << 6);
	LPC_GPIO2->FIOCLR |= (1 << 6);
	
	// Enable input at the pushbutton pin
	LPC_GPIO2->FIODIR |= (0 << 10);
	
	while (true)
	{
		uint32_t pushButtonInput = (LPC_GPIO2->FIOPIN & (1 << 10));
		
		// Test for an active low (pushbutton is pressed)
		if (pushButtonInput == 0)
		{
			LPC_GPIO2->FIOSET |= (1 << 6); // Turn on the LED
		}
		
		if (pushButtonInput == (1 << 10))
		{
			LPC_GPIO2->FIOCLR |= (1 << 6); // Turn off the LED
		}
		
		osThreadYield();
	}
}

bool prevLEDState;
bool prevButtonState;

void t2v2(void *arg)
{
	// Turn off the left-most LED
	LPC_GPIO2->FIODIR |= (1 << 6);
	LPC_GPIO2->FIOCLR |= (1 << 6);
	
	// Enable input at the pushbutton pin
	LPC_GPIO2->FIODIR |= (0 << 10);
	
	while (true)
	{
		uint32_t pushButtonInput = (LPC_GPIO2->FIOPIN & (1 << 10));
		
		if (prevButtonState && pushButtonInput != 0)
		{
			if (prevLEDState)
			{
				LPC_GPIO2->FIOCLR |= (1 << 6); // Turn off the LED
			}
			else
			{
				LPC_GPIO2->FIOSET |= (1 << 6); // Turn on the LED
			}
			
			prevLEDState = !prevLEDState;
			prevButtonState = false;
		}
		else if (!prevButtonState && pushButtonInput == 0)
		{
			prevButtonState = true;
		}
		
		osThreadYield();
	}
}

void t3(void *arg)
{
	// Turn on power for ADC
	LPC_SC->PCONP |= (1 << 12);
	
	// Setup AD function for pin 7
	// Default function is P0.25 which is controlled by pins 18, 19 of PINSEL1
	LPC_PINCON->PINSEL1 &= ~(3 << 18);
	LPC_PINCON->PINSEL1 |= (1 << 18);
	
	LPC_ADC->ADCR &= ~(0xFF); // Clear SEL bits
	
	// Set AD channel (AD0.2), clock divisor, and enable circuitry
	LPC_ADC->ADCR |= (1 << 2) | (4 << 8) | (1 << 21);
	
	while (true)
	{
		// Start conversion
		LPC_ADC->ADCR |= (1 << 24);
	
		while ((LPC_ADC->ADGDR & (1u << 31)) == 0);
		
		uint32_t result = (LPC_ADC->ADGDR >> 4) & 0xFFF;
		
		float voltageValue = result / (4095 / 3.3);
		
		printf("AD converter value: %f\n", voltageValue);
		
		osThreadYield();
	}
}

int main(void) {
	// Enable input-output at all LED pins
	int pinMap[] = { 6, 5, 4, 3, 2, 31, 29, 28 };
	for (int i = 0; i < (sizeof(pinMap) / sizeof(int)); i++)
	{
		if (i < 5)
		{
			LPC_GPIO2->FIODIR |= (1 << pinMap[i]);
		}
		else
		{
			LPC_GPIO1->FIODIR |= (1 << pinMap[i]);
		}
	}
	
	prevButtonState = false;
	prevLEDState = false;
	
	turnOffLEDs();
	
	osKernelInitialize();
	osThreadNew(t1, NULL, NULL);
	osThreadNew(t2v2, NULL, NULL);
	osThreadNew(t3, NULL, NULL);
	osKernelStart();
}
