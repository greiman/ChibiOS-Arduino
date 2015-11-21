// Simple demo of three threads
// LED blink thread, print thread, and main thread
#include <ChibiOS_ARM.h>
// Redefine AVR Flash string macro as nop for ARM
#undef F
#define F(str) str

const uint8_t LED_PIN = 13;

volatile uint32_t count = 0;

//------------------------------------------------------------------------------
// thread 1 - high priority for blinking LED
// 64 byte stack beyond task switch and interrupt needs
static THD_WORKING_AREA(waThread1, 64);

static THD_FUNCTION(Thread1 ,arg) {
  pinMode(LED_PIN, OUTPUT);

  // Flash led every 200 ms.
  while (1) {
    // Turn LED on.
    digitalWrite(LED_PIN, HIGH);

    // Sleep for 50 milliseconds.
    chThdSleepMilliseconds(50);

    // Turn LED off.
    digitalWrite(LED_PIN, LOW);

    // Sleep for 150 milliseconds.
    chThdSleepMilliseconds(150);
  }
}
//------------------------------------------------------------------------------
// thread 2 - print main thread count every second
// 100 byte stack beyond task switch and interrupt needs
static THD_WORKING_AREA(waThread2, 100);

static THD_FUNCTION(Thread2 ,arg) {

  // print count every second
  while (1) {
    // Sleep for one second.
    chThdSleepMilliseconds(1000);

    // Print count for previous second.
    Serial.print("Count: ");
    Serial.print(count);

    // Print unused stack for threads.
    Serial.print(", Unused Stack: ");
    Serial.print(chUnusedStack(waThread1, sizeof(waThread1)));
    Serial.print(' ');
    Serial.print(chUnusedStack(waThread2, sizeof(waThread2)));
    Serial.print(' ');
    Serial.println(chUnusedHeapMain());

    // Zero count.
    count = 0;
  }
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  // wait for USB Serial
  while (!Serial) {}

  // read any input
  delay(200);
  while (Serial.read() >= 0) {}

  chBegin(mainThread);
  // chBegin never returns, main thread continues with mainThread()
  while(1) {}
}
//------------------------------------------------------------------------------
// main thread runs at NORMALPRIO
void mainThread() {

  // start blink thread
  chThdCreateStatic(waThread1, sizeof(waThread1),
                          NORMALPRIO + 2, Thread1, NULL);

  // start print thread
  chThdCreateStatic(waThread2, sizeof(waThread2),
                          NORMALPRIO + 1, Thread2, NULL);

  // increment counter
  while (1) {
    // must insure increment is atomic in case of context switch for print
    // should use mutex for longer critical sections
    noInterrupts();
    count++;
    interrupts();
  }
}
//------------------------------------------------------------------------------
void loop() {
 // not used
}