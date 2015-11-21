
/**
 * \file
 * \brief ChibiOS for Due and Teensy 3.0
 */
//#include <unistd.h>
/** should use uinstd.h to define sbrk but Due causes a conflict
 * \param[in] incr Must call with zero here
 * \return start of main stack
 */
char* sbrk(int incr);
#include <Arduino.h>
#include <ChibiOS_ARM.h>
/** define loop */
extern void loop();
//------------------------------------------------------------------------------
/** calibration factor for delayMS */
#define CAL_FACTOR (F_CPU/7000)
/** delay between led error flashes
 * \param[in] millis milliseconds to delay
 */
static void delayMS(uint32_t millis) {
  uint32_t i;
  uint32_t iterations = millis * CAL_FACTOR;
  for(i = 0; i < iterations; ++i) {
    asm volatile("nop\n\t");
  }
}
//------------------------------------------------------------------------------
/** Blink error pattern
 *
 * \param[in] n  number of short pulses
 */
static void errorBlink(int n) {
	noInterrupts();
  pinMode(13, OUTPUT);
  for (;;) {
    int i;
    for (i = 0; i < n; i++) {
      digitalWrite(13, 1);
      delayMS(300);
      digitalWrite(13, 0);
      delayMS(300);
    }
    delayMS(2000);
  }
}
//------------------------------------------------------------------------------
// catch Teensy and Due exceptions
/** Hard fault - blink one short flash every two seconds */
void hard_fault_isr()	{errorBlink(1);}

/** Hard fault - blink one short flash every two seconds */
void HardFault_Handler() 	{errorBlink(1);}

/** Bus fault - blink two short flashes every two seconds */
void bus_fault_isr() {errorBlink(2);}
/** Bus fault - blink two short flashes every two seconds */
void BusFault_Handler() {errorBlink(2);}

/** Usage fault - blink three short flashes every two seconds */
void usage_fault_isr() {errorBlink(3);}
/** Usage fault - blink three short flashes every two seconds */
void UsageFault_Handler() {errorBlink(3);}

/** Dummy init - already done in startup */
void hal_lld_init(void) {
}
/** Dummy init - already done in  startup */
void boardInit(void) {
}
//------------------------------------------------------------------------------
/** fill heap on Teensy */
#if 0  // Teensy startup_early_hook has changed
void startup_early_hook() {
 uint32_t* end = &_estack - 100;
 uint32_t* p = &_ebss;
 while (p < end) *p++ = MEMORY_FILL_PATTERN;
}
#endif  // Teensy startup_early_hook has changed
//------------------------------------------------------------------------------
/** continuation of main thread */
static void (*mainFcn)() = 0;
/**
 * Start ChibiOS/RT - does not return
 * \param[in] mainThread Function to be called before repeated calls
 *                       to loop().
 */
void chBegin(void (*mainThread)()) {
  extern sysTickEnabled;
  sysTickEnabled = 1;
  mainFcn = mainThread;
  uint32_t msp, psp, reg;
  // disable interrupts
  asm volatile ("cpsid   i");
  // set Main Stack pointer to top of SRAM
  msp = (uint32_t)(&_estack);
  asm volatile ("msr     MSP, %0" : : "r" (msp));
  // Process Stack initialization for main thread
  // allow HANDLER_STACK_SIZE entries for handler stack */
  psp = (uint32_t)(&_estack - HANDLER_STACK_SIZE);
  asm volatile ("msr     PSP, %0" : : "r" (psp));
  reg = 2;
  asm volatile ("msr     CONTROL, %0" : : "r" (reg));
  asm volatile ("isb");
  {
    uint32_t* p = (uint32_t*)sbrk(0);
    // fill memory - loop works since compiler dosen't use stack
    while (p < &_estack) *p++ = MEMORY_FILL_PATTERN;
  }
  halInit();
  chSysInit();
  interrupts();
  if (mainFcn) mainFcn();
  while(1) {loop();}
}
//------------------------------------------------------------------------------
/**
 * Determine unused bytes in the handler stack area
 *
 * \return number of unused bytes
 */
size_t chUnusedHandlerStack() {

  uint32_t* used = &_estack - HANDLER_STACK_SIZE;
  while (used < &_estack) {
    if (*used != MEMORY_FILL_PATTERN) break;
    used++;
  }
  return sizeof(uint32_t)*(used - &_estack + HANDLER_STACK_SIZE);
}
//------------------------------------------------------------------------------
/**
 * Determine unused bytes in the heap/loop stack area
 *
 * \return number of unused bytes
 */
size_t chUnusedHeapMain() {
  uint32_t* bgn;
  uint32_t* end;
  uint32_t* brk = (uint32_t*) sbrk(0);
  if (*brk == MEMORY_FILL_PATTERN) {
    bgn = brk - 1;
    while (bgn >= &_ebss) {
      if (*bgn != MEMORY_FILL_PATTERN) break;
      bgn--;
    }
    end = brk + 1;
    while (end < (&_estack - HANDLER_STACK_SIZE)) {
      if (*end != MEMORY_FILL_PATTERN) break;
      end++;
    }
    return sizeof(uint32_t)*(end - bgn -1);
  }
  return 0;
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
  while (startp < endp)
    if(*startp++ == CH_STACK_FILL_VALUE) ++n;
#endif
  return n;
}