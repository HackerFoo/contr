contr - A continuation based function reactive library in C for real-time embedded applications
==============================================================================================

__contr__ is a library designed to allow a functional reactive programming style to be used in C for embedded applications. It was originally written to make writing a simple timer running on TI's MSP430 LaunchPad (to control a pump for my aeroponic garden) more fun.

USE
---

Define indices into the contexts array for each unique reactive value.

    enum reactives {
      R_TIME = 0,
      R_BUTTON
    };

In the code where you want to use these values, get the stack pointer with `GET_SP`, and store it.

     word *start;
     GET_SP(start);

Call the `r_{type}()` function, passing `start`, `NULL`, the reactive value's index, and the initial value.

    t = r_uint16_t(start, NULL, R_TIME, 0);

If you want to evaluate a block of code only when the reactive value changes, wrap it in a `BLOCK`.  Make sure that any variables you want to stay updated are not on the stack. (e.g. static)

    static uint16_t x;
    BLOCK(a_block) {
      t = r_uint16_t(start, a_block, R_TIME, 0);
      x = computationally_expensive_function(t);
    }

In your interrupt handler, call `r_call_{type}()` with the reactive value's index and the new value.


    __attribute__((interrupt(TIMERA0_VECTOR)))
    void timer_isr() {
      static uint16_t timer_cnt = 0;
      r_call_uint16_t(R_TIME, timer_cnt++);
    }

Make sure there are enough contexts and stack storage for your program by defining the `MAX_CONTEXTS` and `MAX_STACK_SPACE` appropriately, by passing `-DMAX_CONTEXTS=x` and `-DMAX_STACK_SPACE=y` to gcc when compiling the library.  If you define `DEBUG`, __contr__ will warn you if you don't have enough stack space.  See the Makefile for examples.

Now you're ready to use __contr__ in your program!

Building the examples
----------------------

Run `make clean` first.

* __test__: `make ARCH=X86 test` or `make ARCH=X86_64 test`
* __test-msp430__ (for MSP430 LaunchPad): `make ARCH=MSP430 test-msp430.elf`
* __pump__ (for MSP430 LaunchPad): `make ARCH=MSP430 pump.elf`
* __exp\_board__ (for FRAM Experimenter's Board): `make ARCH=MSP430 exp\_board.elf

Adding Architectures
--------------------

The `GET_SP` and `SET_SP` functions need to be defined in assembly.  Also, if the stack does not grow down (towards a lower address), more work will need to be done.  I intend to add support for the AVR ATmega family of microprocessors.

Credits
-------

The method of using continuations in C was inspired by Dan Piponi's (sigfpe) [Continuations in C](http://homepage.mac.com/sigfpe/Computing/continuations.html).
