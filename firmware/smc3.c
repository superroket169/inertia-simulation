#include "smc3.h"

bool smc3_feed(Smc3Parser *p, uint8_t c)
{
    switch (p->state) {
    case 0:
        if (c == '[') p->state = 1;
        break;
    case 1:
        if (c == 'A' || c == 'B') {
            p->axis = c - 'A';
            p->state = 2;
        } else {
            p->state = 0;
        }
        break;
    case 2:
        p->value = (uint16_t)c << 8;
        p->state = 3;
        break;
    case 3:
        p->value |= c;
        p->state = 4;
        break;
    case 4:
        p->state = 0;
        return c == ']';
    }
    return false;
}
