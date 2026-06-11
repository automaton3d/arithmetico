/*
 * pulsating.h
 * CA Wavefront com detecção local de superfície ativa
 * SEM divisão, SEM módulo, SEM floats
 */

#ifndef PULSATING_H_
#define PULSATING_H_

#include <SDL3/SDL.h>
#include <stdint.h>

#define L 121
#define INF_R2 0xFFFFFFFFu
#define GRID_SPACING 8
#define ACTIVE_THICKNESS 2

/* Parâmetros do pulso (constantes) */
#define PULSE_MIN_R2 25
#define PULSE_STEP 7

typedef struct {
    unsigned int r2;      // distância Manhattan ponderada ao centro (r² Euclidiano exato)
    unsigned int active;  // 1 se está na superfície ativa no pulso atual
} Cell;

extern Cell (*grid)[L][L];
extern Cell (*grid_next)[L][L];

extern const int MID;
extern const int R_MAX;
extern const unsigned int PULSE_MAX_R2;
extern const unsigned int PULSE_SPAN;
extern const unsigned int PULSE_PERIOD;

/* Função para atualizar o pulso (controlador, não parte do CA) */
void pulse_update(unsigned int *phase, unsigned int *pulse_r2);

/* Atualiza a frente de onda - usa apenas estado local */
void update_wavefront(void);

/* Recalcula os bits active baseado no pulso atual */
void update_active_flags(unsigned int pulse_r2);

/* Atualização completa do CA */
void update_ca(unsigned int pulse_r2);

/* Renderização - recebe pulso como parâmetro, não consulta função global */
void render_frame(SDL_Renderer* ren,
                  SDL_Texture* tex,
                  uint32_t* pixels,
                  unsigned int pulse_r2);

/* Inicialização da grade */
void init_ca(void);

/* Análise de precisão */
void analyze_metric_precision(void);

#endif /* PULSATING_H_ */
