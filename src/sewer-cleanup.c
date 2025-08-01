#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "types.h"
#include "fsm.h"
#include "fsm-character.c"

#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 720

#define FIXED_TICK_RATE 16

SDL_Window *window;
SDL_Renderer *renderer;

#define SC_EVENT_KEYUP   0b0
#define SC_EVENT_KEYDOWN 0b1

#define KEY_RIGHT 0b0001
#define KEY_LEFT  0b0010
#define KEY_JUMP  0b0100

void resetPlayer(SC_Character* player, Uint64 now)
{
    player->pos.x = 200.0f;
    player->pos.y = GROUND_Y;
    player->vel.x = 0.0f;
    player->vel.y = 0.0f;
    player->acc.x = 0.0f;
    player->acc.y = 0.0f;
    player->flags = CHARACTER_FLAG_FACE_RIGHT;
    player->state = SC_CHARACTER_STAND;
}

void resetAppState(SC_AppState *scAppState, Uint64 now)
{
    scAppState->prevTick = now;
    scAppState->keysDown = 0;
    scAppState->numCharacters = 1;
    scAppState->characters = SDL_calloc(5, sizeof(SC_Character));
    resetPlayer(scAppState->characters, now);
}

SC_AppState* initAppState(Uint64 now)
{
    initCharacterFSM();

    SC_AppState *scAppState = (SC_AppState *) SDL_malloc(sizeof(SC_AppState));
    scAppState->msAccum = 0;
    resetAppState(scAppState, now);
    return scAppState;
}


void eventCharacter(SC_Character *c, SC_Event e, Uint64 now, Uint64 opts)
{
    int newMoveState = FSMsCharacter[c->state].input(c, e, now, &opts);

    if (newMoveState != SC_FSM_NO_CHANGE) {
        //SDL_Log("Leave %d, Enter %d", c->state, newMoveState);
        FSMsCharacter[c->state].exit(c, &opts);
        c->state = newMoveState;
        FSMsCharacter[c->state].enter(c, &opts);
    }
}


void tickCharacters(SC_AppState *scAppState, Uint64 delta, Uint64 now)
{
    for (int i = 0; i < scAppState->numCharacters; i++) {
        SC_Character *c = scAppState->characters + i;
        Uint64 opts = 0;

        int newState = FSMsCharacter[c->state].tick(c, delta, now, &opts);

        if (newState != SC_FSM_NO_CHANGE) {
            //SDL_Log("Leave %d, Enter %d", c->state, newState);
            FSMsCharacter[c->state].exit(c, &opts);
            c->state = newState;
            FSMsCharacter[c->state].enter(c, &opts);
        }
    }
}

void destroyAppState(SC_AppState *scAppState)
{
    SDL_free(scAppState->characters);
    scAppState->characters = NULL;
    SDL_free(scAppState);
    scAppState = NULL;

    destroyCharacterFSM();
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("Sewer Cleanup", "1.0.0", "net.faisonz.games.sewer-cleanup");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to init video: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Sewer Cleanup", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)) {
        SDL_Log("Failed to create window and/or renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    *appstate = initAppState(SDL_GetTicks());

    return SDL_APP_CONTINUE;
}

void handleInput(SC_AppState *s, Uint64 event, Uint32 keyFlag, Uint64 now)
{
    if (event == SC_EVENT_KEYDOWN) {
        if (keyFlag == KEY_RIGHT && (s->keysDown & KEY_RIGHT) == 0) {
            s->keysDown |= KEY_RIGHT;
            eventCharacter(s->characters, SC_EVENT_RUN_START, now, CHARACTER_MOVE_RIGHT);
        } else if (keyFlag == KEY_LEFT && (s->keysDown & KEY_LEFT) == 0) {
            s->keysDown |= KEY_LEFT;
            eventCharacter(s->characters, SC_EVENT_RUN_START, now, CHARACTER_MOVE_LEFT);
        } else if (keyFlag == KEY_JUMP && (s->keysDown & KEY_JUMP) == 0) {
            s->keysDown |= KEY_JUMP;
            eventCharacter(s->characters, SC_EVENT_JUMP, now, 0);
        }
    } else if (event == SC_EVENT_KEYUP) {
        if (keyFlag == KEY_RIGHT && (s->keysDown & KEY_RIGHT) > 0) {
            s->keysDown &= ~KEY_RIGHT;
            eventCharacter(s->characters, SC_EVENT_RUN_STOP, now, CHARACTER_MOVE_RIGHT);
        } else if (keyFlag == KEY_LEFT && (s->keysDown & KEY_LEFT) > 0) {
            s->keysDown &= ~KEY_LEFT;
            eventCharacter(s->characters, SC_EVENT_RUN_STOP, now, CHARACTER_MOVE_LEFT);
        } else if (keyFlag == KEY_JUMP && (s->keysDown & KEY_JUMP) > 0) {
            s->keysDown &= ~KEY_JUMP;
            eventCharacter(s->characters, SC_EVENT_JUMP_STOP, now, 0);
        }
    }
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    SC_AppState *scAppState = (SC_AppState *) appstate;
    Uint64 now = SDL_GetTicks();

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    } else if (event->type == SDL_EVENT_KEY_DOWN) {
        switch (event->key.key) {
            case SDLK_D:
                handleInput(scAppState, SC_EVENT_KEYDOWN, KEY_RIGHT, now);
                break;
            case SDLK_A:
                handleInput(scAppState, SC_EVENT_KEYDOWN, KEY_LEFT, now);
                break;
            case SDLK_SPACE:
                handleInput(scAppState, SC_EVENT_KEYDOWN, KEY_JUMP, now);
                break;
        }
    } else if (event->type == SDL_EVENT_KEY_UP) {
        switch (event->key.key) {
            case SDLK_D:
                handleInput(scAppState, SC_EVENT_KEYUP, KEY_RIGHT, now);
                break;
            case SDLK_A:
                handleInput(scAppState, SC_EVENT_KEYUP, KEY_LEFT, now);
                break;
            case SDLK_SPACE:
                handleInput(scAppState, SC_EVENT_KEYUP, KEY_JUMP, now);
                break;
        }
    }

    return SDL_APP_CONTINUE;
}

