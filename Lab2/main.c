#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <lpc17xx.h>

#define PART 1
#define DEBUG_MODE false

void partOne()
{
	// Turn off the left-most LED
	LPC_GPIO2->FIODIR |= (1 << 6);
	LPC_GPIO2->FIOCLR |= (1 << 6);
	
	// Enable input at the pushbutton pin
	LPC_GPIO2->FIODIR |= (0 << 10);
	
	while (true)
	{
		uint32_t pushButtonInput = (LPC_GPIO2->FIOPIN & (1 << 10));
		
		#if DEBUG_MODE
			printf("PB: %d\n", pushButtonInput);
		#endif
		
		// Test for an active low (pushbutton is pressed)
		if (pushButtonInput == 0)
		{
			LPC_GPIO2->FIOSET |= (1 << 6); // Turn on the LED
		}
		
		if (pushButtonInput == (1 << 10))
		{
			LPC_GPIO2->FIOCLR |= (1 << 6); // Turn off the LED
		}
	}
}

void partTwo()
{
	while (true)
	{
		uint32_t A = (LPC_GPIO1->FIOPIN & (1 << 23)); // North
		uint32_t B = (LPC_GPIO1->FIOPIN & (1 << 24)); // East
		uint32_t C = (LPC_GPIO1->FIOPIN & (1 << 26)); // West
		uint32_t D = (LPC_GPIO1->FIOPIN & (1 << 25)); // South
		uint32_t ctr = (LPC_GPIO1->FIOPIN & (1 << 20)); // Press/release
		
		uint32_t activeLow = 0x00000000;
		char *direction = malloc(6*sizeof(char));
		
		if (A == activeLow) 
		{
			direction = "North\0";
		} 
		else if (B == activeLow)
		{
			direction = "East\0";
		}
		else if (C == activeLow)
		{
			direction = "West\0";
		}
		else if (D == activeLow)
		{
			direction = "South\0";
		}
		else
		{
			direction = "Center\0";
		}
		
		bool pressed = ctr == activeLow;
		
		printf("Direction: %s, Pressed: %s\n", direction, pressed ? "Yes" : "No");
	}
}

void partThree()
{
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
	
	while (true)
	{
		char *input = malloc(3 * sizeof(char));
		printf("Enter a number:\n");
		scanf("%s", input);
		
		#if DEBUG_MODE
			printf("Your selection: %s\n", input);
		#endif
		
		char *end;
		uint32_t num = (uint32_t)strtol(input, &end, 10);
		
		for (int i = 0; i < 8; i++)
		{
			// Determine if the bit is set
			int testBit = num & 0x01;
			
			// LSB corresponds to left-most LED
			int ledIdx = 8 - i - 1;
			
			// Turn the LED ON if bit is set (vice-versa)
			if (testBit == 0x01)
			{
				if (ledIdx < 5)
				{
					LPC_GPIO2->FIOSET |= (1 << pinMap[ledIdx]);
				}
				else
				{
					LPC_GPIO1->FIOSET |= (1 << pinMap[ledIdx]);
				}
			}
			else
			{
				if (ledIdx < 5)
				{
					LPC_GPIO2->FIOCLR |= (1 << pinMap[ledIdx]);
				}
				else
				{
					LPC_GPIO1->FIOCLR |= (1 << pinMap[ledIdx]);
				}
			}
			
			num = num >> 1;
		}
		
		free(input);
		free(end);
	}
}

void partFour()
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
	}
}

#if PART == 1
	int main() 
	{
		partOne();
	}
#elif PART == 2
	int main() 
	{
		partTwo();
	}
#elif PART == 3
	int main() 
	{
		partThree();
	}
#else
	int main() 
	{
		partFour();
	}
#endif
