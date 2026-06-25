/*
 * pulsating_3d.h
 * Visualizador 3D isométrico para o CA
 */

#ifndef PULSATING_3D_H_
#define PULSATING_3D_H_

#include <SDL3/SDL.h>
#include <stdint.h>
#include <math.h>

/* Configuração da projeção isométrica */
#define ISO_ANGLE 0.523599f  // 30 graus em radianos
#define DEFAULT_SCALE 4.0f   // Escala padrão para projeção

/* Modos de visualização */
#define VIEW_MODE_3D_FULL 0      // Mostra todos os voxels ativos em 3D
#define VIEW_MODE_2D_SLICE 1     // Mostra apenas o corte equatorial (z = MID)
#define VIEW_MODE_3D_SHELL_ONLY 2 // Mostra apenas a casca (voxels ativos na superfície)

/* Variáveis globais do visualizador */
extern int g_center_x;
extern int g_center_y;
extern float g_cos_angle;
extern float g_sin_angle;
extern float g_scale;
extern int g_view_mode;  // Modo de visualização atual

/* Estrutura para ponto 3D projetado */
typedef struct {
    int screen_x;
    int screen_y;
    unsigned int r2;
    unsigned char active;
} ProjectedPoint;

/* Funções de visualização 3D */
void project_isometric(int x, int y, int z, int center_x, int center_y, int *sx, int *sy);
void render_3d(SDL_Renderer* ren, unsigned int pulse_r2);
void render_3d_fast(SDL_Renderer* ren, unsigned int pulse_r2);
void render_3d_thick_shell(SDL_Renderer* ren, unsigned int pulse_r2);
void render_3d_active(SDL_Renderer* ren, unsigned int pulse_r2);
void render_2d_slice(SDL_Renderer* ren, unsigned int pulse_r2);  // Corte equatorial
void draw_axes(SDL_Renderer* ren);
void init_3d_view(int window_width, int window_height);
void toggle_view_mode(void);  // Alterna entre os modos

#endif /* PULSATING_3D_H_ */
