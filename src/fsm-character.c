#include <SDL3/SDL.h>
#include "fsm.h"
#include "types.h"

extern SC_FSM *FSMsCharacter;
SC_FSM *FSMsCharacter;

#define PLAYER_X_VEL_START 0.05f
#define PLAYER_X_VEL_MAX   0.25f
#define PLAYER_X_ACC_RUN   0.00075f
#define PLAYER_X_ACC_STOP  0.00050f

// Max height = 150
// Time to peak = 375
// Time to ground = 750
#define PLAYER_Y_VEL_MAX    0.8025f
#define PLAYER_Y_VEL_START -PLAYER_Y_VEL_MAX
#define PLAYER_Y_ACC        0.00214f
// This should allow a bunny hop of 1 height
#define PLAYER_Y_VEL_STOP  -0.214f

#define GROUND_Y 600.f

#define PLAYER_JUMP_HEIGHT_MAX 120.0f

#define CHARACTER_MOVE_RIGHT 0b01
#define CHARACTER_MOVE_LEFT  0b10

void CharacterEnterStand(void *el, Uint64 *opts)
{
    SC_Character *c = el;
    c->vel.x = 0;
    c->vel.y = 0;
    c->acc.x = 0;
    c->acc.y = 0;
}

void CharacterExitStand(void *el, Uint64 *opts)
{
}

int CharacterInputStand(void *el, SC_Event e, Uint64 now, Uint64 *opts)
{
    if (e == SC_EVENT_RUN_START) {
        return SC_CHARACTER_RUN_START;
    } else if (e == SC_EVENT_RUN_STOP) {
        // If both left and right were down, update *opts with
        // correct direction to move
        Uint64 optUpdate =  (*opts & CHARACTER_MOVE_RIGHT) > 0 ? CHARACTER_MOVE_LEFT : CHARACTER_MOVE_RIGHT;
        *opts &= ~*opts;
        *opts |= optUpdate;
        return SC_CHARACTER_RUN_START;
    } else if (e == SC_EVENT_JUMP) {
        return SC_CHARACTER_STAND_JUMP;
    }

    return SC_FSM_NO_CHANGE;
}

int CharacterTickStand(void *el, Uint64 delta, Uint64 now, Uint64 *opts)
{
    return SC_FSM_NO_CHANGE;
}

void CharacterEnterRun(void *el, Uint64 *opts)
{
    SC_Character *c = el;
    float dir = (*opts & CHARACTER_MOVE_RIGHT) > 0 ? 1.0f : -1.0f;
    c->vel.x = dir * PLAYER_X_VEL_MAX;
    c->vel.y = 0;
    c->acc.x = 0;
    c->acc.y = 0;
}

void CharacterExitRun(void *el, Uint64 *opts)
{
}

int CharacterInputRun(void *el, SC_Event e, Uint64 now, Uint64 *opts)
{
    SC_Character *c = el;
    if (e == SC_EVENT_RUN_STOP) {
        return SC_CHARACTER_RUN_STOP;
    } else if (e == SC_EVENT_RUN_START) {
        *opts &= ~(c->vel.x > 0 ? CHARACTER_MOVE_LEFT : CHARACTER_MOVE_RIGHT);
        *opts |= c->vel.x > 0 ? CHARACTER_MOVE_RIGHT : CHARACTER_MOVE_LEFT;
        return SC_CHARACTER_RUN_STOP;
    } else if (e == SC_EVENT_JUMP) {
        return SC_CHARACTER_RUN_JUMP;
    }
    return SC_FSM_NO_CHANGE;
}

int CharacterTickRun(void *el, Uint64 delta, Uint64 now, Uint64 *opts)
{
    SC_Character *c = el;
    c->pos.x += delta * c->vel.x;
    return SC_FSM_NO_CHANGE;
}

void CharacterEnterRunStart(void *el, Uint64 *opts)
{
    SC_Character *c = el;
    float dir = (*opts & CHARACTER_MOVE_RIGHT) > 0 ? 1.0f : -1.0f;
    if (c->vel.x == 0) {
        c->vel.x = dir * PLAYER_X_VEL_START;
    }
    c->acc.x = dir * PLAYER_X_ACC_RUN;
    c->vel.y = 0;
    c->acc.y = 0;
}

void CharacterExitRunStart(void *el, Uint64 *opts)
{
}

