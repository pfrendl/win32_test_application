#include <stdlib.h>

#include "random.h"


#define M_PI 3.14159265358979323846


double randu() {
    return (double)rand() / (double)RAND_MAX;
}


Vec2 randn() {
    double u1 = (double)rand() / (double)RAND_MAX;
    double u2 = (double)rand() / (double)RAND_MAX;
    double r = sqrt(-2 * log(u1));
    double z0 = r * cos(2 * M_PI * u2);
    double z1 = r * sin(2 * M_PI * u2);
    return {z0, z1};
}


double random_uniform(double low, double high) {
    return low + (high - low) * randu();
}


Vec2 random_normal(Vec2 mean, double std) {
    Vec2 n = randn();
    return {mean.x + std * n.x, mean.y + std * n.y};
}
