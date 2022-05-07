#pragma once

struct inter_point
{
    int x;
    int y;
};

int interpolateInt(int x1, int y1, int x2, int y2, int x);
int interpolateIntClamped(int x1, int y1, int x2, int y2, int x);
int interpolate_1d_int(const struct inter_point *p, int size, int x);