int CharacterInputRunStart(void *el, SC_Event e, Uint64 now, Uint64 *opts)
{
    SC_Character *c = el;

    if (e == SC_EVENT_RUN_STOP) {
        return SC_CHARACTER_RUN_STOP;
    } else if (e == SC_EVENT_RUN_START) {
        // If both left and right were down, update *opts with
        // correct move direction
        *opts &= ~(c->vel.x > 0 ? CHARACTER_MOVE_LEFT : CHARACTER_MOVE_RIGHT);
        *opts |= c->vel.x > 0 ? CHARACTER_MOVE_RIGHT : CHARACTER_MOVE_LEFT;
        return SC_CHARACTER_RUN_STOP;
    } else if (e == SC_EVENT_JUMP) {
        *opts |= c->acc.x > 0 ? CHARACTER_MOVE_RIGHT : CHARACTER_MOVE_LEFT;
        return SC_CHARACTER_RUN_START_JUMP;
    }

    return SC_FSM_NO_CHANGE;
}

int CharacterTickRunStart(void *el, Uint64 delta, Uint64 now, Uint64 *opts)
{
    Uint64 ret = SC_FSM_NO_CHANGE;

    SC_Character *c = el;

    // The run start part
    c->vel.x += delta * c->acc.x;

    if (SDL_fabsf(c->vel.x) >= PLAYER_X_VEL_MAX) {
        c->vel.x = PLAYER_X_VEL_MAX * (c->vel.x > 0 ? 1.0f : -1.0f);
        *opts |= (c->vel.x > 0 ? CHARACTER_MOVE_RIGHT : CHARACTER_MOVE_LEFT);
        ret = SC_CHARACTER_RUN;
    }

    c->pos.x += delta * c->vel.x;
    return ret;
}

void CharacterEnterRunStop(void *el, Uint64 *opts)
{
    SC_Character *c = el;
    float dir = (*opts & CHARACTER_MOVE_RIGHT) > 0 ? -1.0f : 1.0f;
    if (dir < 0 && c->vel.x < 0) {
        dir = 1.0f;
    } else if (dir > 0 && c->vel.x > 0) {
        dir = -1.0f;
    }
    c->acc.x = dir * PLAYER_X_ACC_STOP;
    c->vel.y = 0;
    c->acc.y = 0;
}

void CharacterExitRunStop(void *el, Uint64 *opts)
{
}

int CharacterInputRunStop(void *el, SC_Event e, Uint64 now, Uint64 *opts)
{
    SC_Character *c = el;

    if (e == SC_EVENT_RUN_START) {
        return SC_CHARACTER_RUN_START;
    } else if (e == SC_EVENT_RUN_STOP) {
        // If both left and right were down, update *opts with
        // correct direction to move
        Uint64 optUpdate =  (*opts & CHARACTER_MOVE_RIGHT) > 0 ? CHARACTER_MOVE_LEFT : CHARACTER_MOVE_RIGHT;
        *opts &= ~*opts;
        *opts |= optUpdate;
        return SC_CHARACTER_RUN_START;
    } else if (e == SC_EVENT_JUMP) {
        *opts |= c->vel.x > 0 ? CHARACTER_MOVE_RIGHT : CHARACTER_MOVE_LEFT;
        return SC_CHARACTER_RUN_STOP_JUMP;
    }
    return SC_FSM_NO_CHANGE;
}

int CharacterTickRunStop(void *el, Uint64 delta, Uint64 now, Uint64 *opts)
{
    SC_Character *c = el;

    float dir = c->acc.x > 0 ? -1.0f : 1.0f;
    c->vel.x += delta * c->acc.x;

    if (SDL_fabsf(dir * PLAYER_X_VEL_MAX - c->vel.x) >= PLAYER_X_VEL_MAX) {
        return SC_CHARACTER_STAND;
    }

    c->pos.x += delta * c->vel.x;

    return SC_FSM_NO_CHANGE;
}

void CharacterEnterStandJump(void *el, Uint64 *opts)
{
    SC_Character *c = el;
    c->vel.x = 0;
    c->vel.y = 0;
    if (c->acc.y == 0) {
        c->vel.y = PLAYER_Y_VEL_START;
    }
    c->acc.y = PLAYER_Y_ACC;
}

