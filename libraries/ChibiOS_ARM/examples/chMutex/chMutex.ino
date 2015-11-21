// Example of mutex to prevent print calls from being scrambled
#include <ChibiOS_ARM.h>
// declare and initialize a mutex for limiting access to threads
MUTEX_DECL(demoMutex);

// data structures and stack for thread 2
static THD_WORKING_AREA(waTh2, 100);

// data structures and stack for thread 3
static THD_WORKING_AREA(waTh3, 100);
//------------------------------------------------------------------------------
void notify(const char* name, int state) {

  // wait to enter print region
  chMtxLock(&demoMutex);
  
  // only one thread in this region while doing prints
  Serial.print(name);
  Serial.write(": ");
  Serial.println(state);
  
  // exit protected region
  chMtxUnlock(&demoMutex);
}
//------------------------------------------------------------------------------
static THD_FUNCTION(thdFcn, args) {
  while (true) {
    notify((const char*)args, 0);
    chThdSleep(1000);
    notify((const char*)args, 1);
    chThdSleep(1000);
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