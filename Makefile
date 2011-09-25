ARCH = X86_64
CFLAGS = -std=gnu99 -D__$(ARCH)__ -g -Os

ifeq ($(ARCH), MSP430)
  CC = msp430-gcc
  CFLAGS += -mmcu=$(MCU)
  objects = test-msp430.elf
else
  CC = gcc
  objects = test
endif

.PHONY: all
all: clean $(objects)

contr.o: contr.c contr.h

test.o: test.c contr.h

test-msp430.o: test-msp430.c contr.h

pump.o: pump.c contr.h

exp_board.o: exp_board.c contr.h
	$(CC) $(CFLAGS) $^ -c

test: CFLAGS += -DDEBUG
test: contr.o test.o
	$(CC) $(CFLAGS) $^ -o $@

test-msp430.elf: MCU = msp430g2231
test-msp430.elf: contr.o test-msp430.o
	$(CC) $(CFLAGS) $^ -o $@

pump.elf: MCU = msp430g2231
pump.elf: contr.o pump.o
	$(CC) -mmcu=msp430g2231 $(CFLAGS) $^ -o $@

exp_board.elf: MCU = msp430fr5739
exp_board.elf: CFLAGS += -DMAX_CONTEXTS=6 -DMAX_STACK_SIZE=7
exp_board.elf: contr.o exp_board.o
	$(CC) -mmcu=msp430fr5739 $(CFLAGS) $^ -o $@

.PHONY: debug
debug:
	msp430-gdb contr.elf --eval-command="target remote localhost:2000"

.PHONY: clean
clean:
	rm -f *.o *.elf *.gch test



