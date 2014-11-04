#include "systick_context_switcher.h"
#include "sys_init.h"
#include "tasks.h"
// #include "stm32f10x_conf.h"
#include <stdlib.h>

#define MAIN_RETURN 0xFFFFFFF9
#define THREAD_RETURN 0xFFFFFFFD

typedef struct programContext{
	int32_t r0; // arg1
	int32_t r1; // arg2
	int32_t r2; // arg3
	int32_t r3; // arg4
	int32_t r4;
	int32_t r5;
	int32_t r6;
	int32_t r7;
	int32_t r8;
	int32_t r9; // real frame pointer??
	int32_t r10; // stack limit
	int32_t r11; // argument pointer
	int32_t r12; // temp workspace
	int32_t r13; // stack pointer
	int32_t r14; // link workspace registry
	int32_t r15; // program counter
} programContext;

typedef struct {
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r12;
  uint32_t lr;
  uint32_t pc;
  uint32_t psr;
} hw_stack_frame_t;

typedef struct {
	void *task1SP;
	void *task2SP;
} TaskStackPointers;

programContext ledContext;
programContext motorContext;

int handlingLedOrMotor; // 1 for LED, 0 for motors
int ledInitialized;
int motorInitialized;

uint32_t *task1PSP;
uint32_t *otherTask1PSP;
uint32_t task2PSP;


uint32_t task1Stack[300];

TaskStackPointers stackPointers;

// MSP value
static uint32_t * stack;


void main(void)
{
	init_system_clk();
	init_motors();
	init_blink();

	// NVIC_DisableIRQ(SysTick_IRQn);

	// task_blink_led();
	handlingLedOrMotor = 1;
	ledInitialized = 0;
	motorInitialized = 0;
	// task1PSP = malloc(sizeof(uint32_t));
	// task2PSP = malloc(sizeof(uint32_t));
	task1PSP = 0;
	task2PSP = 0;
	// 72000000


  	// Initialize the two tasks
  	// stackPointers.task1SP = (void *)0x200ccccc;
  	newTask(&Dummy_Function, (void *)0, (void *)0x200000cc, 20);
  	// stackPointers.task2SP = (void *)0x200fffff;
  	// newTask(&Other_Dummy_Function, (void *)0, (void *)0x200fffff, 20);
  	// NVIC_EnableIRQ(SysTick_IRQn);

	// NVIC_InitTypeDef nvicStructure;
 // 	nvicStructure.NVIC_IRQChannel = SysTick_IRQn;
 // 	nvicStructure.NVIC_IRQChannelPreemptionPriority = 1;
 // 	nvicStructure.NVIC_IRQChannelSubPriority = 1;
 // 	nvicStructure.NVIC_IRQChannelCmd = ENABLE;
 // 	NVIC_Init(&nvicStructure);

	if (SysTick_Config(72000000 / 1000))
	{ 
	  /* Capture error */ 
	  while (1);
	}

	// task_spin_motors();

	while(1){
	}
	// Dummy_Function();
}

void newTask(void (*p)(void), void * arg, void * stackaddr, int stack_size){

	// int i;
 //    void * mem;
 //    void * pc;
 //    hw_stack_frame_t * process_frame;
 //    uint32_t *general_pointer;
 //    general_pointer = 0;

	// process_frame = (hw_stack_frame_t *)(stackaddr - sizeof(hw_stack_frame_t));
 //    process_frame->r0 = (uint32_t)arg;
 //    process_frame->r1 = (uint32_t)0;
 //    process_frame->r2 = (uint32_t)0;
 //    process_frame->r3 = 0;
 //    process_frame->r12 = 0;
 //    process_frame->pc = ((uint32_t)p);
 //    // address of return function
 //    // process_frame->lr = (uint32_t)del_process;
 //    process_frame->lr = 0;
 //    process_frame->psr = 0x21000000; //default PSR value
    // general_pointer = (uint32_t *)(stackaddr - sizeof(hw_stack_frame_t));
    // *general_pointer = (uint32_t)10;
    // uint32_t *new_pointer = (uint32_t *)(stackaddr - sizeof(hw_stack_frame_t) + sizeof(uint32_t));
    // *general_pointer = (uint32_t)10;

	// uint32_t *stackPointerTest = (uint32_t *)0x200000cc;
    int i;
    for (i = 0; i < 14; i++){
    	task1Stack[285 + i] = 0;
    }
    // Initialize SP
    task1Stack[299] = (uint32_t)p;
    // Initialize PSR
    task1Stack[300] = 0x21000000;

    task1PSP = &task1Stack[285];


    /******* Old hardcoded stackPointer stuff ********************/
    // uint32_t *stackPointerTest = (uint32_t *)0x200000cc;
    // int i;
    // for (i = 0; i < 14; i++){
    // 	*stackPointerTest = 0;
    // 	stackPointerTest += 1;
    // }
    // // Initialize LR
    // *stackPointerTest = 0;
    // stackPointerTest += 1;
    // // Initialize SP
    // *stackPointerTest = (uint32_t)p;
    // stackPointerTest += 1;
    // // Initialize PSR
    // *stackPointerTest = 0x21000000;
    // stackPointerTest += 1;
    // // task1PSP = (void *)(((uint32_t *)0x200000cc) + 8);
    // task1PSP = (void *)0x200000cc;
    // otherTask1PSP = (uint32_t *)0x2000000ec;

    // *stackPointerTest
    // uint32_t testValue = 20;
    // uint32_t *testPointer = &testValue;
	// asm volatile (
	// 	"ldr r0, %0\n\t"
	// 	"str r0, %1" : : "m" (testPointer) , "m" (*stackPointerTest)
	// );

    // asm volatile (
    // 	"mov r8, #20"
    // );

    // asm volatile (
    // 	"movw r9, #0x00cc"
    // );

    // asm volatile (
    // 	"movt r9, #0x2000"
    // );

    // asm volatile (
    // 	"str r8, [r9]"
    // );
    // register uint32_t r8 asm("r8");
    // *stackPointerTest = r8;
	// asm volatile (
	// 	"mov r6, #20\n\t"
	// 	"movw r7, #0xcccc\n\t"
	// 	"movt r7, #0x200c\n\t"
	// 	"str r6, [r7]"
	// );
    // SysTick_Config();s
	// uint32_t otherTestValue = 50;
	// otherTestValue = otherTestValue + *stackPointerTest;
	// // otherTestValue++;
	// // *stackPointerTest = 30;
	// otherTestValue = 70;
	// uint32_t newTestValue = 50;
    // process_frame->r2 = (uint32_t)0;
    // process_frame->r3 = 0;
    // process_frame->r12 = 0;
    // process_frame->pc = ((uint32_t)p);
    // // address of return function
    // // process_frame->lr = (uint32_t)del_process;
    // process_frame->lr = 0;
    // process_frame->psr = 0x21000000; //default PSR value
    // core_proc_table[i].sp = mem + 
    //     stack_size - 
    //     sizeof(hw_stack_frame_t);
}

