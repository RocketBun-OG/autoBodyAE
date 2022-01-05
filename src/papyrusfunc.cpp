#include <papyrusfunc.h>

namespace PapyrusBridging
{
	// wrappers around our native functions so we can call them with papyrus
	// scripts!

	void RegenActor(RE::StaticFunctionTag*, RE::Actor* a_actor) { Presets::HandleGeneration(a_actor, true); }

	void SetINIKey(RE::StaticFunctionTag*, std::string sectionname, std::string keyname, std::string value)
	{
		Presets::Parsing::SetConfigKey(sectionname.c_str(), keyname.c_str(), value.c_str());
	}

	void ApplyPresetByName(RE::StaticFunctionTag*, RE::Actor* a_actor, std::string presetname)
	{
		// logger::trace("ApplyPresetByName was called from a script!");
		auto morphman = Bodygen::Morphman::GetInstance();
		std::vector<Presets::bodypreset> list;
		auto sexint = a_actor->GetActorBase()->GetSex();

		if (sexint == 1) {
			list = Presets::PresetContainer::GetInstance()->femaleMasterSet;
		} else {
			list = Presets::PresetContainer::GetInstance()->maleMasterSet;
		}

		for (Presets::bodypreset item : list) {
			// logger::trace("We're looking at {} in the loop right now", item.name);
			if (item.name == presetname) {
				// logger::trace("Match found!");
				Presets::completedbody readybody = InterpolateAllValues(item, morphman->GetWeight(a_actor));
				// logger::trace("{} is the size of the body we just made",
				// readybody.nodelist.size());
				morphman->ApplySliderSet(a_actor, readybody, "autoBody");
				return;
			}
		}
	}

	auto GetActorPresetPool(RE::StaticFunctionTag*, RE::Actor* a_actor)
	{
		std::vector<RE::BSFixedString> output;

		if (Presets::isInINI(a_actor)) {
			auto actorbodylist = Presets::findActorInINI(a_actor);
			for (Presets::bodypreset item : actorbodylist) { output.push_back(item.name); }
		}
		if (Presets::isInINI(a_actor, true)) {
			auto racebodylist = Presets::findRaceSexInINI(a_actor);
			for (Presets::bodypreset item : racebodylist) { output.push_back(item.name); }
		}
		if (Presets::isInINI(a_actor, false, true)) {
			auto factionbodylist = Presets::findFactionInINI(a_actor);
			for (Presets::bodypreset item : factionbodylist) { output.push_back(item.name); }
		}
		return output;
	}

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

	bool GetActorGenerated(RE::StaticFunctionTag*, RE::Actor* a_actor)
	{
		auto morphman = Bodygen::Morphman::GetInstance();
		return morphman->IsGenned(a_actor);
	}

	bool BindAllFunctions(VM* a_vm)
	{
		a_vm->RegisterFunction("RegenActor", "autoBodyUtils", RegenActor);
		a_vm->RegisterFunction("SetINIKey", "autoBodyUtils", SetINIKey);
		a_vm->RegisterFunction("ApplyPresetByName", "autoBodyUtils", ApplyPresetByName);
		a_vm->RegisterFunction("GetActorPresetPool", "autoBodyUtils", GetActorPresetPool);
		a_vm->RegisterFunction("GetMasterPresetPool", "autoBodyUtils", GetMasterPresetPool);
		a_vm->RegisterFunction("GetActorGenerated", "autoBodyUtils", GetActorGenerated);
		return true;
	}

}  // namespace PapyrusBridging
