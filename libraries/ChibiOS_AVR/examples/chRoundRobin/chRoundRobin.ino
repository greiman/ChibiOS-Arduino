// demo runing two tasks round-robin
//
#include <ChibiOS_AVR.h>
//------------------------------------------------------------------------------
// 32 byte stack beyond task switch and interrupt needs
static WORKING_AREA(waBlink, 32);

static msg_t blink(void *arg) {
  // blink twice per second
  pinMode(13, OUTPUT);
  while (1) {
    digitalWrite(13, HIGH);
    chThdSleepMilliseconds(100);
    digitalWrite(13, LOW);
    chThdSleepMilliseconds(400);
  }
  return 0;
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  // wait for USB Serial
  while (!Serial) {}
  
  // start kernel and continue main thread as loop
  chBegin(mainThread);
  while(1);
}
//------------------------------------------------------------------------------
// main thread and runs at NORMALPRIO
void mainThread() {
  uint32_t m = 1 + millis()/1000;
  
  // start blink thead
  chThdCreateStatic(waBlink, sizeof(waBlink), NORMALPRIO, blink, NULL);
  
  // print time every second
  while (1) {
    Serial.println(m++);
  
    // sleep until next second
    chThdSleepMilliseconds(1000*m - millis());
  }
}
//------------------------------------------------------------------------------
void loop() {
  // not used
}