void tick(SC_AppState *scAppState, Uint64 now)
{
    // TICK UPDATE
    scAppState->msAccum += now - scAppState->prevTick;

    while (scAppState->msAccum >= FIXED_TICK_RATE) {
        tickCharacters(scAppState, FIXED_TICK_RATE, now);

        scAppState->msAccum -= FIXED_TICK_RATE;
    }

    scAppState->prevTick = now;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    SC_AppState *scAppState = (SC_AppState *) appstate;
    Uint64 now = SDL_GetTicks();

    tick(scAppState, now);

    // RENDER
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderLine(renderer, 0, GROUND_Y, WINDOW_WIDTH, GROUND_Y);
    SDL_RenderLine(renderer, 0, GROUND_Y - 40.0f, WINDOW_WIDTH, GROUND_Y - 40.0f);
    SDL_RenderLine(renderer, 0, GROUND_Y - 80.0f, WINDOW_WIDTH, GROUND_Y - 80.0f);
    SDL_RenderLine(renderer, 0, GROUND_Y - 120.0f, WINDOW_WIDTH, GROUND_Y - 120.0f);
    SDL_RenderLine(renderer, 0, GROUND_Y - 160.0f, WINDOW_WIDTH, GROUND_Y - 160.0f);

    SDL_FRect p = {
        .x = scAppState->characters->pos.x - 20.0f,
        .y = scAppState->characters->pos.y - 40.0f,
        .w = 40.0f,
        .h = 40.0f,
    };

    SDL_FRect pH = {
        .x = p.x + 13.0f,
        .y = p.y + 13.0f,
        .w = 14.0f,
        .h = 14.0f,
    };

    SC_Character *c = scAppState->characters;
    float dir = (c->flags & CHARACTER_FLAG_FACE_RIGHT) > 0 ? 1.0f : -1.0f;
    float s = 0.0f;
    switch (c->state) {
        case SC_CHARACTER_STAND:
        case SC_CHARACTER_STAND_JUMP:
        case SC_CHARACTER_STAND_FALL:
            s = 4.0f;
            break;
        case SC_CHARACTER_RUN_START:
        case SC_CHARACTER_RUN_START_JUMP:
        case SC_CHARACTER_RUN_START_FALL:
        case SC_CHARACTER_RUN_STOP:
        case SC_CHARACTER_RUN_STOP_JUMP:
        case SC_CHARACTER_RUN_STOP_FALL:
            s = 8.0f;
            break;
        case SC_CHARACTER_RUN:
        case SC_CHARACTER_RUN_JUMP:
        case SC_CHARACTER_RUN_FALL:
            s = 12.0f;
            break;
        default:
            break;
    }
    pH.x += dir * s;

    dir = 1.0f;
    s = 0.0f;
    switch (c->state) {
        case SC_CHARACTER_STAND_JUMP:
        case SC_CHARACTER_RUN_START_JUMP:
        case SC_CHARACTER_RUN_STOP_JUMP:
        case SC_CHARACTER_RUN_JUMP:
            dir = -1.0f;
            break;
        case SC_CHARACTER_STAND_FALL:
        case SC_CHARACTER_RUN_START_FALL:
        case SC_CHARACTER_RUN_STOP_FALL:
        case SC_CHARACTER_RUN_FALL:
            dir = 1.0f;
            break;
        default:
            break;
    }
    float absY = SDL_fabsf(c->vel.y);
    if (absY == 0.0f) {
        s = 0.0f;
    } else if (absY < 0.2f * PLAYER_Y_VEL_MAX) {
        s = 4.0f;
    } else if (absY < 0.6f * PLAYER_Y_VEL_MAX) {
        s = 8.0f;
    } else {
        s = 12.0f;
    }
    pH.y += dir * s;

    SDL_SetRenderDrawColor(renderer, 254, 231, 97, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &p);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &pH);

    SDL_SetRenderScale(renderer, 3.0f, 3.0f);
    SDL_SetRenderDrawColor(renderer, 255, 238, 229, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugTextFormat(renderer, 5.0f, 05.0f, "Left: %s", (scAppState->keysDown & KEY_LEFT) > 0 ? "Down" : "Up");
    SDL_RenderDebugTextFormat(renderer, 5.0f, 15.0f, "Right: %s", (scAppState->keysDown & KEY_RIGHT) > 0 ? "Down" : "Up");
    SDL_RenderDebugTextFormat(renderer, 5.0f, 25.0f, "Jump: %s", (scAppState->keysDown & KEY_JUMP) > 0 ? "Down" : "Up");
    SDL_RenderDebugTextFormat(renderer, 5.0f, 35.0f, "State: %u", scAppState->characters->state);
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);

    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    destroyAppState((SC_AppState *) appstate);
    appstate = NULL;

    SDL_DestroyRenderer(renderer);
    renderer = NULL;
    SDL_DestroyWindow(window);
    window = NULL;
}
