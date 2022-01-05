#include <morphman.h>
#include <presetmanager.h>
namespace PapyrusBridging
{
	using VM = RE::BSScript::IVirtualMachine;

	void RegenActor(RE::StaticFunctionTag*, RE::Actor* a_actor);

	void SetINIKey(RE::StaticFunctionTag*, std::string sectionname, std::string keyname, std::string value);

	void ApplyPresetByName(RE::StaticFunctionTag*, RE::Actor* a_actor, std::string presetname);

	auto GetActorPresetPool(RE::StaticFunctionTag*, RE::Actor* a_actor);

	auto GetMasterPresetPool(RE::StaticFunctionTag*);

	bool GetActorGenerated(RE::StaticFunctionTag*, RE::Actor* a_actor);

	bool BindAllFunctions(VM* a_vm);

}  // namespace PapyrusBridging
