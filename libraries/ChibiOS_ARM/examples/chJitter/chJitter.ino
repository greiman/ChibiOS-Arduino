// test of jitter in sleep for one tick
//
// idle main thread prints min and max time between sleep calls
//
// Note: access to shared variables tmin and tmax is not atomic
// so glitches are possible if context switch happens during main loop
// access to these variables.
//
#include <ChibiOS_ARM.h>

volatile  uint16_t tmax = 0;
volatile  uint16_t tmin = 0XFFFF;
//------------------------------------------------------------------------------
// thread for sleep interval
static THD_WORKING_AREA(waThread1, 64);

static THD_FUNCTION(Thread1, arg) {
  // initialize tlast
  chThdSleep(1);
  uint32_t tlast = micros();
  
  while (TRUE) {
  
    // sleep until next tick
    chThdSleep(1);
    
    // get wake time
    uint32_t tmp = micros();
    
    // calculate min and max interval between wake times
    uint16_t diff = tmp - tlast;
    if (diff < tmin) tmin = diff;
    if (diff > tmax) tmax = diff;
    tlast = tmp;
  }
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  // wait for USB Serial
  while (!Serial) {}
  
  // start ChibiOS
  chBegin(mainThread);
  while(1) {}
}
//------------------------------------------------------------------------------
void mainThread() {
  int np = 0;

  // start higher priority thread
  chThdCreateStatic(waThread1, sizeof(waThread1), 
    NORMALPRIO + 1, Thread1, NULL);
    
  while (1) {
    chThdSleepMilliseconds(1000);
    Serial.print(tmin);
    Serial.write(',');
    Serial.println(tmax);
    if (np++ == 10) {
      np = 0;
      tmin = 0XFFFF;
      tmax = 0;
      Serial.println("clear");
    }
  }
}
//------------------------------------------------------------------------------
void loop() {
  // not used
}