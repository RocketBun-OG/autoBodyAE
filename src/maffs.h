#pragma once
#include <libs.h>
#include <presetmanager.h>

// calculates a slider value for a given weight.
float InterpolateSliderValue(Presets::slider bodyslider, float weight);

// creates a completed body for application to an actor. These are discarded after use.
Presets::completedbody InterpolateAllValues(Presets::bodypreset body, int weight);
