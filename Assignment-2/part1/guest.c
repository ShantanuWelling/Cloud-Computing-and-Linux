#include <stddef.h>
#include <stdint.h>

struct gva_to_hva { 
	// Structure to store the gva and hva for communicating with hypervisor
	uint32_t gva;
	uint32_t hva;
}; 

char str1[100][100]; // Store the strings for HC_numExitsByType
int str_ind=0;		// Index to store the strings in str1

static void outb(uint16_t port, uint8_t value) {
	asm("outb %0,%1" : /* empty */ : "a" (value), "Nd" (port) : "memory");
}

void HC_print8bit(uint8_t val)
{
	outb(0xE9, val);
}

void HC_print32bit(uint32_t val)
{	
	// Output the 32-bit value to port 0xE9 using outl instruction
	uint16_t port = 0xEA;
    asm("outl %0, %1" : : "a" (val), "Nd" (port) : "memory");
}

uint32_t HC_numExits()
{
	uint16_t port = 0xEB;
	uint32_t val; // Store the numExits value read from port 0xEB
	asm("inl %1, %0" : "=a" (val) : "Nd" (port) : "memory");
	return val;
}

void HC_printStr(char *str)
{
	uint16_t port = 0xEC;
	uint32_t val = (intptr_t)str; // Store the address of the string
	asm("outl %0, %1" : : "a" (val), "Nd" (port) : "memory");
}

char *HC_numExitsByType()
{
	uint16_t port = 0xED;
	// Store the string in str1 and send the address of the string to hypv
	asm("outl %0, %1" : : "a" ((uint32_t)(intptr_t)(str1[str_ind])), "Nd" (port) : "memory");
	return str1[str_ind++];	
}

uint32_t HC_gvaToHva(uint32_t gva)
{
	uint16_t port = 0xFF0;
	struct gva_to_hva arr; // Store the gva and hva
	arr.gva = gva;
	arr.hva = 0;
	// Send the gva to hypervisor and get the hva
	asm("outl %0, %1" : : "a" ((uint32_t)(intptr_t)&arr), "Nd" (port) : "memory");
	return arr.hva;	
}

void
__attribute__((noreturn))
__attribute__((section(".start")))
_start(void) {
	const char *p;

	for (p = "Hello 695!\n"; *p; ++p)
		HC_print8bit(*p);


	/*----------Don't modify this section. We will use grading script---------*/
	/*---Your submission will fail the testcases if you modify this section---*/
	HC_print32bit(2048);
	HC_print32bit(4294967295);

	uint32_t num_exits_a, num_exits_b;
	num_exits_a = HC_numExits();

	char *str = "CS695 Assignment 2\n";
	HC_printStr(str);

	num_exits_b = HC_numExits();

	HC_print32bit(num_exits_a);
	HC_print32bit(num_exits_b);

	char *firststr = HC_numExitsByType();
	uint32_t hva;
	hva = HC_gvaToHva(1024);
	HC_print32bit(hva);
	hva = HC_gvaToHva(4294967295);
	HC_print32bit(hva);
	char *secondstr = HC_numExitsByType();

	HC_printStr(firststr);
	HC_printStr(secondstr);
	/*------------------------------------------------------------------------*/

	*(long *) 0x400 = 42;

	for (;;)
		asm("hlt" : /* empty */ : "a" (42) : "memory");
}
