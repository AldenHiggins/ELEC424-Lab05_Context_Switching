#ifndef __SYSTICK_CONTEXT_SWITCHER_H__
#define __SYSTICK_CONTEXT_SWITCHER_H__


void main(void);

// void SysTick_Handler(void);
void Dummy_Function(void);
void Other_Dummy_Function(void);
void InitializeTaskStacks(void (*p)(void), void * arg, void * stackaddr, int stack_size);
static inline void * rd_program_stack_ptr(void);
static inline void save_context(void);
static inline void load_context(void);


#endif
 //__SYSTICK_CONTEXT_SWITCHER_H__