// Demo runing two threads round-robin.
// Both threads use wait loops.  Each thread will execute
// for one quantum then be preempted.
//
#include <ChibiOS_ARM.h>
//------------------------------------------------------------------------------
// 32 byte stack beyond task switch and interrupt needs
static THD_WORKING_AREA(waBlink, 32);

static THD_FUNCTION(blink, arg) {
  // blink twice per second
  pinMode(13, OUTPUT);
  uint32_t next = millis();
  while (1) {
    digitalWrite(13, HIGH);
    next += 100;
    while((int32_t)(millis() - next) < 0) {}
    digitalWrite(13, LOW);
    next += 400;
    while((int32_t)(millis() - next) < 0) {}
  }
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  // wait for USB Serial
  while (!Serial) {}
  
  if (CH_CFG_TIME_QUANTUM == 0) {
    Serial.println("CH_CFG_TIME_QUANTUM must be nonzero for round-robin.");
    while(1) {}
  }
  // start kernel and continue main thread as loop
  chBegin(mainThread);
  while(1);
}
//------------------------------------------------------------------------------
// main thread and runs at NORMALPRIO
void mainThread() {
  // start blink thead
  chThdCreateStatic(waBlink, sizeof(waBlink), NORMALPRIO, blink, NULL);
  
  // print millis every second
  uint32_t next = 0;
  while (1) {
    next += 1000;
    while ((int32_t)(millis() - next) < 0) {}
    Serial.println(millis());
  }
}
//------------------------------------------------------------------------------
void loop() {
  // not used
}
