// pulsating.c
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "pulsating.h"

Cell (*grid)[L][L];
Cell (*grid_next)[L][L];

const int MID = L / 2;
const int R_MAX = L / 2;

/* Constantes do pulso - calculadas em tempo de compilação */
const unsigned int PULSE_MAX_R2 = (unsigned int)((L/2) * (L/2) * 0.92);  // 60*60*0.92 = 3312
const unsigned int PULSE_SPAN = PULSE_MAX_R2 - PULSE_MIN_R2;  // 3312 - 25 = 3287
const unsigned int PULSE_PERIOD = 2 * PULSE_SPAN;  // 6574

/* =========================================================
 * Pulse update WITHOUT division or modulo
 * Usa apenas adição e subtração
 * ========================================================= */
void pulse_update(unsigned int *phase, unsigned int *pulse_r2)
{
    // Incrementa fase
    *phase += PULSE_STEP;

    // Wrap-around sem divisão (subtrai period enquanto necessário)
    while (*phase >= PULSE_PERIOD) {
        *phase -= PULSE_PERIOD;
    }

    // Calcula o r2 do pulso baseado na fase
    if (*phase < PULSE_SPAN) {
        *pulse_r2 = PULSE_MIN_R2 + *phase;
    } else {
        *pulse_r2 = PULSE_MAX_R2 - (*phase - PULSE_SPAN);
    }
}

/* =========================================================
 * Initialization
 * ========================================================= */
void init_ca(void)
{
    for (unsigned x = 0; x < L; x++)
    for (unsigned y = 0; y < L; y++)
    for (unsigned z = 0; z < L; z++) {
        grid[x][y][z].r2 = INF_R2;
        grid[x][y][z].active = 0;
    }

    grid[MID][MID][MID].r2 = 0;
    grid[MID][MID][MID].active = 0;
}

/* =========================================================
 * Wavefront propagation (Dijkstra-like, synchronous)
 * SEM divisão, SEM módulo, SEM floats
 * ========================================================= */
void update_wavefront(void)
{
    memcpy(grid_next, grid, sizeof(Cell) * L * L * L);

    for (unsigned x = 0; x < L; x++)
    for (unsigned y = 0; y < L; y++)
    for (unsigned z = 0; z < L; z++) {
        Cell *curr = &grid[x][y][z];

        if (curr->r2 == INF_R2)
            continue;

        // Distância Manhattan ao centro em cada eixo
        unsigned ax = (x > MID) ? (x - MID) : (MID - x);
        unsigned ay = (y > MID) ? (y - MID) : (MID - y);
        unsigned az = (z > MID) ? (z - MID) : (MID - z);

        // 6 vizinhos (faces)
        for (unsigned d = 0; d < 6; d++) {
            unsigned nx = x;
            unsigned ny = y;
            unsigned nz = z;
            unsigned diff = 0;

            if (d == 0) {
                if (x + 1 >= L) continue;
                nx = x + 1;
                diff = 2 * ax + 1;
            }
            else if (d == 1) {
                if (x == 0) continue;
                nx = x - 1;
                diff = 2 * ax + 1;
            }
            else if (d == 2) {
                if (y + 1 >= L) continue;
                ny = y + 1;
                diff = 2 * ay + 1;
            }
            else if (d == 3) {
                if (y == 0) continue;
                ny = y - 1;
                diff = 2 * ay + 1;
            }
            else if (d == 4) {
                if (z + 1 >= L) continue;
                nz = z + 1;
                diff = 2 * az + 1;
            }
            else {
                if (z == 0) continue;
                nz = z - 1;
                diff = 2 * az + 1;
            }

            unsigned int new_r2 = curr->r2 + diff;
            Cell *nxt = &grid_next[nx][ny][nz];

            if (new_r2 < nxt->r2) {
                nxt->r2 = new_r2;
                // r2 mudou, então active pode estar inconsistente
                nxt->active = 0;
            }
        }
    }

    // Centro sempre tem r2 = 0
    grid_next[MID][MID][MID].r2 = 0;
    grid_next[MID][MID][MID].active = 0;
}

/* =========================================================
 * Update active flags based on current pulse
 * Regra completamente local: cada célula compara seu r2 com o pulso
 * SEM divisão, SEM módulo, SEM floats
 * ========================================================= */
