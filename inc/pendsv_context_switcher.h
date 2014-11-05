#ifndef __PENDSV_CONTEXT_SWITCHER_H__
#define __PENDSV_CONTEXT_SWITCHER_H__

void main(void);

void PendSV_Handler(void);
void InitializeTaskStacks(void);
static inline void * rd_program_stack_ptr(void);


#endif 
//_PENDSV_CONTEXT_SWITCHER_H__