#pragma once
#include "maffs.h"

namespace Bodygen
{
	// these are stored in actorList.Currently unused.
	struct completedcharacter
	{
		Presets::completedbody body;
		RE::Actor* completedactor;
		RE::FormID referenceID;
		std::string name = "";
		float weight = 1.0f;

		// for comparing characters in actorList
		bool operator==(const completedcharacter& c) { return (c.name._Equal(name)); }
	};
	// for bEnableClothingRefit
	extern std::vector<Presets::slider> clothingsliders;
	extern Presets::bodypreset clothingUnprocessed;
	extern Presets::completedbody clothingProcessed;

	class Morphman
	{
	public:
		//features
		bool usingRace;
		bool usingFaction;
		bool usingClothes;

		bool lazyInstall;

		//options
		bool factionPriority;
		bool enableWeightBias;
		int biasamount{ 0 };
		int weightOptions{ 0 };
		int weightSpecific{ 0 };
		int refitFactor{ 0 };
		//other
		bool clothingInit;

		std::vector<completedcharacter> actorList;

		SKEE::IBodyMorphInterface* morphInterface;

		Morphman() = default;

		static Morphman* GetInstance();

		Presets::completedbody FinishClothing(RE::Actor* a_actor);

		bool GetMorphInterface(SKEE::IBodyMorphInterface* a_morphInterface);

		// checks to see if an actor has been generated already.
		bool IsGenned(RE::Actor* a_actor);

		// sticks a RANDOM preset onto an NPC. This is the core function of the plugin, pretty much.
		void ApplyPreset(RE::Actor* a_actor, std::vector<Presets::bodypreset> list);

		// apply a flattened slider (i.e. a single value and a name, i.e. what the morph interface takes in), plus a morph key
		void ApplySlider(RE::Actor* a_actor, Presets::flattenedslider slider, const char* key);

		// apply an entire sliderset to a person.
		void ApplySliderSet(RE::Actor* a_actor, Presets::completedbody body, const char* key);

		// apply or remove clothing sliders from an actor.
		void ProcessClothing(RE::Actor* a_actor, bool unequip, bool safeguard);

		// how fat are they? lul
		float GetWeight(RE::Actor* a_actor);
	};

};  // namespace Bodygen
