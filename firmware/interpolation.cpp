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