void Dummy_Function(void){
	asm volatile (
		"MOV r0, sp\n\t"
		"MSR psp, r0"
		);
	// while(1){
	// 	Other_Dummy_Function();
	// }
	task_blink_led();
}

void Other_Dummy_Function(void){
	asm volatile (
		"MOV r0, sp\n\t"
		"MSR psp, r0"
		);
	while(1){}
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
  // asm volatile (
  // 	"MRS %0, psp\n\t" 
  //   "LDMIA %0!, {r4-r11}\n\t"
  //   "MSR psp, %0\n\t"  : "=r" (scratch)
  //   );
	// asm volatile (
	// 	"MRS %0, psp\n\t" : "=r" (scratch)
	// );
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
	// Handler is returning from LED and should call motor
	// Dummy function
	if (handlingLedOrMotor == 1)
	{
		// task1PSP = (uint32_t)rd_program_stack_ptr();
		// int32_t *MSP = (int32_t *)rd_main_stack_ptr();

	// 	ledInitialized = 1;
		handlingLedOrMotor = 0;
		// if (motorInitialized == 1)
		// {
			// Load up context and run that way
		// stack = (uint32_t *)rd_main_stack_ptr();
		// *((uint32_t*)stack) = THREAD_RETURN; 
		void *oldStackP = rd_program_stack_ptr();
		asm volatile(
			"MSR psp, %0"
		: : "r" (task1PSP)
			// "MSR psp, =0x200000cc" 
		);
		void *newStackP = rd_program_stack_ptr();

		asm volatile (
			"MRS r0, psp\n\t"
		);
		asm volatile (
			"LDMIA r0!, {r4-r11}"
		);

		// void *thirdStackP = rd_program_stack_ptr();

		// task1PSP = (uint32_t *)((0x200000cc));

		asm volatile(
			"MSR psp, %0"
		: : "r" (&task1Stack[293])
		);


		/** Method of changing to PSP by modifying MSP value */
		// stack = (uint32_t *)rd_main_stack_ptr();
		// *((uint32_t*)stack) = THREAD_RETURN; 

		/** Tell program to use PSP by setting LR **/
		asm volatile(
			// "MOV lr, =0xFFFFFFFD\n\t"
			"movw lr, #0xFFFD\n\t"
			"movt lr, #0xFFFF\n\t"
			"bx lr"
		);

		// write_program_stack_ptr((void *)0x200000cc);
		// void *newPSP = (void *)0x200000cc;
		
		// }
		// else
		// {
		// 	motorInitialized = 1;
			
		// }
	}
	// Handler is returning from MOTOR and should call LED
	else
	{
		// task2PSP = (uint32_t)rd_program_stack_ptr();
		// int32_t *MSP = (int32_t *)rd_main_stack_ptr();

	// 	ledInitialized = 1;
		handlingLedOrMotor = 1;
	
		// Load up context and run that way
		stack = (uint32_t *)rd_main_stack_ptr();
		*((uint32_t*)stack) = THREAD_RETURN; 
		write_program_stack_ptr(stackPointers.task1SP);
	}
	
	// task_spin_motors();
	// int32_t temporaryint32_t = 2;
	// int32_t * dubPointer = &temporaryint32_t;
	// *dubPointer = 2;
	// Bring out the context of the LED
	// asm(
	// "mov     r0, r0\n\t"
	// "mov     r0, r0\n\t"
	// "mov     r0, r0\n\t"
	// "mov     r0, r0"
	// );
	// void *motorPointer = &init_motors;
	// asm(
	// "mov     %[temporaryDub], r15" : [temporaryDub] "=r" (motorPointer)
	// );

	// dubPointer = dubPointer;
}