#include <maffs.h>
#include <morphman.h>

//simple formula to linearly interpolate a slider to its value at some weight.
float InterpolateSliderValue(Presets::slider bodyslider, float weight)
{
	const int maxweight = 100;
	const int minweight = 0;
	float finalvalue = 0;

	finalvalue = ((bodyslider.min * (maxweight - weight)) + (bodyslider.max * (weight - minweight)));
	finalvalue /= (maxweight - minweight);

	return finalvalue;
}

// creates a completed body for application to an actor. These are discarded after use.
Presets::completedbody InterpolateAllValues(Presets::bodypreset body, float weight, bool clothing)
{
	//create a random number generator
	std::random_device random_device;
	//this uses the mersenne twister engine to generate a random number.
	std::mt19937 engine{ random_device() };
	std::uniform_int_distribution<int> numdist(0, 100);

	auto morphman = Bodygen::Morphman::GetInstance();

	//based on options set in the config, this influences weight differently.
	if (!clothing) {
		switch (morphman->weightOptions) {
		case 0:
			logger::trace("Using standard actor weight of {}", weight);
			weight = weight;
			break;
		case 1:
			//logger::trace("Using random weight.");
			weight = numdist(engine);
			//logger::trace("Random number selected was {}", weight);
			break;
		case 2:
			//logger::trace("Using specific weight of {}", morphman->weightSpecific);
			weight = morphman->weightSpecific;
			//logger::trace("Specific weight is {}", weight);
			break;
		}
		//if weightbias is enabled, tune the weight value by that bias amount before flattening the sliders.
		if (morphman->enableWeightBias) {
			weight += morphman->biasamount;
		}
	}

	Presets::completedbody out;

	// flatten all sliders into their final values
	for (Presets::slider interp : body.sliderlist) {
		float floatyweight = weight;
		Presets::flattenedslider pushin{ InterpolateSliderValue(interp, weight), interp.name };
		out.nodelist.push_back(pushin);
	}

	// keep track of what preset we just grabbed, and the weight we generated.
	out.presetname = body.name;
	out.weight = weight;
	logger::trace("The weight generated was {}", weight);
	// logger::info("Preset {} has been interpolated! Ready for application.", body.name);

	return out;
}
