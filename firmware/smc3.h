#ifndef SMC3_H
#define SMC3_H

#include <stdbool.h>
#include <stdint.h>

// SMC3 frames from FlyPT Mover.
//
// Frame: '[' axis hi lo ']'  with axis 'A' or 'B', value big-endian.
// Mover template: [A<R0.A1.B>][B<R0.A2.B>]
//
// No AVR dependencies, so it also builds on the host for tests.

typedef struct {
    uint8_t state;
    uint8_t axis;   // 0 = 'A', 1 = 'B'; valid after smc3_feed returns true
    uint16_t value; // valid after smc3_feed returns true
} Smc3Parser;

// Feeds one byte. Returns true when it completes a frame.
bool smc3_feed(Smc3Parser *p, uint8_t c);

#endif