void update_active_flags(unsigned int pulse_r2)
{
    for (unsigned x = 0; x < L; x++)
    for (unsigned y = 0; y < L; y++)
    for (unsigned z = 0; z < L; z++) {
        Cell *c = &grid[x][y][z];

        if (c->r2 == INF_R2) {
            c->active = 0;
            continue;
        }

        // Calcula diff = |c->r2 - pulse_r2| sem usar abs (que pode usar分支)
        unsigned int diff;
        if (c->r2 > pulse_r2) {
            diff = c->r2 - pulse_r2;
        } else {
            diff = pulse_r2 - c->r2;
        }

        c->active = (diff <= ACTIVE_THICKNESS);
    }
}

/* =========================================================
 * Complete CA update
 * ========================================================= */
void update_ca(unsigned int pulse_r2)
{
    update_wavefront();
    memcpy(grid, grid_next, sizeof(Cell) * L * L * L);
    update_active_flags(pulse_r2);
}

/* =========================================================
 * Render
 * ========================================================= */
void render_frame(SDL_Renderer* ren,
                  SDL_Texture* tex,
                  uint32_t* pixels,
                  unsigned int pulse_r2)
{
    const uint32_t COLOR_BG       = 0xFF000000;
    const uint32_t COLOR_REF_CIRC = 0xFFCC0000;
    const uint32_t COLOR_REF_GRID = 0xFF880000;
    const uint32_t COLOR_SHELL    = 0xFFFFFF00;
    const uint32_t COLOR_CENTER   = 0xFF0000FF;

    const float raio_ideal = (float)R_MAX;
    const float raio_min = raio_ideal - 0.5f;
    const float raio_max = raio_ideal + 0.5f;

    for (unsigned y = 0; y < L; y++) {
        for (unsigned x = 0; x < L; x++) {
            uint32_t pix = COLOR_BG;

            int gx = x - MID;
            int gy = y - MID;

            // Grade de referência (usando valor absoluto sem divisão)
            int abs_gx = (gx < 0) ? -gx : gx;
            int abs_gy = (gy < 0) ? -gy : gy;

            if (abs_gx % GRID_SPACING == 0 && abs_gy % GRID_SPACING == 0) {
                pix = COLOR_REF_GRID;
            }

            // Círculo de referência (precisa de sqrt para visualização, não para o CA)
            // NOTA: sqrt é usado APENAS na renderização, não na lógica do CA
            float raio = sqrtf((float)(gx * gx + gy * gy));
            if (raio >= raio_min && raio <= raio_max) {
                pix = COLOR_REF_CIRC;
            }

            // Célula ativa na superfície (usa o bit, não compara r2)
            Cell *c = &grid[x][y][MID];
            if (c->active) {
                pix = COLOR_SHELL;
            }

            // Centro
            if (x == MID && y == MID) {
                pix = COLOR_CENTER;
            }

            pixels[y * L + x] = pix;
        }
    }

    SDL_UpdateTexture(tex, NULL, pixels, L * sizeof(uint32_t));
    SDL_RenderClear(ren);
    SDL_RenderTexture(ren, tex, NULL, NULL);
    SDL_RenderPresent(ren);
}

/* =========================================================
 * Main
 * ========================================================= */
int main(void)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* win = SDL_CreateWindow(
        "CA - Wavefront (Active Surface) - No Division",
        900,
        900,
        0
    );

    SDL_Renderer* ren = SDL_CreateRenderer(win, NULL);

    SDL_Texture* tex = SDL_CreateTexture(
        ren,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        L,
        L
    );

    grid = malloc(sizeof(Cell) * L * L * L);
    grid_next = malloc(sizeof(Cell) * L * L * L);
    uint32_t* pixels = malloc(L * L * sizeof(uint32_t));

    init_ca();

    // Estado do pulso (apenas no controlador, não no CA)
    unsigned int phase = 0;
    unsigned int pulse_r2 = PULSE_MIN_R2;
    unsigned int tick = 0;

    while (1) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT)
                goto cleanup;
        }

        // Atualiza o pulso SEM divisão
        pulse_update(&phase, &pulse_r2);

        // Atualiza o CA com o pulso atual
        update_ca(pulse_r2);

        // Renderiza (sqrt usado apenas aqui, na visualização)
        render_frame(ren, tex, pixels, pulse_r2);

        tick++;
        if (tick == 500) {
            analyze_metric_precision();
        }

        SDL_Delay(10);
    }

cleanup:
    free(grid);
    free(grid_next);
    free(pixels);

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);

    SDL_Quit();

    return 0;
}
