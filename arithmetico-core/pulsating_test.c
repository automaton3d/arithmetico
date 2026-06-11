/*
 * pulsating_test.c
 * Precision analysis for the emergent quadratic metric.
 * SEM divisão (exceto sqrt na análise, que é apenas para relatório)
 */

#include <stdio.h>
#include <math.h>

#include "pulsating.h"

void analyze_metric_precision(void)
{
    const unsigned sphere_limit_r2 = MID * MID;

    double max_r2_error = 0.0;
    double max_radial_error = 0.0;
    double rms_error = 0.0;
    double rms_relative = 0.0;
    unsigned long long samples = 0;

    int worst_x = 0, worst_y = 0, worst_z = 0;
    unsigned active_count = 0;
    unsigned total_inside = 0;

    for (unsigned x = 0; x < L; x++)
    for (unsigned y = 0; y < L; y++)
    for (unsigned z = 0; z < L; z++) {
        Cell *c = &grid[x][y][z];

        if (c->r2 == INF_R2)
            continue;

        int gx = (x > MID) ? (int)(x - MID) : -(int)(MID - x);
        int gy = (y > MID) ? (int)(y - MID) : -(int)(MID - y);
        int gz = (z > MID) ? (int)(z - MID) : -(int)(MID - z);

        double exact_r2 = (double)gx * gx + (double)gy * gy + (double)gz * gz;

        if (exact_r2 > (double)sphere_limit_r2)
            continue;

        total_inside++;
        if (c->active)
            active_count++;

        double propagated_r2 = (double)c->r2;
        double r2_error = (propagated_r2 > exact_r2) ? (propagated_r2 - exact_r2) : (exact_r2 - propagated_r2);

        double exact_r = sqrt(exact_r2);
        double propagated_r = sqrt(propagated_r2);
        double radial_error = (propagated_r > exact_r) ? (propagated_r - exact_r) : (exact_r - propagated_r);

        double relative_error = (exact_r > 0.0) ? (radial_error / exact_r) : 0.0;

        rms_error += radial_error * radial_error;
        rms_relative += relative_error * relative_error;
        samples++;

        if (r2_error > max_r2_error) {
            max_r2_error = r2_error;
        }

        if (radial_error > max_radial_error) {
            max_radial_error = radial_error;
            worst_x = gx;
            worst_y = gy;
            worst_z = gz;
        }
    }

    if (samples > 0) {
        rms_error = sqrt(rms_error / (double)samples);
        rms_relative = sqrt(rms_relative / (double)samples);
    }

    printf("\n");
    printf("========================================\n");
    printf("Emergent Sphere Precision Analysis\n");
    printf("========================================\n");
    printf("Lattice size              : %d\n", L);
    printf("Sphere radius limit       : %d\n", MID);
    printf("Sphere limit r2           : %u\n", sphere_limit_r2);
    printf("Samples inside sphere     : %llu\n", samples);
    printf("Total cells inside sphere : %u\n", total_inside);
    printf("Active cells (current)    : %u\n", active_count);
    printf("Max r2 error              : %.12f\n", max_r2_error);
    printf("Max radial error          : %.12f\n", max_radial_error);
    printf("RMS radial error          : %.12f\n", rms_error);
    printf("RMS relative error        : %.12e\n", rms_relative);
    printf("Worst point               : (%d,%d,%d)\n", worst_x, worst_y, worst_z);
    printf("========================================\n");
    fflush(stdout);
}
