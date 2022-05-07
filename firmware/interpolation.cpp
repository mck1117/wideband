#include "interpolation.h"

int interpolateInt(int x1, int y1, int x2, int y2, int x)
{
    if (x1 == x2)
        return y1;

    return (y1 + (y2 - y1) * (x - x1) / (x2 - x1));
}

int interpolateIntClamped(int x1, int y1, int x2, int y2, int x)
{
    if (x <= x1)
        return y1;
    if (x >= x2)
        return y2;

    return interpolateInt(x1, y1, x2, y2, x);
}

int interpolate_1d_int(const struct inter_point *p, int size, int x)
{
    int i;

    /* no exterpolation */
    if (x < p[0].x)
        return p[0].y;

    for (i = 0; i < size - 1; i++) {
        if ((x >= p[i].x) && (x < p[i + 1].x)) {
            return interpolateInt(p[i].x, p[i].y, p[i + 1].x, p[i + 1].y, x);
        }
    }

    /* no exterpolation */
    return p[size - 1].y;
}
