#ifndef SC_FSM_H
#define SC_FSM_H

#include <SDL3/SDL.h>

typedef enum SC_Event {
    SC_EVENT_RUN_START,
    SC_EVENT_RUN_STOP,
    SC_EVENT_JUMP_START,
    SC_EVENT_JUMP_STOP,
    SC_EVENT_FALL_START,
    SC_EVENT_FALL_STOP,
} SC_Event;

#define SC_FSM_NO_CHANGE -1

// Some things to explain
//
// Why do `input` and `tick` return an int?
//   - You can return a new State Enum value (e.g. `SC_CHARACTER_MOVE_RUN`) to
//     change state
//
// Why are `opts` passed in by reference?
//   - I ran into a situation where I needed to modify the `opts` in `input`
//     before returning a new State Enum value.
//
//     If you look at `CharacterInputStand`, there's a case where holding down
//     LEFT and RIGHT makes the character stand. If you release LEFT, then the
//     `SC_EVENT_RUN_STOP` event is triggered with `CHARACTER_MOVE_LEFT` set in
//     the `opts`. But we want the character to start running to the right.
//
//     So we reverse the direction in the opts and return `SC_EVENT_RUN_START`.

typedef struct SC_FSM {
    void (*enter)(void *el, Uint64 *opts);
    void (*exit)(void *el, Uint64 *opts);
    int (*input)(void *el, SC_Event e, Uint64 now, Uint64 *opts);
    int (*tick)(void *el, Uint64 delta, Uint64 now);
} SC_FSM;

#define SC_CHARACTER_MOVE_STATE_TOTAL 2

typedef enum SC_Character_Move_State {
    SC_CHARACTER_MOVE_STAND,
    SC_CHARACTER_MOVE_RUN,
} SC_Character_Move_State;

#endif
