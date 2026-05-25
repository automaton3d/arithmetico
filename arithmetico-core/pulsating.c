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
 * Deterministic pulse without global state
 * ========================================================= */
unsigned int pulse_from_time(unsigned int t)
{
    const unsigned int min_r2 = 25;

    const unsigned int max_r2 =
        (unsigned int)(R_MAX * R_MAX * 0.92);

    const unsigned int step = 7;

    unsigned int span = max_r2 - min_r2;

    if (span == 0)
        return min_r2;

    unsigned int period = 2 * span;

    unsigned int phase = (t * step) % period;

    if (phase < span)
        return min_r2 + phase;
    else
        return max_r2 - (phase - span);
}

/* =========================================================
 * Initialization
 * ========================================================= */
void init_ca(void)
{
    for (unsigned x = 0; x < L; x++)
    for (unsigned y = 0; y < L; y++)
    for (unsigned z = 0; z < L; z++) {

        Cell *c = &grid[x][y][z];

        c->r2 = INF_R2;
    }

    grid[MID][MID][MID].r2 = 0;
}

/* =========================================================
 * Render
 * ========================================================= */
void render_frame(SDL_Renderer* ren,
                  SDL_Texture* tex,
                  uint32_t* pixels,
                  unsigned int pulse_threshold_r2)
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

            if (abs(gx) % GRID_SPACING == 0 &&
                abs(gy) % GRID_SPACING == 0)
            {
                pix = COLOR_REF_GRID;
            }

            float raio =
                sqrtf((float)(gx * gx + gy * gy));

            if (raio >= raio_min &&
                raio <= raio_max)
            {
                pix = COLOR_REF_CIRC;
            }

            Cell *c = &grid[x][y][MID];

            if (c->r2 != INF_R2 &&
                c->r2 == pulse_threshold_r2)
            {
                pix = COLOR_SHELL;
            }

            if (x == MID && y == MID)
                pix = COLOR_CENTER;

            pixels[y * L + x] = pix;
        }
    }

    SDL_UpdateTexture(
        tex,
        NULL,
        pixels,
        L * sizeof(uint32_t)
    );

    SDL_RenderClear(ren);
    SDL_RenderTexture(ren, tex, NULL, NULL);
    SDL_RenderPresent(ren);
}

/* =========================================================
 * Wavefront
 * ========================================================= */
void update_wavefront(void)
{
    memcpy(
        grid_next,
        grid,
        sizeof(Cell) * L * L * L
    );

    for (unsigned x = 0; x < L; x++)
    for (unsigned y = 0; y < L; y++)
    for (unsigned z = 0; z < L; z++) {

        Cell *curr = &grid[x][y][z];

        if (curr->r2 == INF_R2)
            continue;

        unsigned ax =
            (x > MID)
            ? (x - MID)
            : (MID - x);

        unsigned ay =
            (y > MID)
            ? (y - MID)
            : (MID - y);

        unsigned az =
            (z > MID)
            ? (z - MID)
            : (MID - z);

        for (unsigned d = 0; d < 6; d++) {

            unsigned nx = x;
            unsigned ny = y;
            unsigned nz = z;

            unsigned diff = 0;

            if (d == 0) {

                if (x + 1 >= L)
                    continue;

                nx = x + 1;

                diff = 2 * ax + 1;
            }
            else if (d == 1) {

                if (x == 0)
                    continue;

                nx = x - 1;

                diff = 2 * ax + 1;
            }
            else if (d == 2) {

                if (y + 1 >= L)
                    continue;

                ny = y + 1;

                diff = 2 * ay + 1;
            }
            else if (d == 3) {

                if (y == 0)
                    continue;

                ny = y - 1;

                diff = 2 * ay + 1;
            }
            else if (d == 4) {

                if (z + 1 >= L)
                    continue;

                nz = z + 1;

                diff = 2 * az + 1;
            }
            else {

                if (z == 0)
                    continue;

                nz = z - 1;

                diff = 2 * az + 1;
            }

            unsigned int new_r2 =
                curr->r2 + diff;

            Cell *nxt =
                &grid_next[nx][ny][nz];

            if (new_r2 < nxt->r2) {

                nxt->r2 = new_r2;
            }
        }
    }
}

/* =========================================================
 * Update
 * ========================================================= */
void update_ca(void)
{
    update_wavefront();

    grid_next[MID][MID][MID].r2 = 0;

    memcpy(
        grid,
        grid_next,
        sizeof(Cell) * L * L * L
    );
}

/* =========================================================
 * Main
 * ========================================================= */
int main(void)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* win =
        SDL_CreateWindow(
            "CA - Wavefront (Ultra Compact)",
            900,
            900,
            0
        );

    SDL_Renderer* ren =
        SDL_CreateRenderer(win, NULL);

    SDL_Texture* tex =
        SDL_CreateTexture(
            ren,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            L,
            L
        );

    grid =
        malloc(sizeof(Cell) * L * L * L);

    grid_next =
        malloc(sizeof(Cell) * L * L * L);

    uint32_t* pixels =
        malloc(L * L * sizeof(uint32_t));

    init_ca();

    unsigned int tick = 0;

    while (1) {

        SDL_Event ev;

        while (SDL_PollEvent(&ev)) {

            if (ev.type == SDL_EVENT_QUIT)
                goto cleanup;
        }

        update_ca();

        render_frame(
            ren,
            tex,
            pixels,
            pulse_from_time(tick)
        );

        tick++;
        if (tick == 500)
            analyze_metric_precision();

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
