/**
 * \file
 * \brief ChibiOS for AVR Arduinos
 */
#include <stdlib.h>
#include <Arduino.h>
#include <ChibiOS_AVR.h>
/** define loop */
extern void loop();
/** continuation of main thread */
static void (*mainFcn)() = 0;
//------------------------------------------------------------------------------
/** end of heap */
extern char *__brkval;

#if defined(CORE_TEENSY) || ARDUINO == 104 || ARDUINO == 152
extern char __bss_end;

/** \return first loc after heap */
static inline uint8_t* stkBrk() {
 return __brkval ? __brkval : &__bss_end;
}
/** \return size of heap/main stack in bytes */
size_t chHeapMainSize() {
  return (char*)RAMEND - &__bss_end + 1;
}
#else  // CORE_TEENSY
/** \return first loc after heap */
static inline uint8_t* stkBrk() {
 return __brkval ? __brkval : __malloc_heap_start;
}
/** \return size of heap/main stack in bytes */
size_t chHeapMainSize() {
  return (char*)RAMEND - __malloc_heap_start + 1;
}
#endif  // CORE_TEENSY
//------------------------------------------------------------------------------
/**
 * Start ChibiOS/RT - does not return
 * \param[in] mainThread Function to be called before repeated calls
 *                       to loop().
 */
void chBegin(void (*mainThread)()) {
  mainFcn = mainThread;
  noInterrupts();
  halInit();
  chSysInit();
  // start of stack
  uint8_t* p = stkBrk();
  while (p < (uint8_t*)(&p - 10)) *p++ = CH_STACK_FILL_VALUE;
  
  interrupts();
  if (mainFcn) mainFcn();
  while(1) {loop();}
}
//------------------------------------------------------------------------------
/**
 * Determine unused bytes in the heap/main stack area
 *
 * \return number of unused bytes
 */
size_t chUnusedHeapMain() {
  size_t n = 0;
  uint8_t* p = stkBrk();
  while (p < RAMEND) {
    if(*p++ != CH_STACK_FILL_VALUE) break;
    n++;
  }
  return n;
}
//------------------------------------------------------------------------------
/**
 * Determine unused stack for a thread
 *
 * \param[in] wsp pointer to working space for thread
 * \param[in] size working space size
 *
 * \return number of unused stack locations
 */
size_t chUnusedStack(void *wsp, size_t size) {
  size_t n = 0;
#if CH_DBG_FILL_THREADS
  uint8_t *startp = (uint8_t *)wsp + sizeof(Thread);
  uint8_t *endp = (uint8_t *)wsp + size;
  while (startp < endp) {
    if(*startp++ != CH_STACK_FILL_VALUE) break;
    n++;
  }
#endif // CH_DBG_FILL_THREADS
  return n;
}
//------------------------------------------------------------------------------
/** Dummy init - already done in startup */
void hal_lld_init(void) {
}