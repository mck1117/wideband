#pragma once

struct inter_point {
	float		x;
	float		y;
};

float interpolateFloat(float x1, float y1, float x2, float y2, float x);
float interpolateFloatClamped(float x1, float y1, float x2, float y2, float x);
float interpolate_1d_float(const struct inter_point *p, int size, float x);
