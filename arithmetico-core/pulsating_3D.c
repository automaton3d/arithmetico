#ifdef PULSE_C
/*
 * pulsating_3d.c
 * Visualizador 3D isométrico - USA active_visual para renderização
 */

#include <SDL3/SDL.h>
#include <math.h>
#include <stdio.h>
#include "pulsating.h"
#include "pulsating_3d.h"

/* Definição das variáveis globais */
int g_center_x = 450;
int g_center_y = 300;
float g_cos_angle = 0.866025f;
float g_sin_angle = 0.5f;
float g_scale = DEFAULT_SCALE;
int g_view_mode = VIEW_MODE_3D_FULL;

/* =========================================================
 * Alterna entre os modos de visualização
 * ========================================================= */
void toggle_view_mode(void)
{
    g_view_mode++;
    if (g_view_mode > VIEW_MODE_3D_SHELL_ONLY) {
        g_view_mode = VIEW_MODE_3D_FULL;
    }

    const char* mode_name;
    switch(g_view_mode) {
        case VIEW_MODE_3D_FULL:
            mode_name = "3D FULL (todos os voxels ativos)";
            break;
        case VIEW_MODE_2D_SLICE:
            mode_name = "2D SLICE (corte equatorial)";
            break;
        case VIEW_MODE_3D_SHELL_ONLY:
            mode_name = "3D SHELL (apenas casca)";
            break;
        default:
            mode_name = "UNKNOWN";
    }
    printf("Modo de visualização: %s\n", mode_name);
}

/* =========================================================
 * Inicializa o visualizador 3D
 * ========================================================= */
void init_3d_view(int window_width, int window_height)
{
    g_center_x = window_width / 2;
    g_center_y = window_height / 2 + 50;
    g_scale = DEFAULT_SCALE;
}

/* =========================================================
 * Projeção isométrica
 * ========================================================= */
void project_isometric(int x, int y, int z, int center_x, int center_y, int *sx, int *sy)
{
    float xf = (float)(x - MID);
    float yf = (float)(y - MID);
    float zf = (float)(z - MID);

    *sx = center_x + (int)((xf - zf) * g_cos_angle * g_scale);
    *sy = center_y + (int)((xf + zf) * g_sin_angle * g_scale - yf * g_scale);
}

/* =========================================================
 * Desenha os três eixos cartesianos (X, Y, Z)
 * ========================================================= */
void draw_axes(SDL_Renderer* ren)
{
    int sx, sy;

    SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
    for (int x = -R_MAX; x <= R_MAX; x += 2) {
        project_isometric(x + MID, MID, MID, g_center_x, g_center_y, &sx, &sy);
        SDL_RenderPoint(ren, (float)sx, (float)sy);
    }

    SDL_SetRenderDrawColor(ren, 0, 255, 0, 255);
    for (int y = -R_MAX; y <= R_MAX; y += 2) {
        project_isometric(MID, y + MID, MID, g_center_x, g_center_y, &sx, &sy);
        SDL_RenderPoint(ren, (float)sx, (float)sy);
    }

    SDL_SetRenderDrawColor(ren, 0, 0, 255, 255);
    for (int z = -R_MAX; z <= R_MAX; z += 2) {
        project_isometric(MID, MID, z + MID, g_center_x, g_center_y, &sx, &sy);
        SDL_RenderPoint(ren, (float)sx, (float)sy);
    }

    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
    project_isometric(MID, MID, MID, g_center_x, g_center_y, &sx, &sy);
    for (int i = -2; i <= 2; i++) {
        SDL_RenderPoint(ren, (float)(sx + i), (float)sy);
        SDL_RenderPoint(ren, (float)sx, (float)(sy + i));
    }
}

/* =========================================================
 * Obtém cor baseada na distância ao centro
 * ========================================================= */
static uint32_t get_color_by_radius(unsigned int r2)
{
    if (r2 == INF_R2) return 0x44000000;

    unsigned int r = 0;
    while ((r + 1) * (r + 1) <= r2) r++;

    unsigned int intensity = (r * 255) / R_MAX;
    return 0xFF000000 | (intensity << 16) | (intensity << 8) | (255 - intensity);
}

/* =========================================================
 * Modo 3D FULL: usa active_visual (casca densa)
 * ========================================================= */
