#ifndef SC_TYPES_H
#define SC_TYPES_H
#include "fsm.h"
#include <SDL3/SDL.h>

typedef struct SC_Character {
    SDL_FPoint pos;
    SDL_FPoint vel;
    SDL_FPoint acc;
    SC_Character_State state;
} SC_Character;

typedef struct SC_AppState {
    SC_Character *characters;
    Uint64 prevTick;
    Uint64 msAccum;
    Uint32 keysDown;
    Uint8 numCharacters;
} SC_AppState;

#endif
