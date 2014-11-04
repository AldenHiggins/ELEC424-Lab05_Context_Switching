#include "systick_context_switcher.h"
#include "sys_init.h"
#include "tasks.h"
// #include "stm32f10x_conf.h"
#include <stdlib.h>

#define MAIN_RETURN 0xFFFFFFF9
#define THREAD_RETURN 0xFFFFFFFD

int handlingLedOrMotor; // 1 for LED, 0 for motors
int ledInitialized;
int motorInitialized;
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
	ledInitialized = 0;
	motorInitialized = 0;

	motorPSP = 0;
	ledPSP = 0;
	counter = 0;
	firstInterruptFlag = 1;
	firstTimeSetLR = 1;


	
  	// Initialize the two tasks
  	InitializeTaskStacks(&Dummy_Function, (void *)0, (void *)0x200000cc, 20);



	if (SysTick_Config(72000000 / 5000))
	{ 
	  /* Capture error */ 
	  while (1);
	}

	//Use PSP by modifying control register
	asm volatile(
		"movw r0, #0x0002\n\t"
		"movt r0, #0x0000\n\t"
		"MSR CONTROL, r0"
	);

	// task_blink_led();

	while(1){
	}
}

void InitializeTaskStacks(void (*p)(void), void * arg, void * stackaddr, int stack_size){
	// Spin motor initailization
    int i;
    for (i = 0; i < 14; i++){
    	motorStack[284 + i] = 0;
    }
    // Initialize SP
    motorStack[298] = (uint32_t)&Dummy_Function;
    // Initialize PSR
    motorStack[299] = 0x21000000;
    motorPSP = &motorStack[284];


    // Blink led initailization
    for (i = 0; i < 14; i++){
    	ledStack[284 + i] = 0;
    }
    // Initialize SP
    ledStack[298] = (uint32_t)&Other_Dummy_Function;
    // Initialize PSR
    ledStack[299] = 0x21000000;
    ledPSP = &ledStack[284];
}

// Motor associated dummy function
void Dummy_Function(void){
	// while(1){}
	task_spin_motors();
}

// LED associated dummy function
void Other_Dummy_Function(void){
	// while(1){}
	task_blink_led();
}



//Reads the main stack pointer
static inline void * rd_program_stack_ptr(void){
  void * result = 0;
  asm volatile ("MRS %0, psp\n\t"
      : "=r" (result) : );
  return result;
}

static inline void * rd_main_stack_ptr(void){
  void * result = 0;
  asm volatile ("MRS %0, msp\n\t"
      : "=r" (result) );
  return result;
}

static inline void write_program_stack_ptr(void * newPSP){
	asm volatile("MSR psp, %0"
		: "=r" (newPSP) );
}

static inline void save_context(void){
  uint32_t scratch;
  asm volatile ("MRS %0, psp\n\t"
      "STMDB %0!, {r4-r11}\n\t"
      "MSR psp, %0\n\t"  : "=r" (scratch) );
}

static inline void load_context(void){
  uint32_t volatile scratch; 
  	asm volatile (
		"MRS r0, psp\n\t"
	);
	asm volatile (
		"LDMIA r0!, {r4-r11}"
	);
	// asm volatile (
	// 	"LDMIA r0!, {r4-r11}" : "=r" (scratch)
	// );
	// asm volatile (
	// 	"MSR psp, %0"  : "=r" (scratch)
	// );
}

void SysTick_Handler(void)
{
	// Handle the first case
	if (firstInterruptFlag == 1)
	{	
		firstInterruptFlag = 0;
		void *randomPSP = rd_program_stack_ptr();
		// Load and run the LED blink
		// Set your PSP 
		asm volatile(
			"MSR psp, %0"
		: : "r" (ledPSP)
		);

		// Load in software stack registers and increment
		// PSP
		asm volatile (
			"MRS r0, psp\n\t"
			"LDMIA r0!, {r4-r11}\n\t"
			"MSR psp, r0\n\t"
		);

		/** Tell program to use PSP by setting LR **/
		asm volatile(
			"movw lr, #0xFFFD\n\t"
			"movt lr, #0xFFFF\n\t"
			"bx lr"
		);
	}
	else
	{
		// // Save context
		asm volatile ("MRS r0, psp\n\t"
  			"STMDB r0!, {r4-r11}\n\t"
  			"MSR psp, r0\n\t"
  		);



		// Use counter to slow down the systick
		counter++;
		if (counter >= COUNT_INTERVAL){
			counter = 0;
			// Handler is returning from LED and should call motor
			if (handlingLedOrMotor == 1)
			{
				handlingLedOrMotor = 0;
				// void *PSPval = rd_program_stack_ptr();
				// // Save context
				// asm volatile ("MRS r0, psp\n\t"
	   //    			"STMDB r0!, {r4-r11}\n\t"
	   //    			"MSR psp, r0\n\t");
				// Save current PSP of LED
				asm volatile(
					"MRS %0, psp"
				: "=r" (ledPSP) : 
				);


				// Set your PSP 
				asm volatile(
					"MSR psp, %0"
				: : "r" (motorPSP)
				);

				// Load in software stack registers and increment
				// PSP
				asm volatile (
					"MRS r0, psp\n\t"
					"LDMIA r0!, {r4-r11}\n\t"
					"MSR psp, r0\n\t"
				);

				// asm volatile(
				// 	"MSR psp, %0"
				// : : "r" (&motorStack[293])
				// );
				if (firstTimeSetLR)
				{
					firstTimeSetLR = 0;
					/** Tell program to use PSP by setting LR **/
					asm volatile(
						"movw lr, #0xFFFD\n\t"
						"movt lr, #0xFFFF\n\t"
						"bx lr"
					);	
				}
				
			}
			// Handler is returning from MOTOR and should call LED
			else
			{
				handlingLedOrMotor = 1;
				// // // Save current LED context
				// asm volatile ("MRS r0, psp\n\t"
	   //    			"STMDB r0!, {r4-r11}\n\t"
	   //    			"MSR psp, r0\n\t");
				// Save current PSP of LED
				asm volatile(
					"MRS %0, psp"
				: "=r" (motorPSP) : 
				);


				// Set your PSP 
				asm volatile(
					"MSR psp, %0"
				: : "r" (ledPSP)
				);

				// // Load in software stack registers and increment
				// // PSP
				asm volatile (
					"MRS r0, psp\n\t"
					"LDMIA r0!, {r4-r11}\n\t"
					"MSR psp, r0\n\t"
				);

				// asm volatile(
				// 	"MSR psp, %0"
				// : : "r" (&motorStack[293])
				// );

				// /** Tell program to use PSP by setting LR **/
				// asm volatile(
				// 	"movw lr, #0xFFFD\n\t"
				// 	"movt lr, #0xFFFF\n\t"
				// 	"bx lr"
				// );
			}
		}

		if (counter != 0)
		{
			// Load context
			asm volatile (
				"MRS r0, psp\n\t"
				"LDMIA r0!, {r4-r11}\n\t"
				"MSR psp, r0\n\t"
			);
		}
		

		// /** Tell program to use PSP by setting LR **/
		// asm volatile(
		// 	"movw lr, #0xFFFD\n\t"
		// 	"movt lr, #0xFFFF\n\t"
		// 	"bx lr"
		// );
	}
}