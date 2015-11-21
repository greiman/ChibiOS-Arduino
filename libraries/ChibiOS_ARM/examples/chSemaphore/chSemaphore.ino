// Example of counting semaphore
#include <ChibiOS_ARM.h>

// declare and initialize a semaphore for limiting access
SEMAPHORE_DECL(twoSlots, 2);

// data structures and stack for thread 2
static THD_WORKING_AREA(waTh2, 100);

// data structures and stack for thread 3
static THD_WORKING_AREA(waTh3, 100);
//------------------------------------------------------------------------------
static THD_FUNCTION(thdFcn, name) {
 while (true) {

  // wait for slot
  chSemWait(&twoSlots);

    // only two threads can be in this region at a time
    Serial.println((char*)name);

    chThdSleep(1000);

    // exit region
    chSemSignal(&twoSlots);
  }
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  // wait for USB Serial
  while (!Serial) {}

  // initialize and start ChibiOS
  chBegin(chSetup);

  // should not return
  while(1);
}
//------------------------------------------------------------------------------
void chSetup() {
  // schedule thread 2
  chThdCreateStatic(waTh2, sizeof(waTh2), NORMALPRIO, thdFcn, (void*)"Th 2");

  // schedule thread 3
  chThdCreateStatic(waTh3, sizeof(waTh3), NORMALPRIO, thdFcn, (void*)"Th 3");

  // main thread is thread 1 at NORMALPRIO
  thdFcn((void*)"Th 1");
}
//------------------------------------------------------------------------------
void loop() {/* not used */}
