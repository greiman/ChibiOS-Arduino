// example of memory pool and mailboxes with two senders and one receiver
#include <ChibiOS_ARM.h>
//------------------------------------------------------------------------------
// mailbox size and memory pool object count
const size_t MB_COUNT = 6;

// type for a memory pool object
struct PoolObject_t {
  char* name;
  int msg;
};
// array of memory pool objects
PoolObject_t PoolObject[MB_COUNT];

// memory pool structure
MEMORYPOOL_DECL(memPool, MB_COUNT, 0);
//------------------------------------------------------------------------------
// slots for mailbox messages
msg_t letter[MB_COUNT];

// mailbox structure
MAILBOX_DECL(mail, &letter, MB_COUNT);
//------------------------------------------------------------------------------
// data structures and stack for thread 2
static THD_WORKING_AREA(waTh2, 64);

// data structures and stack for thread 3
static THD_WORKING_AREA(waTh3, 64);
//------------------------------------------------------------------------------
// send message every 1000 ticks
static THD_FUNCTION(thdFcn, name) {
  int msg = 0;

  while (1) {

    // get object from memory pool
    PoolObject_t* p = (PoolObject_t*)chPoolAlloc(&memPool);
    if (!p) {
      Serial.println("chPoolAlloc failed");
      while(1);
    }
    // form message
    p->name = (char*)name;
    p->msg = msg++;

    // send message
    msg_t s = chMBPost(&mail, (msg_t)p, TIME_IMMEDIATE);
    if (s != MSG_OK) {
      Serial.println("chMBPost failed");
      while(1);  
    }
    chThdSleep(1000);    
  }
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  // wait for USB Serial
  while (!Serial) {}
  
  // initialize heap/stack memory and start ChibiOS
  chBegin(mainThread);
  while(1);
}
//------------------------------------------------------------------------------
// main is thread 1 and runs at NORMALPRIO
void mainThread() {

  // fill pool with PoolObject array
  for (size_t i = 0; i < MB_COUNT; i++) {
    chPoolFree(&memPool, &PoolObject[i]);
  }
  // schedule thread 2
  chThdCreateStatic(waTh2, sizeof(waTh2), NORMALPRIO, thdFcn, (void*)"Th 2");

  // schedule thread 3
  chThdCreateStatic(waTh3, sizeof(waTh2), NORMALPRIO, thdFcn, (void*)"Th 3");

  while (1) {
    PoolObject_t *p;

    // get mail
    chMBFetch(&mail, (msg_t*)&p, TIME_INFINITE);

    Serial.print(p->name);
    Serial.write(' ');
    Serial.println(p->msg);

    // put memory back into pool
    chPoolFree(&memPool, p);
  }
}
//------------------------------------------------------------------------------
void loop() {
  // not used
}