// To build, just run 'make'

// To run:
//  ./pifm left_right.wav 103.3 22050 stereo
//  ./pifm sound.wav

// Created by Oliver Mattos and Oskar Weigl.
// Code quality = Totally hacked together.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License version 2 as publised
// by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but
// without any warranty, without even the implied warranty of merchantability
// or fitness for a particular purpose.  See the GNU GPL for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program.  If not, you may write to the Free Software Foundation,
// Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#define die(...) fprintf(stderr, __VA_ARGS__), exit(1)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void *iobase;

#define IO(register) *(volatile uin32_t *)(iobase+(register & 0x00ffffff))

#define CM_GP0CTL           0x7e101070
#define GP_FSEL0            0x7e200000
#define CM_GP0DIV           0x7e101074

#define divisor(KHz) ((500000/(KHz))*4096)

void shutdown(int status)
{
    IO(CM_GP0CTL) = 0x5a000000;                                         // disable GPCLOCK
    IO(GP_FSEL0) &= ~(7 << 12);                                         // and set GPIO4 normal function
}

int main(int argc, char *argv[])
{
    if (argc == 1) die("Usage:\n\
    \n\
    pifm KHz < pcm.dat\n\
\n\
Modulate specified frequency on GPIO4, +/- 75KHz based on 16-bit signed\n\
samples read from stdin, in the machines native-endianness. \n\
\n\
Such data can be produced with:\n\
\n\
    sox <input options> <input file> -b 16 -c 1 -e signed -t raw -  \n\
");
    
    if (isatty(0)) die("Expected 16-bit signed data on stdin\n");
   
    uint32_t KHz = atoi(argv[1]);
    
    int devmem=open("/dev/mem", O_RDWR | O_SYNC);
    if (devmem < 0) die("Can't open /dev/mem (are you root?)\n");

    iobase = mmap(NULL, 0x01000000, PROT_READ | PROT_WRITE, MAP_SHARED, devmem, IOBASE);
    if (iobase == MAP_FAILED) die("Can't mmap 0X%X\n", IOBASE);

    signal(SIGINT, shutdown);
    signal(SIGTERM, shutdown);
    signal(SIGHUP, shutdown);
    signal(SIGQUIT, shutdown);
    
    # enable clock on GPIO4
    IO(GP_FSEL0) &= ~(7<<12);
    IO(GP_FSEL0) |= 4<<12;

    # turn the clock on
    IO(CM_GP0CTL) = 0x5a002016; // Clock from PLLD==500MHz==500000Khz

    uint32_t min=divisor(KHz-75);
    uint32_t center=divisor(KHz+75);
    
    int16 data[
    
    while (got = read(0, &data, sizeof(data))
    {
        for (int i = 0; i < got; i++)
        {
            float value = data[i] * 4 * volume;                         // modulation index (AKA volume!)
            value += fracerror;                                         // error that couldn't be encoded from last time.
            int intval = (int)(round(value));                           // integer component
            float frac = (value - (float)intval + 1) / 2;
            unsigned int fracval = round(frac * clocksPerSample);       // the fractional component
            timeErr = timeErr - int (timeErr) + clocksPerSample;

            fracerror = (frac - (float)fracval * (1.0 - 2.3 / clocksPerSample) / clocksPerSample) * 2;  // error to feed back for delta sigma

            // Note, the 2.3 constant is because our PWM isn't perfect.  There
            // is a finite time for the DMA controller to load a new value from
            // memory, Therefore the width of each pulse we try to insert has a
            // constant added to it.  That constant is about 2.3 bytes written
            // to the serializer, or about 18 cycles.  We use delta sigma to
            // correct for this error and the pwm timing quantization error.

            // To reduce noise, rather than just rounding to the nearest clock
            // we can use, we PWM between the two nearest values.

            // delay if necessary.  We can also print debug stuff here while
            // not breaking timing.
            static int time;
            time++;

            while ((IO(DMA0_CONBLK_AD) & ~0x7F) == (int)(instrs[bufPtr].p))
            {
                usleep(sleeptime);                                      // are we anywhere in the next 4 instructions?
            }

            // Create DMA command to set clock controller to output FM signal for PWM "LOW" time.
            ((struct CB *)(instrs[bufPtr].v))->SOURCE_AD = (int)constPage.p + 2048 + intval * 4 - 4;
            bufPtr++;

            // Create DMA command to delay using serializer module for suitable time.
            ((struct CB *)(instrs[bufPtr].v))->TXFR_LEN = (int)timeErr - fracval;
            bufPtr++;

            // Create DMA command to set clock controller to output FM signal for PWM "HIGH" time.
            ((struct CB *)(instrs[bufPtr].v))->SOURCE_AD = (int)constPage.p + 2048 + intval * 4 + 4;
            bufPtr++;

            // Create DMA command for more delay.
            ((struct CB *)(instrs[bufPtr].v))->TXFR_LEN = fracval;
            bufPtr = (bufPtr + 1) % (BUFFERINSTRUCTIONS);
        }
    }
    shutdown(0);
}                                                                       // main