void CharacterExitStandJump(void *el, Uint64 *opts)
{
}

int CharacterInputStandJump(void *el, SC_Event e, Uint64 now, Uint64 *opts)
{
    SC_Character *c = el;

    if (e == SC_EVENT_RUN_START) {
        return SC_CHARACTER_RUN_START_JUMP;
    } else if (e == SC_EVENT_RUN_STOP) {
        // If both left and right were down, update *opts with
        // correct direction to move
        Uint64 optUpdate =  (*opts & CHARACTER_MOVE_RIGHT) > 0 ? CHARACTER_MOVE_LEFT : CHARACTER_MOVE_RIGHT;
        *opts &= ~*opts;
        *opts |= optUpdate;
        return SC_CHARACTER_RUN_START_JUMP;
    } else if (e == SC_EVENT_JUMP_STOP) {
        if (c->vel.y < PLAYER_Y_VEL_STOP) {
            c->vel.y = PLAYER_Y_VEL_STOP;
        }
        return SC_CHARACTER_STAND_FALL;
    }

    return SC_FSM_NO_CHANGE;
}

int CharacterTickStandJump(void *el, Uint64 delta, Uint64 now, Uint64 *opts)
{
    SC_Character *c = el;
    c->vel.y += delta * c->acc.y;

    if (c->vel.y >= 0) {
        return SC_CHARACTER_STAND_FALL;
    }

    c->pos.y += delta * c->vel.y;
    return SC_FSM_NO_CHANGE;
}

void CharacterEnterStandFall(void *el, Uint64 *opts)
{
    SC_Character *c = el;
    c->vel.x = 0;
    c->acc.y = PLAYER_Y_ACC;
}

void CharacterExitStandFall(void *el, Uint64 *opts)
{
}

int CharacterInputStandFall(void *el, SC_Event e, Uint64 now, Uint64 *opts)
{
    if (e == SC_EVENT_RUN_START) {
        return SC_CHARACTER_RUN_START_FALL;
    } else if (e == SC_EVENT_RUN_STOP) {
        // If both left and right were down, update *opts with
        // correct direction to move
        Uint64 optUpdate =  (*opts & CHARACTER_MOVE_RIGHT) > 0 ? CHARACTER_MOVE_LEFT : CHARACTER_MOVE_RIGHT;
        *opts &= ~*opts;
        *opts |= optUpdate;
        return SC_CHARACTER_RUN_START_FALL;
    }
    return SC_FSM_NO_CHANGE;
}

int CharacterTickStandFall(void *el, Uint64 delta, Uint64 now, Uint64 *opts)
{
    SC_Character *c = el;
    c->vel.y += delta * c->acc.y;

    if (c->vel.y >= PLAYER_Y_VEL_MAX) {
        c->vel.y = PLAYER_Y_VEL_MAX;
    }

    c->pos.y += delta * c->vel.y;

    if (c->pos.y >= GROUND_Y) {
        c->pos.y = GROUND_Y;
        return SC_CHARACTER_STAND;
    }
    return SC_FSM_NO_CHANGE;
}

void CharacterEnterRunStartJump(void *el, Uint64 *opts)
{
    SC_Character *c = el;
    float dir = (*opts & CHARACTER_MOVE_RIGHT) > 0 ? 1.0f : -1.0f;

    if (c->vel.x == 0) {
        c->vel.x = dir * PLAYER_X_VEL_START;
    }
    c->acc.x = dir * PLAYER_X_ACC_RUN;

    if (c->vel.y == 0) {
        c->vel.y = PLAYER_Y_VEL_START;
    }

    c->acc.y = PLAYER_Y_ACC;
}

void CharacterExitRunStartJump(void *el, Uint64 *opts)
{
}

int CharacterInputRunStartJump(void *el, SC_Event e, Uint64 now, Uint64 *opts)
{
    SC_Character *c = el;

    if (e == SC_EVENT_RUN_STOP) {
        return SC_CHARACTER_RUN_STOP_JUMP;
    } else if (e == SC_EVENT_RUN_START) {
        // If both left and right were down, update *opts with
        // correct move direction
        *opts &= ~(c->vel.x > 0 ? CHARACTER_MOVE_LEFT : CHARACTER_MOVE_RIGHT);
        *opts |= c->vel.x > 0 ? CHARACTER_MOVE_RIGHT : CHARACTER_MOVE_LEFT;
        return SC_CHARACTER_RUN_STOP_JUMP;
    } else if (e == SC_EVENT_JUMP_STOP) {
        if (c->vel.y < PLAYER_Y_VEL_STOP) {
            c->vel.y = PLAYER_Y_VEL_STOP;
        }
        *opts |= c->acc.x > 0 ? CHARACTER_MOVE_RIGHT : CHARACTER_MOVE_LEFT;
        return SC_CHARACTER_RUN_START_FALL;
    }

    return SC_FSM_NO_CHANGE;
}

