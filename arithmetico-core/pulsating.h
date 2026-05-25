/*
 * pulsating.h
 */

#ifndef PULSATING_H_
#define PULSATING_H_

#include <SDL3/SDL.h>
#include <stdint.h>

#define L 121
#define INF_R2 0xFFFFFFFFu
#define GRID_SPACING 8

typedef struct {

    unsigned int r2;

} Cell;

extern Cell (*grid)[L][L];
extern Cell (*grid_next)[L][L];

extern const int MID;
extern const int R_MAX;

unsigned int pulse_from_time(unsigned int t);

void update_wavefront(void);

void update_ca(void);

void render_frame(SDL_Renderer* ren,
                  SDL_Texture* tex,
                  uint32_t* pixels,
                  unsigned int pulse_threshold_r2);

void init_ca(void);

void analyze_metric_precision(void);

#endif /* PULSATING_H_ */
