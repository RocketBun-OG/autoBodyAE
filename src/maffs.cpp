#include <maffs.h>

float InterpolateSliderValue(Presets::slider bodyslider, float weight) {
  const int maxweight  = 100;
  const int minweight  = 0;
  float     finalvalue = 0;

  finalvalue = ((bodyslider.min * (maxweight - weight)) + (bodyslider.max * (weight - minweight)));
  finalvalue /= (maxweight - minweight);

  return finalvalue;
}

// creates a completed body for application to an actor. These are discarded after use.
Presets::completedbody InterpolateAllValues(Presets::bodypreset body, float weight, float weightbias) {
  logger::trace("Interpolation is taking place.");
  Presets::completedbody out;
  weight = weight + weightbias;
  if (weight > 100) {
    weight = 100;
  } else if (weight < 0) {
    weight = 0;
  }
  // flatten all sliders into their final values
  for (Presets::slider interp : body.sliderlist) {
    Presets::flattenedslider pushin{InterpolateSliderValue(interp, weight), interp.name};
    out.nodelist.push_back(pushin);
  }

  // keep track of what preset we just grabbed.
  out.presetname = body.name;

  // logger::info("Preset {} has been interpolated! Ready for application.", body.name);

  return out;
}