int CharacterTickRunStartJump(void *el, Uint64 delta, Uint64 now, Uint64 *opts)
{
    SC_Character *c = el;

    // The run start part
    c->vel.x += delta * c->acc.x;

    if (SDL_fabsf(c->vel.x) >= PLAYER_X_VEL_MAX) {
        c->vel.x = PLAYER_X_VEL_MAX * (c->vel.x > 0 ? 1.0f : -1.0f);
        *opts |= (c->vel.x > 0 ? CHARACTER_MOVE_RIGHT : CHARACTER_MOVE_LEFT);
        return SC_CHARACTER_RUN_JUMP;
    }

    c->pos.x += delta * c->vel.x;

    // The jump part
    c->vel.y += delta * c->acc.y;

    if (c->vel.y >= 0) {
        *opts |= c->acc.x > 0 ? CHARACTER_MOVE_RIGHT : CHARACTER_MOVE_LEFT;
        return SC_CHARACTER_RUN_START_FALL;
    }

    c->pos.y += delta * c->vel.y;
    return SC_FSM_NO_CHANGE;
}

void CharacterEnterRunStartFall(void *el, Uint64 *opts)
{
    SC_Character *c = el;
    float dir = (*opts & CHARACTER_MOVE_RIGHT) > 0 ? 1.0f : -1.0f;
    if (c->vel.x == 0) {
        c->vel.x = dir * PLAYER_X_VEL_START;
    }
    c->acc.x = dir * PLAYER_X_ACC_RUN;

    c->acc.y = PLAYER_Y_ACC;
}

void CharacterExitRunStartFall(void *el, Uint64 *opts)
{
}

int CharacterInputRunStartFall(void *el, SC_Event e, Uint64 now, Uint64 *opts)
{
    SC_Character *c = el;

    if (e == SC_EVENT_RUN_STOP) {
        return SC_CHARACTER_RUN_STOP_FALL;
    } else if (e == SC_EVENT_RUN_START) {
        // If both left and right were down, update *opts with
        // correct move direction
        *opts &= ~(c->vel.x > 0 ? CHARACTER_MOVE_LEFT : CHARACTER_MOVE_RIGHT);
        *opts |= c->vel.x > 0 ? CHARACTER_MOVE_RIGHT : CHARACTER_MOVE_LEFT;
        return SC_CHARACTER_RUN_STOP_FALL;
    }

    return SC_FSM_NO_CHANGE;
}

int CharacterTickRunStartFall(void *el, Uint64 delta, Uint64 now, Uint64 *opts)
{
    SC_Character *c = el;

    // The run start part
    c->vel.x += delta * c->acc.x;

    if (SDL_fabsf(c->vel.x) >= PLAYER_X_VEL_MAX) {
        c->vel.x = PLAYER_X_VEL_MAX * (c->vel.x > 0 ? 1.0f : -1.0f);
        *opts |= (c->vel.x > 0 ? CHARACTER_MOVE_RIGHT : CHARACTER_MOVE_LEFT);
        return SC_CHARACTER_RUN_FALL;
    }

    c->pos.x += delta * c->vel.x;

    // Fall part
    c->vel.y += delta * c->acc.y;

    if (c->vel.y >= PLAYER_Y_VEL_MAX) {
        c->vel.y = PLAYER_Y_VEL_MAX;
    }

    c->pos.y += delta * c->vel.y;

    if (c->pos.y >= GROUND_Y) {
        c->pos.y = GROUND_Y;
        *opts |= (c->acc.x > 0 ? CHARACTER_MOVE_RIGHT : CHARACTER_MOVE_LEFT);
        return SC_CHARACTER_RUN_START;
    }

    return SC_FSM_NO_CHANGE;
}

