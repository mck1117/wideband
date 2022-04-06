#include "interpolation.h"

float interpolateFloat(float x1, float y1, float x2, float y2, float x)
{
	if (x1 == x2)
		return y1;

	return (y1 + (y2 - y1) * (x - x1) / (x2 - x1));
}

float interpolateFloatClamped(float x1, float y1, float x2, float y2, float x)
{
	if (x <= x1)
		return y1;
	if (x >= x2)
		return y2;

	return interpolateFloat(x1, y1, x2, y2, x);
}

float interpolate_1d_float(const struct inter_point *p, int size, float x)
{
	int i;

	/* no exterpolation */
	if (x < p[0].x)
		return p[0].y;

	for (i = 0; i < size - 1; i++) {
		if ((x >= p[i].x) && (x < p[i + 1].x)) {
			return interpolateFloat(p[i].x, p[i].y, p[i + 1].x, p[i + 1].y, x);
		}
	}

	/* no exterpolation */
	return p[size - 1].y;
}
