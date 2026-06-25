#ifdef MAIN_3D
/*
 * main_3d.c
 * Versão com visualizador 3D isométrico - Com toggle de modo
 */

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pulsating.h"
#include "pulsating_3d.h"

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 600

/* Referência às variáveis globais do visualizador */
extern int g_center_x;
extern int g_center_y;
extern float g_cos_angle;
extern float g_sin_angle;
extern float g_scale;
extern int g_view_mode;

int main(void)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* win = SDL_CreateWindow(
        "CA - Wavefront (Isometric 3D) - Press M to change view mode",
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0
    );

    SDL_Renderer* ren = SDL_CreateRenderer(win, NULL);

    // Aloca grids
    grid = malloc(sizeof(Cell) * L * L * L);
    grid_next = malloc(sizeof(Cell) * L * L * L);

    init_ca();
    init_3d_view(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Estado do pulso
    unsigned int phase = PULSE_MIN_R2;
    unsigned int direction = 0;
    unsigned int pulse_r2;
    unsigned int tick = 0;

    // Controle de rotação
    int rotate = 0;
    float angle = 0.0f;

    printf("Controles:\n");
    printf("  Setas: rotacionar\n");
    printf("  + / - : zoom\n");
    printf("  M     : alternar modo de visualização\n");
    printf("  Espaço: resetar rotação\n\n");

    while (1) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT)
                goto cleanup;

            if (ev.type == SDL_EVENT_KEY_DOWN) {
                switch(ev.key.key) {
                    case SDLK_LEFT: rotate = -1; break;
                    case SDLK_RIGHT: rotate = 1; break;
                    case SDLK_UP:
                        g_scale += 0.2f;
                        if (g_scale > 10.0f) g_scale = 10.0f;
                        break;
                    case SDLK_DOWN:
                        g_scale -= 0.2f;
                        if (g_scale < 1.0f) g_scale = 1.0f;
                        break;
                    case SDLK_M:  // SDL3 usa SDLK_M (maiúsculo)
                        toggle_view_mode();
                        break;
                    case SDLK_SPACE:
                        rotate = 0;
                        angle = 0.0f;
                        g_cos_angle = cosf(angle);
                        g_sin_angle = sinf(angle);
                        break;
                    default:
                        break;
                }
            }
        }

        // Rotação suave
        if (rotate != 0) {
            angle += rotate * 0.02f;
            g_cos_angle = cosf(angle);
            g_sin_angle = sinf(angle);
        }

        // Atualiza o pulso triangular
        pulse_update_triangular(&phase, &pulse_r2, &direction);

        // Atualiza o CA
        update_ca(pulse_r2);

        // Renderização
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        // Desenha os eixos cartesianos
        draw_axes(ren);

        // Desenha de acordo com o modo atual
        render_3d(ren, pulse_r2);

        // Mostra o modo atual na janela (através do título)
        char title[200];
        const char* mode_name;
        switch(g_view_mode) {
            case VIEW_MODE_3D_FULL: mode_name = "3D FULL"; break;
            case VIEW_MODE_2D_SLICE: mode_name = "2D SLICE"; break;
            case VIEW_MODE_3D_SHELL_ONLY: mode_name = "3D SHELL"; break;
            default: mode_name = "UNKNOWN";
        }
        snprintf(title, sizeof(title), "CA Wavefront - %s - Pulse r2=%u", mode_name, pulse_r2);
        SDL_SetWindowTitle(win, title);

        SDL_RenderPresent(ren);

        tick++;
        if (tick == 500) {
            analyze_metric_precision();
        }

        SDL_Delay(16);
    }

cleanup:
    free(grid);
    free(grid_next);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
#endif
