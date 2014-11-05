#ifndef __SYSTICK_CONTEXT_SWITCHER_H__
#define __SYSTICK_CONTEXT_SWITCHER_H__

void main(void);

void SysTick_Handler(void);
void InitializeTaskStacks(void);
static inline void * rd_program_stack_ptr(void);

#endif
 //__SYSTICK_CONTEXT_SWITCHER_H__