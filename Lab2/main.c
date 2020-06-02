#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
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

/**
 * Converts a hexademical digit to its integer value.
 * @param h ascii character representing a hexadecimal digit
 * @return value of hexadecimal digit or -1 on error
 */
int8_t hToI(char h) {
	//if(!c_assert((h>='0' && h<='9') || (h>='A' && h<='F'))) return -1;
	if(h>='0' && h<='9') return h - '0';
	if(h>='A' && h<='F') return h - 'A' + 10;
    return 0;
}

/**
 * Converts an integer to its hexademical character representation.
 * @param i integer in range [0,15]
 * @return ascii code of hexademical digit or 0 on error
 */
char iToH(uint8_t i) {
	//if(!c_assert(i>=0 && i<=15)) return 0;
	if(i < 10) return '0' + i;
	else return 'A' + i - 10;
}

/**
 * Computes the length of a C string.
 * @param arr C array
 * @return length of array, 0 if it cannot be computed
*/
int getSize(char arr[])
{
    int count = 0;
    
    int i = 0;
    while (*(arr + i) != '\0')
    {
        count++;
        i++;
    }
    
    return count;
}

void partThree()
{
	// TODO: taking user input
	
	char *end;
	char *input = "75";
	uint32_t num = (uint32_t)strtol(input, &end, 10);
	
	char bitStr[] = { '0', '0', '0', '0', '0', '0', '0', '0', '\0' }; 
	for (int i = 0; i < 8; i++)
	{
		int testBit = num & 0x01;
		if (testBit == 0x01)
		{
			bitStr[8 - i - 1] = '1';
		}
		num = num >> 1;
	}
	
	printf("OH NO: %s\n", bitStr);
	
	int pinMap[] = { 6, 5, 4, 3, 2, 31, 29, 28 };
	for (int i = 0; i < 8; i++)
	{
		if (bitStr[i] == '0')
		{
			if (i < 4)
			{
				LPC_GPIO2->FIODIR |= (1 << pinMap[i]);
				LPC_GPIO2->FIOCLR |= (1 << pinMap[i]);
			}
			else
			{
				LPC_GPIO1->FIODIR |= (1 << pinMap[i]);
				LPC_GPIO1->FIOCLR |= (1 << pinMap[i]);
			}
		}
	}
}

int main()
{
	printf("Hello, world!\n");
	
	//partOne();
	//partTwo();
	partThree();
}
