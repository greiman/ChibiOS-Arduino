// This example illustrates cooperative scheduling. Cooperative scheduling
// simplifies multitasking since no preemptive context switches occur.
//
// You must call chYield() or other ChibiOS functions such as chThdSleep
// to force a context switch to other threads.
//
// Insert a delay(100) in the mainThread loop to see the effect of not
// using chThdSleep with cooperative scheduling.
//
// Setting CH_CFG_TIME_QUANTUM to zero disables the preemption for threads
// with equal priority and the round robin becomes cooperative.
// Note that higher priority threads can still preempt, the kernel
// is always preemptive.
//
#include <ChibiOS_ARM.h>
const uint8_t LED_PIN = 13;
volatile uint32_t count = 0;
volatile uint32_t maxDelay = 0;
//------------------------------------------------------------------------------
// thread 1 blink LED
// 64 byte stack beyond task switch and interrupt needs
static THD_WORKING_AREA(waThread1, 64);

static THD_FUNCTION(Thread1, arg) {
  pinMode(LED_PIN, OUTPUT);
  while (TRUE) {
    digitalWrite(LED_PIN, HIGH);
    chThdSleepMilliseconds(50);
    digitalWrite(LED_PIN, LOW);
    chThdSleepMilliseconds(150);
  }
}
//------------------------------------------------------------------------------
// thread 2 increment a counter and records max delay
// 64 byte stack beyond task switch and interrupt needs
static THD_WORKING_AREA(waThread2, 64);

static THD_FUNCTION(Thread2, arg) {

  while (TRUE) {
    count++;
    uint32_t t = micros();
    // yield so other threads can run
    chThdYield();
    t = micros() - t;
    if (t > maxDelay) maxDelay = t;
  }
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  // wait for USB Serial
  while (!Serial) {}
  Serial.println();
  
  if (CH_CFG_TIME_QUANTUM) {
    Serial.println("You must set CH_CFG_TIME_QUANTUM zero in");
    Serial.println("libraries/ChibiOS_ARM/src/utility/chconf.h");
    Serial.println("to enable cooperative scheduling.");
    while(1);
  }
  // start ChibiOS
  chBegin(mainThread);
  
  // chBegin() will not return
  while(1) {}
}
//------------------------------------------------------------------------------
// main thread runs at NORMALPRIO
void mainThread() {

  // start blink thread
  chThdCreateStatic(waThread1, sizeof(waThread1),
    NORMALPRIO, Thread1, NULL);
    
  // start count thread
  chThdCreateStatic(waThread2, sizeof(waThread2),
    NORMALPRIO, Thread2, NULL);
    
  Serial.println("Count, MaxDelay micros");

  while (1) {
    Serial.print(count);
    Serial.write(',');
    Serial.println(maxDelay);
    count = 0;
    maxDelay = 0;
    
    // allow other threads to run for 1 sec
    chThdSleepMilliseconds(1000);
  }
}
//------------------------------------------------------------------------------
void loop() {
  // not used
}