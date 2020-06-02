#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <lpc17xx.h>

void partOne()
{
	LPC_GPIO1->FIODIR |= (1 << 28);
	LPC_GPIO1->FIOCLR |= (1 << 28);
	
	LPC_GPIO2->FIODIR |= (0 << 10);
	
	while (true)
	{
		uint32_t pushButtonInput = (LPC_GPIO2->FIOPIN & (1 << 10));
		printf("PB: %d\n", pushButtonInput);
		
		uint32_t activeLow = 0x00000000;
		if (pushButtonInput == activeLow)
		{
			LPC_GPIO1->FIOSET |= (1 << 28); // Turn on the LED
		}
		
		if (pushButtonInput == (1 << 10))
		{
			LPC_GPIO1->FIOCLR |= (1 << 28); // Turn off the LED
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

int main()
{
	printf("Hello, world!\n");
	
	partOne();
	//partTwo();
}
