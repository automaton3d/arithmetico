/*
 * ca_minimal.h
 *
 * Versão mínima do algoritmo de frente de onda.
 * Implementa a propagação exata de r² Euclidiano.
 *
 * Uso:
 *   1. init_ca()
 *   2. convergir: update_ca(pulse) até estabilizar
 *   3. loop: update_ca(pulse) e ler cell.active
 */

#ifndef CA_MINIMAL_H
#define CA_MINIMAL_H

#define L 61               // Dimensão (ímpar, < 127 para caber em inteiro)
#define INF_R2 0xFFFFFFFFu

typedef struct {
    unsigned int r2;       // distância² Euclidiana exata
    unsigned int active;   // 1 se |r2 - pulse_r2| <= 1
} Cell;

extern Cell grid[L][L][L];

void init_ca(void);
void update_ca(unsigned int pulse_r2);
void pulse_triangle(unsigned int *phase, unsigned int *direction, unsigned int *pulse_r2);

#endif