void CharacterEnterRunJump(void *el, Uint64 *opts)
{
    SC_Character *c = el;
    if (c->vel.y == 0) {
        c->vel.y = PLAYER_Y_VEL_START;
    }
    c->acc.y = PLAYER_Y_ACC;
}

void CharacterExitRunJump(void *el, Uint64 *opts)
{
}

int CharacterInputRunJump(void *el, SC_Event e, Uint64 now, Uint64 *opts)
{
    SC_Character *c = el;

    if (e == SC_EVENT_RUN_STOP) {
        return SC_CHARACTER_RUN_STOP_JUMP;
    } else if (e == SC_EVENT_RUN_START) {
        *opts &= ~(c->vel.x > 0 ? CHARACTER_MOVE_LEFT : CHARACTER_MOVE_RIGHT);
        *opts |= c->vel.x > 0 ? CHARACTER_MOVE_RIGHT : CHARACTER_MOVE_LEFT;
        return SC_CHARACTER_RUN_STOP_JUMP;
    } else if (e == SC_EVENT_JUMP_STOP) {
        if (c->vel.y < PLAYER_Y_VEL_STOP) {
            c->vel.y = PLAYER_Y_VEL_STOP;
        }
        return SC_CHARACTER_RUN_FALL;
    }

    return SC_FSM_NO_CHANGE;
}

int CharacterTickRunJump(void *el, Uint64 delta, Uint64 now, Uint64 *opts)
{
    SC_Character *c = el;
    c->pos.x += delta * c->vel.x;

    c->vel.y += delta * c->acc.y;

    if (c->vel.y >= 0) {
        return SC_CHARACTER_RUN_FALL;
    }

    c->pos.y += delta * c->vel.y;
    return SC_FSM_NO_CHANGE;
}

void CharacterEnterRunFall(void *el, Uint64 *opts)
{
    SC_Character *c = el;
    c->acc.y = PLAYER_Y_ACC;
}

void CharacterExitRunFall(void *el, Uint64 *opts)
{
}

int CharacterInputRunFall(void *el, SC_Event e, Uint64 now, Uint64 *opts)
{
    SC_Character *c = el;

    if (e == SC_EVENT_RUN_STOP) {
        return SC_CHARACTER_RUN_STOP_FALL;
    } else if (e == SC_EVENT_RUN_START) {
        *opts &= ~(c->vel.x > 0 ? CHARACTER_MOVE_LEFT : CHARACTER_MOVE_RIGHT);
        *opts |= c->vel.x > 0 ? CHARACTER_MOVE_RIGHT : CHARACTER_MOVE_LEFT;
        return SC_CHARACTER_RUN_STOP_FALL;
    }

    return SC_FSM_NO_CHANGE;
}

int CharacterTickRunFall(void *el, Uint64 delta, Uint64 now, Uint64 *opts)
{
    SC_Character *c = el;
    c->pos.x += delta * c->vel.x;

    c->vel.y += delta * c->acc.y;

    if (c->vel.y >= PLAYER_Y_VEL_MAX) {
        c->vel.y = PLAYER_Y_VEL_MAX;
    }

    c->pos.y += delta * c->vel.y;

    if (c->pos.y >= GROUND_Y) {
        c->pos.y = GROUND_Y;
        *opts |= (c->vel.x > 0 ? CHARACTER_MOVE_RIGHT : CHARACTER_MOVE_LEFT);
        return SC_CHARACTER_RUN;
    }

    return SC_FSM_NO_CHANGE;
}

void CharacterEnterRunStopJump(void *el, Uint64 *opts)
{
    SC_Character *c = el;
    float dir = (*opts & CHARACTER_MOVE_RIGHT) > 0 ? -1.0f : 1.0f;

    if (dir < 0 && c->vel.x < 0) {
        dir = 1.0f;
    } else if (dir > 0 && c->vel.x > 0) {
        dir = -1.0f;
    }

    c->acc.x = dir * PLAYER_X_ACC_STOP;

    if (c->vel.y == 0) {
        c->vel.y = PLAYER_Y_VEL_START;
    }

    c->acc.y = PLAYER_Y_ACC;
}

void CharacterExitRunStopJump(void *el, Uint64 *opts)
{
}

