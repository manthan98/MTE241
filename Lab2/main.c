#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <lpc17xx.h>

#define PART 4

void partOne()
{
	// Turn off the left-most LED
	LPC_GPIO2->FIODIR |= (1 << 6);
	LPC_GPIO2->FIOCLR |= (1 << 6);
	
	// Enable writes to the pushbutton pin
	LPC_GPIO2->FIODIR |= (0 << 10);
	
	while (true)
	{
		uint32_t pushButtonInput = (LPC_GPIO2->FIOPIN & (1 << 10));
		
		// For debugging
		printf("PB: %d\n", pushButtonInput);
		
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
	// Enable writes for at all LED pins
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
		
		// For debugging only
		printf("Your selection: %s\n", input);
		
		char *end;
		uint32_t num = (uint32_t)strtol(input, &end, 10);
		
		char bitStr[] = { '0', '0', '0', '0', '0', '0', '0', '0', '\0' }; 
		for (int i = 0; i < 8; i++)
		{
			// Determine if the bit is set
			int testBit = num & 0x01;
			if (testBit == 0x01)
			{
				bitStr[8 - i - 1] = '1';
			}
			num = num >> 1;
		}
		
		// For debugging only
		printf("Binary output: %s\n", bitStr);
		
		for (int i = 0; i < 8; i++)
		{
			// If the bit is not set, turn off the corresponding LED (vice versa)
			if (bitStr[i] == '0')
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
			else
			{
				if (i < 5)
				{
					LPC_GPIO2->FIOSET |= (1 << pinMap[i]);
				}
				else
				{
					LPC_GPIO1->FIOSET |= (1 << pinMap[i]);
				}
			}
		}
		
		free(input);
	}
}

void partFour()
{
	// Turn on power for ADC
	LPC_SC->PCONP |= (1 << 12);
	
	// Setup AD function for pin 7
	LPC_PINCON->PINSEL1 &= ~(3 << 18);
	LPC_PINCON->PINSEL1 |= (1 << 18);
	
	LPC_ADC->ADCR &= ~(0xFF); // Clear SEL bits
	
	// Set AD channel (AD0.2), sample rate, and enable circuitry
	LPC_ADC->ADCR |= (1 << 2) | (4 << 8) | (1 << 21);
	
	while (true)
	{
		LPC_ADC->ADCR |= (1 << 24);
	
		while ((LPC_ADC->ADGDR & (1u << 31)) == 0);
		
		uint32_t totalSum = 0;
		for (int i = 4; i <= 15; i++)
		{
			uint32_t val = LPC_ADC->ADGDR & (1 << i);
			if (val != 0)
			{
				totalSum += pow(2, i);
			}
		}
		
		printf("AD converter value: %d\n", totalSum);
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
