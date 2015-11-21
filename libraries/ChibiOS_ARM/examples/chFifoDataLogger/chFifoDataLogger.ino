// Data logger based on a FIFO to decouple SD write latency from data
// acquisition timing.
//
// The FIFO uses two semaphores to synchronize between tasks.
#include <SPI.h>

#include <ChibiOS_ARM.h>
#include <SdFat.h>
//
// interval between points in units of 1000 usec
const uint16_t intervalTicks = 1;
//------------------------------------------------------------------------------
// SD file definitions
const uint8_t sdChipSelect = SS;
SdFat sd;
SdFile file;
//------------------------------------------------------------------------------
// Fifo definitions

// size of fifo in records
const size_t FIFO_SIZE = 200;

// count of data records in fifo
SEMAPHORE_DECL(fifoData, 0);

// count of free buffers in fifo
SEMAPHORE_DECL(fifoSpace, FIFO_SIZE);

// data type for fifo item
struct FifoItem_t {
  uint32_t usec;  
  int value;
  int error;
};
// array of data items
FifoItem_t fifoArray[FIFO_SIZE];
//------------------------------------------------------------------------------
// 64 byte stack beyond task switch and interrupt needs
static THD_WORKING_AREA(waThread1, 32);

static THD_FUNCTION(Thread1, arg) {
  // index of record to be filled
  size_t fifoHead = 0;

  // count of overrun errors
  int error = 0;

  // dummy data
  int count = 0;

  while (1) {
    chThdSleep(intervalTicks);
    // get a buffer
    if (chSemWaitTimeout(&fifoSpace, TIME_IMMEDIATE) != MSG_OK) {
      // fifo full indicate missed point
      error++;
      continue;
    }
    FifoItem_t* p = &fifoArray[fifoHead];
    p->usec = micros();

    // replace next line with data read from sensor such as
    // p->value = analogRead(0);
    p->value = count++;

    p->error = error;
    error = 0;

    // signal new data
    chSemSignal(&fifoData);
    
    // advance FIFO index
    fifoHead = fifoHead < (FIFO_SIZE - 1) ? fifoHead + 1 : 0;
  }
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  // wait for USB Serial
  while (!Serial) {}
  
  Serial.println(F("type any character to begin"));
  while(!Serial.available()); 
  
  // open file
  if (!sd.begin(sdChipSelect)
    || !file.open("DATA.CSV", O_CREAT | O_WRITE | O_TRUNC)) {
    Serial.println(F("SD problem"));
    sd.errorHalt();
  }
  
  // throw away input
  while (Serial.available()) {
    Serial.read();
    delay(10);
  }
  Serial.println(F("type any character to end"));
  
  // start kernel
  chBegin(mainThread);
  while(1);
}
//------------------------------------------------------------------------------
// main thread runs at NORMALPRIO
void mainThread() {
  // FIFO index for record to be written
  size_t fifoTail = 0;

  // time in micros of last point
  uint32_t last = 0;

  // remember errors
  bool overrunError = false;

  // start producer thread
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO + 1, Thread1, NULL);  

  // start SD write loop
  while (!Serial.available()) {
    // wait for next data point
    chSemWait(&fifoData);

    FifoItem_t* p = &fifoArray[fifoTail];
    if (fifoTail >= FIFO_SIZE) fifoTail = 0;

    // print interval between points
    if (last) {
      file.print(p->usec - last);
    } else {
      file.write("NA");
    }
    last = p->usec;
    file.write(',');
    file.print(p->value);
    file.write(',');
    file.println(p->error);

    // remember error
    if (p->error) overrunError = true;

    // release record
    chSemSignal(&fifoSpace);
    
    // advance FIFO index
    fifoTail = fifoTail < (FIFO_SIZE - 1) ? fifoTail + 1 : 0;
  }
  // close file, print stats and stop
  file.close();
  Serial.println(F("Done"));
  Serial.print(F("Thread1 unused stack: "));
  Serial.println(chUnusedStack(waThread1, sizeof(waThread1)));
  Serial.print(F("Heap/Main unused: "));
  Serial.println(chUnusedHeapMain());
  if (overrunError) {
    Serial.println();
    Serial.println(F("** overrun errors **"));
  }
  while(1);
}
//------------------------------------------------------------------------------
void loop() {
  // not used
}