int CharacterInputRunStopJump(void *el, SC_Event e, Uint64 now, Uint64 *opts)
{
    SC_Character *c = el;

    if (e == SC_EVENT_RUN_START) {
        return SC_CHARACTER_RUN_START_JUMP;
    } else if (e == SC_EVENT_RUN_STOP) {
        // If both left and right were down, update *opts with
        // correct direction to move
        Uint64 optUpdate =  (*opts & CHARACTER_MOVE_RIGHT) > 0 ? CHARACTER_MOVE_LEFT : CHARACTER_MOVE_RIGHT;
        *opts &= ~*opts;
        *opts |= optUpdate;
        return SC_CHARACTER_RUN_START_JUMP;
    } else if (e == SC_EVENT_JUMP_STOP) {
        if (c->vel.y < PLAYER_Y_VEL_STOP) {
            c->vel.y = PLAYER_Y_VEL_STOP;
        }
        *opts |= c->acc.x < 0 ? CHARACTER_MOVE_RIGHT : CHARACTER_MOVE_LEFT;
        return SC_CHARACTER_RUN_STOP_FALL;
    }

    return SC_FSM_NO_CHANGE;
}

int CharacterTickRunStopJump(void *el, Uint64 delta, Uint64 now, Uint64 *opts)
{
    SC_Character *c = el;

    // The run stop part
    float dir = c->acc.x > 0 ? -1.0f : 1.0f;
    c->vel.x += delta * c->acc.x;

    if (SDL_fabsf(dir * PLAYER_X_VEL_MAX - c->vel.x) >= PLAYER_X_VEL_MAX) {
        return SC_CHARACTER_STAND_JUMP;
    }

    c->pos.x += delta * c->vel.x;

    // The jump part
    c->vel.y += delta * c->acc.y;

    if (c->vel.y >= 0) {
        *opts |= (c->acc.x > 0 ? CHARACTER_MOVE_RIGHT : CHARACTER_MOVE_LEFT);
        return SC_CHARACTER_RUN_STOP_FALL;
    }

    c->pos.y += delta * c->vel.y;
    return SC_FSM_NO_CHANGE;
}

void CharacterEnterRunStopFall(void *el, Uint64 *opts)
{
    SC_Character *c = el;
    float dir = (*opts & CHARACTER_MOVE_RIGHT) > 0 ? -1.0f : 1.0f;

    if (dir < 0 && c->vel.x < 0) {
        dir = 1.0f;
    } else if (dir > 0 && c->vel.x > 0) {
        dir = -1.0f;
    }

    c->acc.x = dir * PLAYER_X_ACC_STOP;
    c->acc.y = PLAYER_Y_ACC;
}

void CharacterExitRunStopFall(void *el, Uint64 *opts)
{
}

int CharacterInputRunStopFall(void *el, SC_Event e, Uint64 now, Uint64 *opts)
{
    if (e == SC_EVENT_RUN_START) {
        return SC_CHARACTER_RUN_START_FALL;
    } else if (e == SC_EVENT_RUN_STOP) {
        // If both left and right were down, update *opts with
        // correct direction to move
        Uint64 optUpdate =  (*opts & CHARACTER_MOVE_RIGHT) > 0 ? CHARACTER_MOVE_LEFT : CHARACTER_MOVE_RIGHT;
        *opts &= ~*opts;
        *opts |= optUpdate;
        return SC_CHARACTER_RUN_START_FALL;
    }

    return SC_FSM_NO_CHANGE;
}

int CharacterTickRunStopFall(void *el, Uint64 delta, Uint64 now, Uint64 *opts)
{
    SC_Character *c = el;

    // The run stop part
    float dir = c->acc.x > 0 ? -1.0f : 1.0f;
    c->vel.x += delta * c->acc.x;

    if (SDL_fabsf(dir * PLAYER_X_VEL_MAX - c->vel.x) >= PLAYER_X_VEL_MAX) {
        return SC_CHARACTER_STAND_FALL;
    }

    c->pos.x += delta * c->vel.x;

    // Fall part
    c->vel.y += delta * c->acc.y;

    if (c->vel.y >= PLAYER_Y_VEL_MAX) {
        c->vel.y = PLAYER_Y_VEL_MAX;
    }

    c->pos.y += delta * c->vel.y;

    if (c->pos.y >= GROUND_Y) {
        c->pos.y = GROUND_Y;
        *opts |= (c->vel.x > 0 ? CHARACTER_MOVE_RIGHT : CHARACTER_MOVE_LEFT);
        return SC_CHARACTER_RUN_STOP;
    }

    return SC_FSM_NO_CHANGE;
}

