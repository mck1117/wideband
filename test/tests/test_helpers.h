#pragma once

#include <cstdint>

constexpr float kValueDelta = 0.01f; // Small value delta for boundary testing

constexpr int64_t toUs(float seconds)
{
    return static_cast<int64_t>(seconds * 1'000'000.0f);
}

// Helper functions to express values just above or below a threshold, for boundary testing.
constexpr float under(float value, float delta = kValueDelta)
{
    return value - delta;
}

constexpr float over(float value, float delta = kValueDelta)
{
    return value + delta;
}