#pragma once
#include <libs.h>
#include <morphman.h>
#include <presetmanager.h>
namespace Event
{
	// this fires when an object holding a script (i.e. NPCs) loads their scripts. We use this for body application triggers.
	class InitScriptEventHandler : public RE::BSTEventSink<RE::TESInitScriptEvent>
	{
	public:
		static InitScriptEventHandler* GetSingleton()
		{
			static InitScriptEventHandler singleton;
			return &singleton;
		}

		virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESInitScriptEvent* a_event, RE::BSTEventSource<RE::TESInitScriptEvent>*)
		{
			if (!a_event || !a_event->objectInitialized->Is3DLoaded()) {
				return RE::BSEventNotifyControl::kContinue;
			}

			RE::Actor* actor = a_event->objectInitialized->As<RE::Actor>();
			if (actor) {
				Presets::HandleGeneration(actor, false);
			}
			return RE::BSEventNotifyControl::kContinue;
		}

	private:
		InitScriptEventHandler() = default;
		InitScriptEventHandler(const InitScriptEventHandler&) = delete;
		InitScriptEventHandler(InitScriptEventHandler&&) = delete;
		virtual ~InitScriptEventHandler() = default;

		InitScriptEventHandler& operator=(const InitScriptEventHandler&) = delete;
		InitScriptEventHandler& operator=(InitScriptEventHandler&&) = delete;
	};

	// Credit to Sairion for this function
	class EquipEventHandler : public RE::BSTEventSink<RE::TESEquipEvent>
	{
	public:
		static EquipEventHandler* GetSingleton()
		{
			static EquipEventHandler singleton;
			return &singleton;
		}

		virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* a_event, RE::BSTEventSource<RE::TESEquipEvent>*)
		{
			auto morphman = Bodygen::Morphman::GetInstance();

			auto actor = a_event->actor->As<RE::Actor>();
			if (!a_event || !actor || !morphman->usingClothes)
				return RE::BSEventNotifyControl::kContinue;

			auto form = RE::TESForm::LookupByID(a_event->baseObject);
			if (!form)
				return RE::BSEventNotifyControl::kContinue;

			if (form->Is(RE::FormType::Armor) || form->Is(RE::FormType::Armature)) {
				auto dobj = RE::BGSDefaultObjectManager::GetSingleton();
				auto keywordNPC = dobj->GetObject<RE::BGSKeyword>(RE::DEFAULT_OBJECT::kKeywordNPC);

				if (actor->HasKeyword(keywordNPC) && (actor->GetActorBase()->GetSex() == 1)) {
					// logger::info("Processing equipment {} ", actor->GetName());

					bool removingBody = false;
					if (!a_event->equipped) {
						auto changes = actor->GetInventoryChanges();
						auto const armor = changes->GetArmorInSlot(32);
						if (armor) {
							removingBody = (armor->formID == form->formID);
						} else {
							removingBody = true;
						}
					}

					morphman->ProcessClothing(actor, removingBody);
				}
			}

			return RE::BSEventNotifyControl::kContinue;
		}

	private:
		EquipEventHandler() = default;
		EquipEventHandler(const EquipEventHandler&) = delete;
		EquipEventHandler(EquipEventHandler&&) = delete;
		virtual ~EquipEventHandler() = default;

		EquipEventHandler& operator=(const EquipEventHandler&) = delete;
		EquipEventHandler& operator=(EquipEventHandler&&) = delete;
	};

	void RegisterEvents()
	{
		auto events = RE::ScriptEventSourceHolder::GetSingleton();

		if (events) {
			logger::trace("Event hookup found. Registering events. ");
			events->AddEventSink(InitScriptEventHandler::GetSingleton());
			events->AddEventSink(EquipEventHandler::GetSingleton());
		}
	}

}  // namespace Event
