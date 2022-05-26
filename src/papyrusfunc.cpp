#include <papyrusfunc.h>

namespace PapyrusBridging
{
	// wrappers around our native functions so we can call them with papyrus!

	//wrapper for HandleGeneration
	void RegenActor(RE::StaticFunctionTag*, RE::Actor* a_actor) { Presets::HandleGeneration(a_actor, true); }

	//sets a key on the INI. Just a wrapper for SetConfigKey.
	void SetINIKey(RE::StaticFunctionTag*, std::string sectionname, std::string keyname, std::string value) { Presets::Parsing::SetConfigKey(sectionname.c_str(), keyname.c_str(), value.c_str()); }

	//does what it says on the tin! Put in an actor and a preset name, and the actor gets the preset.
	void ApplyPresetByName(RE::StaticFunctionTag*, RE::Actor* a_actor, std::string presetname)
	{
		// logger::trace("ApplyPresetByName was called from a script!");
		auto actor_name = a_actor->GetName();
		//first get the morph manager.
		auto morphman = Bodygen::Morphman::GetInstance();
		std::vector<Presets::bodypreset> list;

		//then acquire the actor's sex, so we know which set to search for the named preset.
		auto sexint = a_actor->GetActorBase()->GetSex();

		if (sexint == 1) {
			list = Presets::PresetContainer::GetInstance()->femaleBackupSet;
		} else {
			list = Presets::PresetContainer::GetInstance()->maleBackupSet;
		}

		//now sift through the master list we just selected and try to find the preset.
		for (Presets::bodypreset item : list) {
			logger::trace("Checking {}", item.name);
			if (item.name == presetname) {
				//if we find a match, set it up to be applied by interpolating it to the correct weight value (whatever that is, based on the user's settings).
				Presets::completedbody readybody = InterpolateAllValues(item, morphman->GetWeight(a_actor));

				//then wipe any previous preset AutoBody has applied to them.
				morphman->morphInterface->ClearBodyMorphKeys(a_actor, "autoBody");

				//then finally apply the preset.				
				logger::trace("{}, the selected preset, is now being applied to {}.", readybody.presetname, actor_name);

				morphman->morphInterface->SetMorph(a_actor, "autoBody_weight", "autoBody_weight", readybody.weight);

				logger::trace("The simulated weight of the {} is {}", actor_name, morphman->morphInterface->GetMorph(a_actor, "autoBody_weight", "autoBody_weight"));

				morphman->ApplySliderSet(a_actor, readybody, "autoBody");
				return;
			}
		}
		logger::info("The preset selected for ApplyPresetByName does not exist!");
		return;
	}

	//gets the list of eligible presets for an actor
	auto GetActorPresetPool(RE::StaticFunctionTag*, RE::Actor* a_actor)
	{
		std::vector<RE::BSFixedString> output;
		std::vector<Presets::bodypreset> master;
		auto container = Presets::PresetContainer::GetInstance();

		bool specific = false;
		//what this block is doing is looking through each body list the actor could potentially be in.
		//If the actor is found, we mark it as "specific," i.e. not using the master preset list.
		if (Presets::isInINI(a_actor)) {
			auto actorbodylist = Presets::findActorInINI(a_actor);
			for (Presets::bodypreset item : actorbodylist) { output.push_back(item.name); }
			specific = true;
		}
		if (Presets::isInINI(a_actor, true)) {
			auto racebodylist = Presets::findRaceSexInINI(a_actor);
			for (Presets::bodypreset item : racebodylist) { output.push_back(item.name); }
			specific = true;
		}
		if (Presets::isInINI(a_actor, false, true)) {
			auto factionbodylist = Presets::findFactionInINI(a_actor);
			for (Presets::bodypreset item : factionbodylist) { output.push_back(item.name); }
			specific = true;
		}

		//But if they're not found in any of those body lists, default back to the master list of presets for their sex instead.
		if (!specific) {
			logger::trace("Actor was not found to match body lists. Using the master set instead.");
			if (a_actor->GetActorBase()->GetSex() == 1) {
				master = container->femaleMasterSet;
			} else {
				master = container->maleMasterSet;
			}

			for (Presets::bodypreset item : master) {
				logger::trace("{} is being looked at for their presets", item.name);
				output.push_back(item.name);
			}
		}
		return output;
	}

	//gets the master list of presets for a sex
	auto GetMasterPresetPool(RE::StaticFunctionTag*, bool female = true)
	{
		std::vector<RE::BSFixedString> output;

		auto container = Presets::PresetContainer::GetInstance();

		std::vector<Presets::bodypreset> list;

		if (female) {
			list = container->femaleMasterSet;
		} else {
			list = container->maleMasterSet;
		}

		for (Presets::bodypreset item : list) { output.push_back(item.name); }

		return output;
	}

	//gets the raw list of presets for a sex, ignoring all restrictions in morphs.ini
	auto GetBackupMasterPool(RE::StaticFunctionTag*, bool female = true)
	{
		std::vector<RE::BSFixedString> output;

		auto container = Presets::PresetContainer::GetInstance();

		std::vector<Presets::bodypreset> list;

		if (female) {
			list = container->femaleBackupSet;
		} else {
			list = container->maleBackupSet;
		}

		for (Presets::bodypreset item : list) { output.push_back(item.name); }

		return output;
	}

	//returns true if an actor has been generated by autoBody already. Wrapper for IsGenned.
	bool GetActorGenerated(RE::StaticFunctionTag*, RE::Actor* a_actor)
	{
		auto morphman = Bodygen::Morphman::GetInstance();
		return morphman->IsGenned(a_actor);
	}

	void ClearAllMorphs(RE::StaticFunctionTag*, RE::Actor* a_actor)
	{
		auto morphman = Bodygen::Morphman::GetInstance();
		morphman->morphInterface->ClearBodyMorphKeys(a_actor, "autoBody");
		morphman->morphInterface->ClearBodyMorphKeys(a_actor, "autoBodyClothes");
		morphman->morphInterface->ApplyBodyMorphs(a_actor);
	}

	bool BindAllFunctions(VM* a_vm)
	{
		a_vm->RegisterFunction("RegenActor", "autoBodyUtils", RegenActor);
		a_vm->RegisterFunction("SetINIKey", "autoBodyUtils", SetINIKey);
		a_vm->RegisterFunction("ApplyPresetByName", "autoBodyUtils", ApplyPresetByName);
		a_vm->RegisterFunction("GetActorPresetPool", "autoBodyUtils", GetActorPresetPool);
		a_vm->RegisterFunction("GetMasterPresetPool", "autoBodyUtils", GetMasterPresetPool);
		a_vm->RegisterFunction("GetBackupMasterPool", "autoBodyUtils", GetBackupMasterPool);
		a_vm->RegisterFunction("ClearAllMorphs", "autoBodyUtils", ClearAllMorphs);
		a_vm->RegisterFunction("GetActorGenerated", "autoBodyUtils", GetActorGenerated);
		return true;
	}

}  // namespace PapyrusBridging
