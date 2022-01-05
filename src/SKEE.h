#pragma once

// Racemenu's SKEE interface, in header form so sources aren't included
#include <libs.h>

namespace SKEE
{
	class IPluginInterface
	{
	public:
		IPluginInterface(){};
		virtual ~IPluginInterface(){};

		virtual uint32_t GetVersion() = 0;
		virtual void Revert() = 0;
	};

	class IInterfaceMap
	{
	public:
		virtual IPluginInterface* QueryInterface(const char* name) = 0;
		virtual bool AddInterface(const char* name, IPluginInterface* pluginInterface) = 0;
		virtual IPluginInterface* RemoveInterface(const char* name) = 0;
	};

	struct InterfaceExchangeMessage
	{
		enum : uint32_t
		{
			kExchangeInterface = 0x9E3779B9
		};

		IInterfaceMap* interfaceMap = nullptr;
	};

	class IBodyMorphInterface : public IPluginInterface
	{
	public:
		class MorphKeyVisitor
		{
		public:
			virtual void Visit(const char*, float) = 0;
		};

		class StringVisitor
		{
		public:
			virtual void Visit(const char*) = 0;
		};

		class ActorVisitor
		{
		public:
			virtual void Visit(RE::TESObjectREFR*) = 0;
		};

		class MorphValueVisitor
		{
		public:
			virtual void Visit(RE::TESObjectREFR*, const char*, const char*, float) = 0;
		};

		class MorphVisitor
		{
		public:
			virtual void Visit(RE::TESObjectREFR*, const char*) = 0;
		};

		virtual void SetMorph(RE::TESObjectREFR* actor, const char* morphName, const char* morphKey, float relative) = 0;
		virtual float GetMorph(RE::TESObjectREFR* actor, const char* morphName, const char* morphKey) = 0;
		virtual void ClearMorph(RE::TESObjectREFR* actor, const char* morphName, const char* morphKey) = 0;

		virtual float GetBodyMorphs(RE::TESObjectREFR* actor, const char* morphName) = 0;
		virtual void ClearBodyMorphNames(RE::TESObjectREFR* actor, const char* morphName) = 0;

		virtual void VisitMorphs(RE::TESObjectREFR* actor, MorphVisitor& visitor) = 0;
		virtual void VisitKeys(RE::TESObjectREFR* actor, const char* name, MorphKeyVisitor& visitor) = 0;
		virtual void VisitMorphValues(RE::TESObjectREFR* actor, MorphValueVisitor& visitor) = 0;

		virtual void ClearMorphs(RE::TESObjectREFR* actor) = 0;

		virtual void ApplyVertexDiff(RE::TESObjectREFR* refr, RE::NiAVObject* rootNode, bool erase = false) = 0;

		virtual void ApplyBodyMorphs(RE::TESObjectREFR* refr, bool deferUpdate = true) = 0;
		virtual void UpdateModelWeight(RE::TESObjectREFR* refr, bool immediate = false) = 0;

		virtual void SetCacheLimit(size_t limit) = 0;
		virtual bool HasMorphs(RE::TESObjectREFR* actor) = 0;
		virtual uint32_t EvaluateBodyMorphs(RE::TESObjectREFR* actor) = 0;

		virtual bool HasBodyMorph(RE::TESObjectREFR* actor, const char* morphName, const char* morphKey) = 0;
		virtual bool HasBodyMorphName(RE::TESObjectREFR* actor, const char* morphName) = 0;
		virtual bool HasBodyMorphKey(RE::TESObjectREFR* actor, const char* morphKey) = 0;
		virtual void ClearBodyMorphKeys(RE::TESObjectREFR* actor, const char* morphKey) = 0;
		virtual void VisitStrings(StringVisitor& visitor) = 0;
		virtual void VisitActors(ActorVisitor& visitor) = 0;
		virtual size_t ClearMorphCache() = 0;
	};
}  // namespace SKEE
