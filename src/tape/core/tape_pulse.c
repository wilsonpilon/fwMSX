#include "tape_pulse.h"

void tape_cursor_set(TapePulseCursor *c, const uint32_t *durations, uint32_t count) {
    c->durations = durations;
    c->count = count;
    tape_cursor_rewind(c);
}

void tape_cursor_rewind(TapePulseCursor *c) {
    c->index = 0;
    c->level = 0;
    c->finished = (c->count == 0) ? 1 : 0;
    c->remaining = (c->count > 0) ? (long)c->durations[0] : 0;
}

void tape_cursor_seek(TapePulseCursor *c, uint32_t index) {
    if (index > c->count) index = c->count;
    c->index = index;
    c->level = (int)(index & 1u);
    if (index >= c->count) {
        c->finished = 1;
        c->remaining = 0;
    } else {
        c->finished = 0;
        c->remaining = (long)c->durations[index];
    }
}

int tape_cursor_advance(TapePulseCursor *c, long cycles) {
    while (cycles > 0 && !c->finished) {
        if (cycles < c->remaining) {
            c->remaining -= cycles;
            cycles = 0;
        } else {
            cycles -= c->remaining;
            c->index++;
            c->level ^= 1;
            if (c->index >= c->count) {
                c->finished = 1;
                c->remaining = 0;
            } else {
                c->remaining = (long)c->durations[c->index];
            }
        }
    }
    return c->level;
}
