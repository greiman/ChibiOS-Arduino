#include "ch.h"

#define CONTEXT_OFFSET  12
#ifndef SCB_ICSR
#define SCB_ICSR        0xE000ED04
#endif  // SCB_ICSR
#define ICSR_PENDSVSET  0x10000000
//-----------------------------------------------------------------------------
__attribute__((naked))
void _port_switch(thread_t *ntp, thread_t *otp) {

  asm volatile ("push    {r4, r5, r6, r7, r8, r9, r10, r11, lr}"
                : : : "memory");
#if CORTEX_USE_FPU
  asm volatile ("vpush   {s16-s31}" : : : "memory");
#endif

  asm volatile ("str     sp, [%1, #12]                       \n\t"
                "ldr     sp, [%0, #12]" : : "r" (ntp), "r" (otp));

#if CORTEX_USE_FPU
  asm volatile ("vpop    {s16-s31}" : : : "memory");
#endif
  asm volatile ("pop     {r4, r5, r6, r7, r8, r9, r10, r11, pc}"
                : : : "memory");
}
//-----------------------------------------------------------------------------
void _port_thread_start(void) {
#if CH_DBG_SYSTEM_STATE_CHECK
  _dbg_check_unlock();
#endif
#if CH_DBG_STATISTICS
  _stats_stop_measure_crit_thd();
#endif
#if CORTEX_SIMPLIFIED_PRIORITY
  asm volatile ("cpsie   i");
#else
  asm volatile ("movs    r3, #0                                           \n\t"             
                "msr     BASEPRI, r3");
#endif
  asm volatile ("mov     r0, r5                                           \n\t"
                "blx     r4                                               \n\t"
                "movs    r0, #0                                           \n\t"             
                "bl      chThdExit");
}
//-----------------------------------------------------------------------------
__attribute__((naked))
void _port_switch_from_isr(void) {
#if CH_DBG_STATISTICS
  _stats_start_measure_crit_thd();
#endif
#if CH_DBG_SYSTEM_STATE_CHECK
  _dbg_check_lock();
#endif
  chSchDoReschedule();
#if CH_DBG_SYSTEM_STATE_CHECK
  _dbg_check_unlock();
#endif
#if CH_DBG_STATISTICS
  _stats_stop_measure_crit_thd();
#endif
//  _port_exit_from_isr();
#if CORTEX_SIMPLIFIED_PRIORITY
  asm volatile ("movw    r3, #:lower16:0xE000ED04                         \n\t"
                "movt    r3, #:upper16:0xE000ED04                         \n\t"
                "mov     r2, #0x10000000                                  \n\t"
                "str     r2, [r3, #0]                                     \n\t"
                "cpsie   i");
#else /* !CORTEX_SIMPLIFIED_PRIORITY */
  asm volatile ("svc     #0");
#endif /* !CORTEX_SIMPLIFIED_PRIORITY */
  while(1) {}
}
//-----------------------------------------------------------------------------
__attribute__((naked))
void _port_exit_from_isr(void) {
#if CORTEX_SIMPLIFIED_PRIORITY
  asm volatile ("movw    r3, #:lower16:0xE000ED04                         \n\t"
                "movt    r3, #:upper16:0xE000ED04                         \n\t"
                "mov     r2, #0x10000000                                  \n\t"
                "str     r2, [r3, #0]                                     \n\t"
                "cpsie   i");
#else /* !CORTEX_SIMPLIFIED_PRIORITY */
  asm volatile ("svc     #0");
#endif /* !CORTEX_SIMPLIFIED_PRIORITY */
  while(1) {}
}