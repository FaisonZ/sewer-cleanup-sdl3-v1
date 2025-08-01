#include <SDL3/SDL.h>
#include "fsm.h"
#include "types.h"

extern SC_FSM *FSMsCharacter;
SC_FSM *FSMsCharacter;

#define PLAYER_START_VEL_X 0.05f
#define PLAYER_MAX_VEL_X 0.25f
#define PLAYER_MAX_ACC_X 0.0005f

#define CHARACTER_MOVE_RIGHT 0b01
#define CHARACTER_MOVE_LEFT  0b10

void CharacterEnterStand(void *el, Uint64 *opts)
{
    SC_Character *c = el;
    c->vel.x = 0;
}

void CharacterExitStand(void *el, Uint64 *opts)
{
}

int CharacterInputStand(void *el, SC_Event e, Uint64 now, Uint64 *opts)
{
    if (e == SC_EVENT_RUN_START) {
        return SC_CHARACTER_MOVE_RUN;
    } else if (e == SC_EVENT_RUN_STOP) {
        // If both left and right were down, update opts with
        // correct direction to move
        Uint64 optUpdate =  (*opts & CHARACTER_MOVE_RIGHT) > 0 ? CHARACTER_MOVE_LEFT : CHARACTER_MOVE_RIGHT;
        *opts &= ~*opts;
        *opts |= optUpdate;
        return SC_CHARACTER_MOVE_RUN;
    }

    return SC_FSM_NO_CHANGE;
}

int CharacterTickStand(void *el, Uint64 delta, Uint64 now)
{
    return SC_FSM_NO_CHANGE;
}

void CharacterEnterRun(void *el, Uint64 *opts)
{
    SC_Character *c = el;
    float dir = (*opts & CHARACTER_MOVE_RIGHT) > 0 ? 1.0f : -1.0f;
    SDL_Log("%f", dir);
    c->vel.x = dir * PLAYER_MAX_VEL_X;
}

void CharacterExitRun(void *el, Uint64 *opts)
{
}

int CharacterInputRun(void *el, SC_Event e, Uint64 now, Uint64 *opts)
{
    if (e == SC_EVENT_RUN_STOP) {
        return SC_CHARACTER_MOVE_STAND;
    } else if (e == SC_EVENT_RUN_START) {
        return SC_CHARACTER_MOVE_STAND;
    }
    return SC_FSM_NO_CHANGE;
}

int CharacterTickRun(void *el, Uint64 delta, Uint64 now)
{
    SC_Character *c = el;
    c->pos.x += delta * c->vel.x;
    return SC_FSM_NO_CHANGE;
}

void initCharacterFSM()
{
    FSMsCharacter = (SC_FSM *) SDL_calloc(SC_CHARACTER_MOVE_STATE_TOTAL, sizeof(SC_FSM));

    SC_FSM *stand = FSMsCharacter + SC_CHARACTER_MOVE_STAND;
    stand->enter = CharacterEnterStand;
    stand->exit = CharacterExitStand;
    stand->input = CharacterInputStand;
    stand->tick = CharacterTickStand;

    SC_FSM *run = FSMsCharacter + SC_CHARACTER_MOVE_RUN;
    run->enter = CharacterEnterRun;
    run->exit = CharacterExitRun;
    run->input = CharacterInputRun;
    run->tick = CharacterTickRun;
}

void destroyCharacterFSM()
{
    SDL_free(FSMsCharacter);
    FSMsCharacter = NULL;
}
