// Simple demo using a mutex for data sharing.
#include <ChibiOS_AVR.h>

const uint8_t X_PIN = 0;
const uint8_t Y_PIN = 1;
const uint8_t Z_PIN = 2;

//------------------------------------------------------------------------------
// Shared data, use volatile to insure correct access.

// Mutex for atomic access to data.
MUTEX_DECL(dataMutex);

// Time data was read.
volatile uint32_t dataT;

// Data X value.
volatile int dataX;

// Data Y value.
volatile int dataY;

// Data Z value.
volatile int dataZ;
//------------------------------------------------------------------------------
// Thread 1, high priority to read sensor.
// 64 byte stack beyond task switch and interrupt needs.
static WORKING_AREA(waThread1, 64);

static msg_t Thread1(void *arg) {


  // Read data every 10 ms.
  systime_t wakeTime = chTimeNow();

  while (1) {
    // Add ticks for 10 ms.
    wakeTime += MS2ST(10);
    chThdSleepUntil(wakeTime);

    // Use temp variables to acquire data.
    uint32_t tmpT = millis();
    int tmpX = analogRead(X_PIN);
    int tmpY = analogRead(Y_PIN);
    int tmpZ = analogRead(Z_PIN);

    // Lock access to data.
    chMtxLock(&dataMutex);

    // Copy tmp variables to shared data.
    dataT = tmpT;
    dataX = tmpX;
    dataY = tmpY;
    dataZ = tmpX;

    // Unlock data access.
    chMtxUnlock();
  }
  return 0;
}
//------------------------------------------------------------------------------
// thread 2 - print data every second.
// 128 byte stack beyond task switch and interrupt needs.
static WORKING_AREA(waThread2, 128);

static msg_t Thread2(void *arg) {

  // Print count every second.
  systime_t wakeTime = chTimeNow();
  while (1) {
    // Sleep for one second.
    wakeTime += MS2ST(1000);
    chThdSleepUntil(wakeTime);

    // Lock access to data.
    chMtxLock(&dataMutex);

    // Copy shared data to tmp variables.
    uint32_t tmpT = dataT;
    int tmpX = dataX;
    int tmpY = dataY;
    int tmpZ = dataZ;

    // Unlock data access.
    chMtxUnlock();

    // Print shared data.
    Serial.print(F("dataAge: "));
    Serial.print(millis() - tmpT);
    Serial.print(F(" ms, dataX: "));
    Serial.print(tmpX);
    Serial.print(F(", dataY: "));
    Serial.print(tmpY);
    Serial.print(F(", dataZ: "));
    Serial.print(tmpZ);

    // Print unused stack for threads.
    Serial.print(F(", Unused Stack: "));
    Serial.print(chUnusedStack(waThread1, sizeof(waThread1)));
    Serial.print(' ');
    Serial.print(chUnusedStack(waThread2, sizeof(waThread2)));
    Serial.print(' ');
    Serial.println(chUnusedHeapMain());
  }
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  // Wait for USB Serial.
  while (!Serial) {}

  // Read any input
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
}
//------------------------------------------------------------------------------
void loop() {
 // not used
}