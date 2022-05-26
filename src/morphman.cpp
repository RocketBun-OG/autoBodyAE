#include <morphman.h>

namespace Bodygen
{
	Morphman* Morphman::GetInstance()
	{
		static Morphman instance;
		return std::addressof(instance);
	};

	//the clothing sliders list is un-interpolated, so we need to do that here.
	Presets::completedbody Morphman::FinishClothing(RE::Actor* a_actor)
	{
		auto weight = GetWeight(a_actor);
		auto container = Presets::PresetContainer::GetInstance();
		auto morphman = Bodygen::Morphman::GetInstance();

		//if we've already applied a preset to them, we need to pay attention to what their simulated weight is.
		if (morphInterface->HasBodyMorph(a_actor, "autoBody_weight", "autoBody_weight")) {
			logger::trace("Grabbing the simulated weight from storage for actor {}...", a_actor->GetName());
			weight = morphInterface->GetMorph(a_actor, "autoBody_weight", "autoBody_weight");
			logger::trace("Simulated weight is {}", weight);
		}

		//jank
		Presets::bodypreset* clothingUnprocessed{ new Presets::bodypreset };
		Presets::completedbody* clothingmods{ new Presets::completedbody };

		//first grab the unprocessed list of sliders (defined directly above this function)
		clothingUnprocessed = &container->clothingUnprocessed;

		//then interpolate those sliders to the correct value for the target weight, using InterpolateAllValues
		*clothingmods = InterpolateAllValues(*clothingUnprocessed, weight, true);

		//apply modifiers to the clothing morphs based on refit factor, if necessary
		if (morphman->refitFactor != 0) {
			for (auto& slider : clothingmods->nodelist) {
				float percentage = morphman->refitFactor / 100.0f;
				slider.value = slider.value * percentage;
			}
		}

		//then just return the clothing sliders.
		return *clothingmods;
	}

	bool Morphman::GetMorphInterface(SKEE::IBodyMorphInterface* a_morphInterface) { return a_morphInterface->GetVersion() ? morphInterface = a_morphInterface : false; }

	// checks to see if an actor has been generated already.
	bool Morphman::IsGenned(RE::Actor* a_actor)
	{
		logger::trace("The value of the tagger morph is {}", morphInterface->GetMorph(a_actor, "autoBody_processed", "autoBody_processed"));
		return (morphInterface->HasBodyMorph(a_actor, "autoBody_processed", "autoBody_processed"));
	}

	// sticks a preset onto an NPC. This is the core function of the plugin, pretty much.
	void Morphman::ApplyPreset(RE::Actor* a_actor, std::vector<Presets::bodypreset> list)
	{
		//ensure that the list has at least one item to avoid divide by zero errors.
		//Then mark the actor as processed to avoid any more failures.
		if (list.size() == 0) {
			logger::trace("There are no presets in this list! We can't apply one!");
			morphInterface->SetMorph(a_actor, "autoBody_processed", "autoBody_processed", 1.0f);
			return;
		}
		// select a random preset from the stack
		auto preset = Presets::FindRandomPreset(list);

		// get the weight of the actor
		auto actorWeight = GetWeight(a_actor);

		//now interpolate the values of the sliders based on either their weight, a random weight, or a specific weight (depending on the user's settings).
		Presets::completedbody readybody = InterpolateAllValues(preset, actorWeight);

		// finally, clear off any sliders we may have already put on them.
		morphInterface->ClearBodyMorphKeys(a_actor, "autoBody");
		// prep complete

		//flag the actor with a morph containing their simulated weight.
		morphInterface->SetMorph(a_actor, "autoBody_weight", "autoBody_weight", readybody.weight);
		morphInterface->SetMorph(a_actor, "autoBody_processed", "autoBody_processed", 1.0f);
		logger::trace("The simulated weight of the actor is {}", morphInterface->GetMorph(a_actor, "autoBody_weight", "autoBody_weight"));

		// apply the sliders to the NPC
		ApplySliderSet(a_actor, readybody, "autoBody");

		// mark the actor as generated, so we don't fuck up and generate them again

		logger::info("Preset [{}] was applied to {}.", readybody.presetname, a_actor->GetName());

		return;
	}

	// apply a flattened slider (i.e. a single value and a name, i.e. what the morph
	// interface takes in), plus a morph key
	void Morphman::ApplySlider(RE::Actor* a_actor, Presets::flattenedslider slider, const char* key)
	{
		// logger::trace("Morph {} with value {} is being applied to actor {}",
		// slider.name, slider.value, a_actor->GetName());
		morphInterface->SetMorph(a_actor, slider.name.c_str(), key, slider.value);
	}

	// apply an entire sliderset to a person.
	void Morphman::ApplySliderSet(RE::Actor* a_actor, Presets::completedbody body, const char* key)
	{
		for (Presets::flattenedslider node : body.nodelist) {
			// logger::trace("Applying slider {} to the actor", node.name);
			ApplySlider(a_actor, node, key);
		}

		morphInterface->ApplyBodyMorphs(a_actor, true);
	}

	// apply or remove clothing sliders from an actor.
	void Morphman::ProcessClothing(RE::Actor* a_actor, bool unequip, bool safeguard)
	{
		auto morphmanager = Bodygen::Morphman::GetInstance();

		if (safeguard) {
			return;
		}

		auto sexint = a_actor->GetActorBase()->GetSex();
		if (sexint == 1) {
			auto modifiers = FinishClothing(a_actor);
			//logger::trace("Modifiers is {} big", modifiers.nodelist.size());

			//this block will always REMOVE clothing morphs, but only ADD them if clothing toggle is flipped.
			//This allows for the user to disable the refit function midsave and not wind up with "stuck" morphs.
			if (unequip) {
				logger::info("Removing clothing morphs from actor {}!", a_actor->GetName());
				morphInterface->ClearBodyMorphKeys(a_actor, "autoBodyClothes");
				morphInterface->ApplyBodyMorphs(a_actor, true);
			} else if (morphmanager->usingClothes) {
				logger::info("Applying clothing morphs to actor {}!", a_actor->GetName());
				morphInterface->ClearBodyMorphKeys(a_actor, "autoBodyClothes");
				ApplySliderSet(a_actor, modifiers, "autoBodyClothes");
			}
		}
	}

	// how fat are they? lul
	float Morphman::GetWeight(RE::Actor* a_actor)
	{
		//logger::trace("getting actor weight.");
		return a_actor->GetWeight();
	}
};  // namespace Bodygen
