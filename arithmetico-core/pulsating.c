#ifdef PULSE
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

/* =========================================================
 * Pulse update - Triangular completo
 * Ativa células tanto na subida quanto na descida
 * SEM divisão, SEM módulo
 * ========================================================= */
void pulse_update_triangular(unsigned int *phase, unsigned int *pulse_r2, unsigned int *direction)
{
    if (*direction == 0) {
        // Subindo
        if (*phase < PULSE_MAX_R2) {
            (*phase)++;
        } else {
            *direction = 1;
            if (*phase > 0) (*phase)--;
        }
    } else {
        // Descendo
        if (*phase > PULSE_MIN_R2) {
            (*phase)--;
        } else {
            *direction = 0;
            if (*phase < PULSE_MAX_R2) (*phase)++;
        }
    }

    *pulse_r2 = *phase;
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
        grid[x][y][z].active_interact = 0;
        grid[x][y][z].active_visual = 0;
    }

    grid[MID][MID][MID].r2 = 0;
    grid[MID][MID][MID].active_interact = 0;
    grid[MID][MID][MID].active_visual = 0;
}

/* =========================================================
 * Wavefront propagation (Dijkstra-like, synchronous)
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

        unsigned ax = (x > MID) ? (x - MID) : (MID - x);
        unsigned ay = (y > MID) ? (y - MID) : (MID - y);
        unsigned az = (z > MID) ? (z - MID) : (MID - z);

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
                // r2 mudou, desliga os bits
                nxt->active_interact = 0;
                nxt->active_visual = 0;
            }
        }
    }

    grid_next[MID][MID][MID].r2 = 0;
    grid_next[MID][MID][MID].active_interact = 0;
    grid_next[MID][MID][MID].active_visual = 0;
}

/* =========================================================
 * Update active flags com DOIS BITS:
 * - active_interact: casca fina (thickness = 1) para interação precisa
 * - active_visual:   casca densa (thickness = 2R+1) para visualização
 * ========================================================= */
void update_active_flags(unsigned int pulse_r2)
{
    // Calcula R = floor(sqrt(pulse_r2)) sem divisão
    unsigned int R = 0;
    while ((R + 1) * (R + 1) <= pulse_r2) {
        R++;
    }

    // Espessura para visualização: casca densa (~4πR² pontos)
    unsigned int visual_thickness = 2 * R + 1;
    if (visual_thickness < 5) visual_thickness = 5;
    if (visual_thickness > 200) visual_thickness = 200;

    // Espessura para interação: casca fina (ativação precisa)
    const unsigned int interact_thickness = 1;

    for (unsigned x = 0; x < L; x++)
    for (unsigned y = 0; y < L; y++)
    for (unsigned z = 0; z < L; z++) {
        Cell *c = &grid[x][y][z];

        if (c->r2 == INF_R2) {
            c->active_interact = 0;
            c->active_visual = 0;
            continue;
        }

        unsigned int diff = (c->r2 > pulse_r2) ? (c->r2 - pulse_r2) : (pulse_r2 - c->r2);

        // Bit de interação: casca fina (preciso)
        c->active_interact = (diff <= interact_thickness);

        // Bit de visualização: casca densa (para mostrar muitos pontos)
        c->active_visual = (diff <= visual_thickness);
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
 * Render (corte equatorial) - usa active_visual
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

    (void)pulse_r2;

    for (unsigned y = 0; y < L; y++) {
        for (unsigned x = 0; x < L; x++) {
            uint32_t pix = COLOR_BG;

            int gx = x - MID;
            int gy = y - MID;

            int abs_gx = (gx < 0) ? -gx : gx;
            int abs_gy = (gy < 0) ? -gy : gy;

            if (abs_gx % GRID_SPACING == 0 && abs_gy % GRID_SPACING == 0) {
                pix = COLOR_REF_GRID;
            }

            float raio = sqrtf((float)(gx * gx + gy * gy));
            if (raio >= raio_min && raio <= raio_max) {
                pix = COLOR_REF_CIRC;
            }

            Cell *c = &grid[x][y][MID];
            // Usa active_visual para renderização
            if (c->active_visual) {
                pix = COLOR_SHELL;
            }

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
int main__(void)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* win = SDL_CreateWindow(
        "CA - Wavefront (Two Bits: Interact + Visual)",
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

    unsigned int phase = PULSE_MIN_R2;
    unsigned int direction = 0;
    unsigned int pulse_r2;
    unsigned int tick = 0;

    while (1) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT)
                goto cleanup;
        }

        pulse_update_triangular(&phase, &pulse_r2, &direction);
        update_ca(pulse_r2);
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
#endif
