/*
 * ca_minimal.c
 *
 * Implementação mínima. Nenhum print, nenhuma dependência externa.
 * Onde ocorreria a interação AND está marcado com "// INTERAÇÃO AND"
 */

#include "ca_minimal.h"

#define MID (L/2)
#define MAX_R2 ((L/2)*(L/2)*92/100)

Cell grid[L][L][L];
static Cell next[L][L][L];

/* ============================================================ */
void init_ca(void)
{
    for (int i = 0; i < L*L*L; i++) {
        ((Cell*)grid)[i].r2 = INF_R2;
        ((Cell*)grid)[i].active = 0;
    }
    grid[MID][MID][MID].r2 = 0;
}

/* ============================================================ */
void pulse_triangle(unsigned int *phase, unsigned int *direction, unsigned int *pulse_r2)
{
    if (*direction == 0) {
        if (*phase < MAX_R2) (*phase)++;
        else { *direction = 1; if (*phase > 0) (*phase)--; }
    } else {
        if (*phase > 0) (*phase)--;
        else { *direction = 0; if (*phase < MAX_R2) (*phase)++; }
    }
    *pulse_r2 = *phase;
}

/* ============================================================ */
static unsigned int abs_diff(unsigned int a, unsigned int b)
{
    return (a > b) ? (a - b) : (b - a);
}

/* ============================================================ */
void update_ca(unsigned int pulse_r2)
{
    // --- Propagação da frente de onda (Dijkstra síncrono) ---
    for (int i = 0; i < L*L*L; i++) {
        ((Cell*)next)[i] = ((Cell*)grid)[i];
    }

    for (int x = 0; x < L; x++)
    for (int y = 0; y < L; y++)
    for (int z = 0; z < L; z++) {
        Cell *c = &grid[x][y][z];
        if (c->r2 == INF_R2) continue;

        int ax = (x > MID) ? (x - MID) : (MID - x);
        int ay = (y > MID) ? (y - MID) : (MID - y);
        int az = (z > MID) ? (z - MID) : (MID - z);

        // 6 vizinhos
        for (int d = 0; d < 6; d++) {
            int nx = x, ny = y, nz = z;
            unsigned int diff = 0;

            switch(d) {
                case 0: if (x+1 >= L) continue; nx = x+1; diff = 2*ax + 1; break;
                case 1: if (x == 0) continue; nx = x-1; diff = 2*ax + 1; break;
                case 2: if (y+1 >= L) continue; ny = y+1; diff = 2*ay + 1; break;
                case 3: if (y == 0) continue; ny = y-1; diff = 2*ay + 1; break;
                case 4: if (z+1 >= L) continue; nz = z+1; diff = 2*az + 1; break;
                case 5: if (z == 0) continue; nz = z-1; diff = 2*az + 1; break;
            }

            unsigned int new_r2 = c->r2 + diff;
            Cell *n = &next[nx][ny][nz];
            if (new_r2 < n->r2) {
                n->r2 = new_r2;
                n->active = 0;
            }
        }
    }

    next[MID][MID][MID].r2 = 0;

    // --- Troca grids ---
    for (int i = 0; i < L*L*L; i++) {
        ((Cell*)grid)[i] = ((Cell*)next)[i];
    }

    // --- Atualiza flags de ativação (espessura = 1) ---
    for (int i = 0; i < L*L*L; i++) {
        Cell *c = &((Cell*)grid)[i];
        if (c->r2 == INF_R2) {
            c->active = 0;
        } else {
            c->active = (abs_diff(c->r2, pulse_r2) <= 1);
        }
    }

    // ========================================================
    // INTERAÇÃO AND: todas as células com active == 1 neste
    // frame estão prontas para interagir.
    //
    // Exemplo de uso:
    //   if (cell.active) {
    //       // Dispara interação AND
    //   }
    // ========================================================
}