void render_3d_active(SDL_Renderer* ren, unsigned int pulse_r2)
{
    (void)pulse_r2;

    int point_count = 0;

    for (int x = 0; x < L; x += 1)
    for (int y = 0; y < L; y += 1)
    for (int z = 0; z < L; z += 1) {
        Cell *c = &grid[x][y][z];

        // Usa active_visual para renderização (casca densa)
        if (c->active_visual) {
            int sx, sy;
            project_isometric(x, y, z, g_center_x, g_center_y, &sx, &sy);

            uint32_t color = get_color_by_radius(c->r2);

            SDL_SetRenderDrawColor(ren,
                (color >> 16) & 0xFF,
                (color >> 8) & 0xFF,
                color & 0xFF,
                255);

            SDL_RenderPoint(ren, (float)sx, (float)sy);
            point_count++;
        }
    }

    static int last_count = 0;
    if (point_count != last_count) {
        printf("3D FULL (visual): %d pontos (pulse_r2=%u)\n", point_count, pulse_r2);
        last_count = point_count;
    }
}

/* =========================================================
 * Modo 2D SLICE: corte equatorial usando active_visual
 * ========================================================= */
void render_2d_slice(SDL_Renderer* ren, unsigned int pulse_r2)
{
    (void)pulse_r2;

    int point_count = 0;
    int z = MID;

    for (int x = 0; x < L; x += 1)
    for (int y = 0; y < L; y += 1) {
        Cell *c = &grid[x][y][z];

        if (c->active_visual) {
            int sx, sy;
            project_isometric(x, y, z, g_center_x, g_center_y, &sx, &sy);

            uint32_t color = get_color_by_radius(c->r2);

            SDL_SetRenderDrawColor(ren,
                (color >> 16) & 0xFF,
                (color >> 8) & 0xFF,
                color & 0xFF,
                255);

            SDL_RenderPoint(ren, (float)sx, (float)sy);
            point_count++;
        }
    }

    static int last_count = 0;
    if (point_count != last_count) {
        printf("2D SLICE: %d pontos (pulse_r2=%u)\n", point_count, pulse_r2);
        last_count = point_count;
    }
}

/* =========================================================
 * Modo 3D SHELL: usa active_interact (casca fina)
 * ========================================================= */
void render_3d_shell_only(SDL_Renderer* ren, unsigned int pulse_r2)
{
    (void)pulse_r2;

    int point_count = 0;

    for (int x = 0; x < L; x += 1)
    for (int y = 0; y < L; y += 1)
    for (int z = 0; z < L; z += 1) {
        Cell *c = &grid[x][y][z];

        // Usa active_interact para mostrar a casca fina (interação)
        if (c->active_interact) {
            int sx, sy;
            project_isometric(x, y, z, g_center_x, g_center_y, &sx, &sy);

            // Cor diferente para destacar a casca de interação
            SDL_SetRenderDrawColor(ren, 255, 255, 0, 255);  // Amarelo

            SDL_RenderPoint(ren, (float)sx, (float)sy);
            point_count++;
        }
    }

    static int last_count = 0;
    if (point_count != last_count) {
        printf("3D SHELL (interação): %d pontos (pulse_r2=%u)\n", point_count, pulse_r2);
        last_count = point_count;
    }
}

/* =========================================================
 * Função principal de renderização
 * ========================================================= */
void render_3d(SDL_Renderer* ren, unsigned int pulse_r2)
{
    switch(g_view_mode) {
        case VIEW_MODE_3D_FULL:
            render_3d_active(ren, pulse_r2);
            break;
        case VIEW_MODE_2D_SLICE:
            render_2d_slice(ren, pulse_r2);
            break;
        case VIEW_MODE_3D_SHELL_ONLY:
            render_3d_shell_only(ren, pulse_r2);
            break;
        default:
            render_3d_active(ren, pulse_r2);
    }
}

/* Funções de compatibilidade */
void render_3d_fast(SDL_Renderer* ren, unsigned int pulse_r2) { render_3d(ren, pulse_r2); }
void render_3d_thick_shell(SDL_Renderer* ren, unsigned int pulse_r2) { render_3d(ren, pulse_r2); }
#endif