void initCharacterFSM()
{
    FSMsCharacter = (SC_FSM *) SDL_calloc(SC_CHARACTER_MOVE_STATE_TOTAL, sizeof(SC_FSM));

    SC_FSM *stand = FSMsCharacter + SC_CHARACTER_STAND;
    stand->enter = CharacterEnterStand;
    stand->exit = CharacterExitStand;
    stand->input = CharacterInputStand;
    stand->tick = CharacterTickStand;

    SC_FSM *run = FSMsCharacter + SC_CHARACTER_RUN;
    run->enter = CharacterEnterRun;
    run->exit = CharacterExitRun;
    run->input = CharacterInputRun;
    run->tick = CharacterTickRun;

    SC_FSM *runStart = FSMsCharacter + SC_CHARACTER_RUN_START;
    runStart->enter = CharacterEnterRunStart;
    runStart->exit = CharacterExitRunStart;
    runStart->input = CharacterInputRunStart;
    runStart->tick = CharacterTickRunStart;

    SC_FSM *runStop = FSMsCharacter + SC_CHARACTER_RUN_STOP;
    runStop->enter = CharacterEnterRunStop;
    runStop->exit = CharacterExitRunStop;
    runStop->input = CharacterInputRunStop;
    runStop->tick = CharacterTickRunStop;

    SC_FSM *standJump = FSMsCharacter + SC_CHARACTER_STAND_JUMP;
    standJump->enter = CharacterEnterStandJump;
    standJump->exit = CharacterExitStandJump;
    standJump->input = CharacterInputStandJump;
    standJump->tick = CharacterTickStandJump;

    SC_FSM *standFall = FSMsCharacter + SC_CHARACTER_STAND_FALL;
    standFall->enter = CharacterEnterStandFall;
    standFall->exit = CharacterExitStandFall;
    standFall->input = CharacterInputStandFall;
    standFall->tick = CharacterTickStandFall;

    SC_FSM *runStartJump = FSMsCharacter + SC_CHARACTER_RUN_START_JUMP;
    runStartJump->enter = CharacterEnterRunStartJump;
    runStartJump->exit = CharacterExitRunStartJump;
    runStartJump->input = CharacterInputRunStartJump;
    runStartJump->tick = CharacterTickRunStartJump;

    SC_FSM *runStartFall = FSMsCharacter + SC_CHARACTER_RUN_START_FALL;
    runStartFall->enter = CharacterEnterRunStartFall;
    runStartFall->exit = CharacterExitRunStartFall;
    runStartFall->input = CharacterInputRunStartFall;
    runStartFall->tick = CharacterTickRunStartFall;

    SC_FSM *runJump = FSMsCharacter + SC_CHARACTER_RUN_JUMP;
    runJump->enter = CharacterEnterRunJump;
    runJump->exit = CharacterExitRunJump;
    runJump->input = CharacterInputRunJump;
    runJump->tick = CharacterTickRunJump;

    SC_FSM *runFall = FSMsCharacter + SC_CHARACTER_RUN_FALL;
    runFall->enter = CharacterEnterRunFall;
    runFall->exit = CharacterExitRunFall;
    runFall->input = CharacterInputRunFall;
    runFall->tick = CharacterTickRunFall;

    SC_FSM *runStopJump = FSMsCharacter + SC_CHARACTER_RUN_STOP_JUMP;
    runStopJump->enter = CharacterEnterRunStopJump;
    runStopJump->exit = CharacterExitRunStopJump;
    runStopJump->input = CharacterInputRunStopJump;
    runStopJump->tick = CharacterTickRunStopJump;

    SC_FSM *runStopFall = FSMsCharacter + SC_CHARACTER_RUN_STOP_FALL;
    runStopFall->enter = CharacterEnterRunStopFall;
    runStopFall->exit = CharacterExitRunStopFall;
    runStopFall->input = CharacterInputRunStopFall;
    runStopFall->tick = CharacterTickRunStopFall;
}

void destroyCharacterFSM()
{
    SDL_free(FSMsCharacter);
    FSMsCharacter = NULL;
}
