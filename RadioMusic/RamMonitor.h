// Teensy 3.x RAM Monitor
// copyright by Adrian Hunt (c) 2015 - 2016
//
// simplifies memory monitoring;  providing both "raw"
// memory information  and with frequent  calls to the
// run() function, adjusted information with simulated
// stack allocations. memory is also monitored for low
// memory state and stack and heap crashes.
//
// raw memory information methods:
//
//     int32_t unallocated() const;
//   calculates space between  heap and current stack.
//   will return negitive if heap and stack currently
//   overlap, corruption is very likely.
//
//     uint32_t stack_used() const;
//   calculates the current stack size.
//
//     uint32_t heap_total() const;
//   return the heap size.
//
//     uint32_t heap_used() const;
//   returns allocated heap.
//
//     uint32_t heap_free() const;
//   returns unused heap.
//
//     int32_t free() const;
//   calculates total free ram; unallocated and unused
//   heap. note that this uses the current stack size.
//
//     uint32_t total() const;
//   returns total physical ram.
//
// extended memory information.  These methods require
// the  RamMonitor  object to  be initialized  and the
// run() method called regularly.
//
//     uint32_t stack_total() const;
//   returns the  memory required for  the stack. this
//   is determind by historical stack usage.
//
//     int32_t stack_free() const;
//   returns stack  space that can be used  before the
//   stack grows and total size is increased.
//
//     int32_t adj_unallocd() const;
//   calculates  unallocated  memory,  reserving space
//   for the stack.
//
//     int32_t adj_free() const;
//   calculates  total  free  ram  by  using  adjusted
//   unallocated and unused heap.
//
//     bool warning_lowmem() const;
//     bool warning_crash() const;
//   return warning states: low memory is flagged when
//   adjusted unallocated memory is below a set value.
//   crash is flagged when  reserved stack space over-
//   laps heap and there is a danger of corruption.
//
//     void initialize();
//   initializes the RamMonitor  object enabling stack
//   monitoring and the extended information methods.
//
//     void run();
//   detects stack growth and updates memory warnings.
//   this function must be called regulary.
//
// when using the extended memory information methods,
// a single  RamMonitor  object  should be  create  at
// global level.  two static  constants define  values
// that control stack allocation step size and the low
// memory  warning level.  these values  are in bytes.
// the stack allocation step must be divisable by 4.
//
//     static const uint16_t STACKALLOCATION;
//     static const uint16_t LOWMEM;
//

#ifndef RAMMONITOR_H
#define RAMMONITOR_H "1.0"

#include <malloc.h>
#include <inttypes.h>

extern int* __brkval;   // top of heap (dynamic ram): grows up towards stack
extern char _estack;    // bottom of stack, top of ram: stack grows down towards heap

class RamMonitor {
private:
  typedef uint32_t MemMarker;
  typedef uint8_t  MemState;

  // user defined consts
  static const uint16_t  STACKALLOCATION    = 1024;  // stack allocation step size: must be 32bit boundries, div'able by 4
  static const uint16_t  LOWMEM             = 4096;  // low memory warning: 4kb (less than between stack and heap)

  // internal consts
  static const uint32_t  HWADDRESS_RAMSTART =
#if defined(__MK20DX256__)
                                        0x1FFF8000;  // teensy 3.1 (? 3.2 ?)
#elif defined(__MKL26Z64__)
                                        0x????????;  // teensy LC
#else
                                        0x1FFFE000;  // teensy 3.0
#endif
  static const MemMarker MEMMARKER    = 0x524D6D6D;  // chars RMmm ... Ram Monitor memory marker
  static const uint16_t  MARKER_STEP  = STACKALLOCATION / sizeof(MemMarker);

  static const MemState msOk          = 0;
  static const MemState msLow         = 1;
  static const MemState msCrash       = 2;

  MemMarker* _mlastmarker;    // last uncorrupted memory marker
  MemState   _mstate;         // detected memory state

  void _check_stack() {
    int32_t free;

    // skip markers already comsumed by the stack
    free = ((char*) &free) - ((char*) _mlastmarker);
    if(free < 0) {
      int32_t steps;

      steps = free / STACKALLOCATION; // note steps will be negitive
      if(free % STACKALLOCATION)
        --steps;

      _mlastmarker += MARKER_STEP * steps;
    };

    // check last marker and move if corrupted
    while((*_mlastmarker != MEMMARKER) && (_mlastmarker >= (MemMarker*) __brkval))
      _mlastmarker -= MARKER_STEP;
  };
public:
  int32_t unallocated() const { char tos; return &tos - (char*) __brkval; };  // calcs space between heap and stack (current): will be negitive if heap/stack crash
  uint32_t stack_used() const { char tos; return &_estack - &tos; };          // calcs stack size (current): grows into unallocated
  uint32_t heap_total() const { return mallinfo().arena; };                   // returns heap size: grows into unallocated
  uint32_t heap_used() const { return mallinfo().uordblks; };                 // returns heap allocated
  uint32_t heap_free() const { return mallinfo().fordblks; };                 // returns free heap

  int32_t free() const { return unallocated() + heap_free(); };               // free ram: unallocated and unused heap
  uint32_t total() const { return &_estack - (char*) HWADDRESS_RAMSTART; };   // physical ram

  // these functions (along with initialize and run)
  // create the ellusion of stack allocation.
  uint32_t stack_total() {                                                  // uses memory markers to "alloc" unallocated
    _check_stack();
    return &_estack - (char*) _mlastmarker;
  };

  int32_t stack_free() {                                                    // calc stack usage before next marker corruption
    char tos;

    _check_stack();
    return &tos - (char*) _mlastmarker;
  };

  int32_t adj_unallocd() {                                                  // calcs space between heap and "alloc'd" stack: will be negitive if heap/stack crash
    _check_stack();
    return ((char*) _mlastmarker) - (char*) __brkval;
  };

  int32_t adj_free() { return adj_unallocd() + heap_free(); };              // free ram: unallocated and unused heap

  bool warning_lowmem() const { return (_mstate & msLow); };        // returns true when unallocated memory is < LOWMEM
  bool warning_crash() const { return (_mstate & msCrash); };       // returns true when stack is in danger of overwriting heap

  void initialize() {
    MemMarker* marker = (MemMarker*) &_estack;      // top of memory
    int32_t    size;
    int32_t    steps;

    // skip current stack;
    size = &_estack - (char*) &marker; // current stack size: marker address is tos
    steps = size / STACKALLOCATION;
    if(size % STACKALLOCATION)
      ++steps;

    marker -= MARKER_STEP * steps;

    // record current top of stack
    _mlastmarker = marker;
    _mstate = msOk;

    // mark unused ram between top of stack and top of heap
    while(marker >= (MemMarker*) __brkval) {
      *marker = MEMMARKER;                 // write memory marker
      marker -= MARKER_STEP;
    };
  };

  void run() {
    int32_t unallocd = adj_unallocd();   // calls _check_stack() internally

    if(unallocd < 0)
      _mstate = msCrash | msLow;
    else if(unallocd < LOWMEM)
      _mstate = msLow;
    else
      _mstate = msOk;
  };

};

#endif
