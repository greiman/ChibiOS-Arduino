// Example of how a handler task can be triggered from an ISR
// by using a binary semaphore.
#include <ChibiOS_AVR.h>

// pin to trigger interrupt
const uint8_t INTERRUPT_PIN = 2;

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
  // On AVR this forces compiler to save registers r18-r31.
  CH_IRQ_PROLOGUE();
  /* IRQ handling code, preemptable if the architecture supports it.*/
  
  // Only ISR processing is to save time
  tIsr = micros();
  
  chSysLockFromIsr();
  /* Invocation of some I-Class system APIs, never preemptable.*/
  
  // signal handler task
  chBSemSignalI(&isrSem);
  chSysUnlockFromIsr();
 
  /* More IRQ handling code, again preemptable.*/
 
  // Perform rescheduling if required.
  CH_IRQ_EPILOGUE();  
}
//------------------------------------------------------------------------------
// handler task for interrupt
static WORKING_AREA(waThd1, 200);
msg_t handler(void *arg) {
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
  
  pinMode(INTERRUPT_PIN, OUTPUT);

  chBegin(mainThread);
  while (1);
}
//------------------------------------------------------------------------------

void mainThread() {
  // start handler task
  chThdCreateStatic(waThd1, sizeof(waThd1), NORMALPRIO + 1, handler, NULL);
  
  // attach interrupt function
  attachInterrupt(0, isrFcn, RISING);
  while (1) {
    // cause an interrupt - normally done by external event
    Serial.println("High");
    digitalWrite(INTERRUPT_PIN, HIGH);
    Serial.println("Low");
    digitalWrite(INTERRUPT_PIN, LOW);
    Serial.println();
    chThdSleepMilliseconds(1000);
  }
}
//------------------------------------------------------------------------------
void loop() {
  // not used
}
