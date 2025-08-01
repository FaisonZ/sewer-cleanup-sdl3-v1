#ifndef SC_TYPES_H
#define SC_TYPES_H
#include "fsm.h"
#include <SDL3/SDL.h>

#define CHARACTER_FLAG_FACE_RIGHT 0b01
#define CHARACTER_FLAG_FACE_LEFT  0b10

typedef struct SC_Character {
    SDL_FPoint pos;
    SDL_FPoint vel;
    SDL_FPoint acc;
    SC_Character_State state;
    Uint8 flags;
} SC_Character;

typedef struct SC_AppState {
    SC_Character *characters;
    Uint64 prevTick;
    Uint64 msAccum;
    Uint32 keysDown;
    Uint8 numCharacters;
} SC_AppState;

#endif
