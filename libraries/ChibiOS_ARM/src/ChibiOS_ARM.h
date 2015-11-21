/**
 * \file
 * \brief ChibiOS for Due and Teensy 3.0
 */

#ifndef ChibiOS_ARM_h
#define ChibiOS_ARM_h
#include <utility/ch.h>
#include <utility/hal.h>
#ifdef __cplusplus
extern "C"{
#endif  //  __cplusplus
//------------------------------------------------------------------------------
/** ChibiOS_ARM version YYYYMMDD */
#define CHIBIOS_ARM_VERSION 20151117
//------------------------------------------------------------------------------
#ifndef __arm__
#error ARM Teensy 3.x or Due required
#endif  // __arm__

/** number of 32-bit entries in main stack */
#define HANDLER_STACK_SIZE 100
/** fill pattern for heap, process stack, and main stack */
#define MEMORY_FILL_PATTERN 0X55555555UL
/** end of bss section */
extern unsigned long _ebss;
/** end of heap/stack area */
extern unsigned long _estack;
void chBegin(void (*mainThread)());
/** \return size of Handler stack in bytes */
inline size_t chHandlerStackSize() {return 4*HANDLER_STACK_SIZE;}
/** \return size of heap/loop stack in bytes */
inline size_t chHeapMainSize() {return 4*(&_estack - &_ebss - HANDLER_STACK_SIZE);}
size_t chUnusedHandlerStack();
size_t chUnusedHeapMain();
size_t chUnusedStack(void *wsp, size_t size);
#ifdef __cplusplus
} // extern "C"
#endif  // __cplusplus
#endif  // ChibiOS_ARM_h
