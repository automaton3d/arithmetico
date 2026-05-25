# arithmetico

**Euclidean geometry emerging from pure unsigned integer arithmetic.**  
A minimal 3D cellular automaton in which the exact quadratic metric $r^2 = x^2 + y^2 + z^2$ is reconstructed through local natural-number wavefront propagation.

---

## Overview

This project demonstrates that Euclidean quadratic geometry can emerge from strictly local rules defined over the natural numbers $\mathbb{N}$. The model uses only unsigned integer additions and comparisons — no floating-point arithmetic, signed coordinates, or global metric evaluations are required during propagation.

Each lattice site stores a single scalar field $r^2$, representing the minimal accumulated squared distance from the origin. The propagation rule is based on the discrete quadratic increment identity:

$$
(x+1)^2 - x^2 = 2x + 1
$$

(and cyclic permutations for $y$ and $z$). Through repeated local application of this identity, the global field converges exactly to the Euclidean metric.

## Key Features

- **Exact metric reconstruction**: Proven to recover $r^2 = x^2 + y^2 + z^2$ with zero error on all interior points.
- **Minimal state**: Only one unsigned integer per cell.
- **Unsigned arithmetic only**: Entirely over natural numbers, friendly to low-level hardware.
- **Emergent causal fronts**: A pulsating spherical shell expands and contracts at stable effective velocity.
- **High performance**: Designed for massive parallelism (FPGAs, GPUs, systolic arrays).
- **Visualization**: Real-time 2D cross-section rendered with SDL3.

## Numerical Validation

On a $121 \times 121 \times 121$ lattice ($R = 60$), the algorithm was tested on 904,089 points inside the inscribed sphere. All error metrics (maximum quadratic error, maximum radial error, RMS radial error, and RMS relative error) equal exactly zero, confirming exact reconstruction of the Euclidean metric from local updates alone.

## Building and Running

### Prerequisites
- SDL3 library
- C compiler (gcc/clang)
