# Assume we're building on the target system, set the IOBASE according to
# machine name. Can override with "make IOBASE=something"
ifeq ($(IOBASE),)
MACHINE=$(shell uname -m)
ifeq ($(MACHINE),armv6l)
# BCM2835/2836
IOBASE=0x20000000
else ifeq ($(MACHINE),armv7l)
# BCM2837 
IOBASE=0x3F000000
else 
$(error Don't know how to build for MACHINE=$(MACHINE))
endif
endif

CPPFLAGS=-DIOBASE=$(IOBASE)

all: pifm

clean:
	rm -f pifm *.o
