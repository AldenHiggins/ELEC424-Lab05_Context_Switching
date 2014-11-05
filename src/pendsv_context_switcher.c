#include "pendsv_context_switcher.h"
#include "sys_init.h"
#include "tasks.h"
// #include "stm32f10x_conf.h"
#include <stdlib.h>

#define MAIN_RETURN 0xFFFFFFF9
#define THREAD_RETURN 0xFFFFFFFD

int handlingLedOrMotor; // 1 for LED, 0 for motors
int firstInterruptFlag;
int firstTimeSetLR;

uint32_t *motorPSP;
uint32_t *ledPSP;

uint32_t motorStack[300];
uint32_t ledStack[300];

#define COUNT_INTERVAL 10000
uint32_t counter;

void main(void)
{
	init_system_clk();
	init_motors();
	init_blink();

	handlingLedOrMotor = 1;
	motorPSP = 0;
	ledPSP = 0;
	counter = 0;
	firstInterruptFlag = 1;
	firstTimeSetLR = 1;

  	// Initialize the two tasks
  	InitializeTaskStacks();

  	// Initialize pendsv
  	// *((uint32_t volatile *)0xE000ED04) = 0x10000000;
	// NVIC_InitTypeDef nvicStructure;
	// nvicStructure.NVIC_IRQChannel = PendSV_IRQn;
	// nvicStructure.NVIC_IRQChannelPreemptionPriority = 50;
	// nvicStructure.NVIC_IRQChannelSubPriority = 1;
	// nvicStructure.NVIC_IRQChannelCmd = ENABLE;
	// NVIC_Init(&nvicStructure);
  	// Set the two interrupts
  	*((uint32_t volatile *)0xE000ED20) = *((uint32_t volatile *)0xE000ED20) | 0x00FF0000;
  	// *priorityLocation = (*priorityLocation | 0x00F80000);

	if (SysTick_Config(72000000 / 5000))
	{ 
	  /* Capture error */ 
	  while (1);
	}

	// Loop to run before SysTick_Handler gets called
	while(1){}
}

// Initialize the two process stacks
void InitializeTaskStacks(void)
{
	// Initialize register values to 0
    int i;
    for (i = 0; i < 14; i++)
    {
    	motorStack[284 + i] = 0;
    	ledStack[284 + i] = 0;
    }
    // Initialize SPs
    motorStack[298] = (uint32_t)&task_spin_motors;
    ledStack[298] = (uint32_t)&task_blink_led;
    // Initialize PSRs
    motorStack[299] = 0x21000000;
    ledStack[299] = 0x21000000;
    // Initialize stack pointers
    ledPSP = &ledStack[284];
    motorPSP = &motorStack[284];
}

//Reads the program stack pointer for debugging purposes
static inline void * rd_program_stack_ptr(void)
{
  void * result = 0;
  asm volatile 
  (
  	"MRS %0, psp\n\t"
      : "=r" (result) : 
  );
  return result;
}


void SysTick_Handler(void)
{
	// Use counter to slow down the systick
	counter++;
	if (counter >= COUNT_INTERVAL)
	{
		counter = 0;
		// Trigger pendsv???
		*((uint32_t volatile *)0xE000ED04) = 0x10000000;
	}
}

void PendSV_Handler(void)
{
	// Handle the first case, load and run LED blink
	if (firstInterruptFlag == 1)
	{	
		firstInterruptFlag = 0;
		// Set your PSP 
		asm volatile
		(
			"MSR psp, %0"
			: : "r" (ledPSP)
		);

		// Load in software stack registers and increment PSP
		asm volatile 
		(
			"MRS r0, psp\n\t"
			"LDMIA r0!, {r4-r11}\n\t"
			"MSR psp, r0\n\t"
		);

		// Tell program to use PSP by setting LR
		asm volatile
		(
			"movw lr, #0xFFFD\n\t"
			"movt lr, #0xFFFF\n\t"
			"bx lr"
		);
	}
	else
	{
		// Save context
		asm volatile 
		(
			"MRS r0, psp\n\t"
  			"STMDB r0!, {r4-r11}\n\t"
  			"MSR psp, r0\n\t"
  		);

		// Handler is returning from LED and should call motor
		if (handlingLedOrMotor == 1)
		{
			handlingLedOrMotor = 0;
			// Save current PSP of LED
			asm volatile
			(
				"MRS %0, psp"
				: "=r" (ledPSP) : 
			);
			// Set your PSP to the motorPSP
			asm volatile
			(
				"MSR psp, %0"
				: : "r" (motorPSP)
			);
		}
		// Handler is returning from motor and should call LED
		else
		{
			handlingLedOrMotor = 1;
			// Save current PSP of LED
			asm volatile
			(
				"MRS %0, psp"
				: "=r" (motorPSP) : 
			);
			// Set your PSP 
			asm volatile
			(
				"MSR psp, %0"
				: : "r" (ledPSP)
			);
		}
		// Load in software stack registers and increment PSP
		asm volatile 
		(
			"MRS r0, psp\n\t"
			"LDMIA r0!, {r4-r11}\n\t"
			"MSR psp, r0\n\t"
		);

		// Set LR only for the first time you run the motor task
		// to signal the program to use the PSP
		if (firstTimeSetLR)
		{
			firstTimeSetLR = 0;
			asm volatile
			(
				"movw lr, #0xFFFD\n\t"
				"movt lr, #0xFFFF\n\t"
				"bx lr"
			);	
		}
	}
}