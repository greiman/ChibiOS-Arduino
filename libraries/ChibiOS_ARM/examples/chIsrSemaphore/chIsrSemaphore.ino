// Example of how a handler task can be triggered from an ISR
// by using a binary semaphore.
#include <ChibiOS_ARM.h>

// pins to generate interrupts - these pins must be connected with a wire
const uint8_t INPUT_PIN = 2;
const uint8_t OUTPUT_PIN = 3;

// initialize semaphore as taken
BSEMAPHORE_DECL(isrSem, 1);

// ISR entry time
volatile uint32_t tIsr = 0;
//------------------------------------------------------------------------------
// Fake ISR, normally
// void isrFcn() {
// would be replaced by somthing like
// CH_IRQ_HANDLER(INT0_vect) {
//
void isrFcn() {

  // on ARM CH_IRQ_PROLOGUE is void
  CH_IRQ_PROLOGUE();
  /* IRQ handling code, preemptable if the architecture supports it.*/

  // Only ISR processing is to save time
  tIsr = micros();

  chSysLockFromISR();
  /* Invocation of some I-Class system APIs, never preemptable.*/

  // signal handler task
  chBSemSignalI(&isrSem);
  chSysUnlockFromISR();

  /* More IRQ handling code, again preemptable.*/

  // Perform rescheduling if required.
  CH_IRQ_EPILOGUE();
}
//------------------------------------------------------------------------------
// handler task for interrupt
static THD_WORKING_AREA(waThd1, 200);

static THD_FUNCTION(handler, arg) {
  while (1) {
    // wait for event
    chBSemWait(&isrSem);
    
    // print elapsed time
    uint32_t t = micros();
    Serial.print("Handler: ");
    Serial.println(t - tIsr);
  }
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}
  
  // setup and check pins
  pinMode(INPUT_PIN, INPUT);
  pinMode(OUTPUT_PIN, OUTPUT);
  digitalWrite(OUTPUT_PIN, HIGH);
  bool shouldBeTrue = digitalRead(INPUT_PIN);
  digitalWrite(OUTPUT_PIN, LOW);
  if (digitalRead(INPUT_PIN) || !shouldBeTrue) {
    Serial.print("pin ");
    Serial.print(INPUT_PIN);
    Serial.print(" must be connected to pin ");
    Serial.println(OUTPUT_PIN);
    while (1) {}
  }

  // Start ChibiOS
  chBegin(mainThread);
  while (1) {}
}
//------------------------------------------------------------------------------
void mainThread() {
  // start handler task
  chThdCreateStatic(waThd1, sizeof(waThd1), NORMALPRIO + 1, handler, NULL);
  
  // attach interrupt function
  attachInterrupt(INPUT_PIN, isrFcn, RISING);
  while (1) {
    // cause an interrupt - normally done by external event
    Serial.println("High");
    digitalWrite(OUTPUT_PIN, HIGH);
    Serial.println("Low");
    digitalWrite(OUTPUT_PIN, LOW);
    Serial.println();
    chThdSleepMilliseconds(1000);
  }
}
//------------------------------------------------------------------------------
void loop() {
  // not used
}