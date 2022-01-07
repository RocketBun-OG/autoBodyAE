#pragma once
#include <Inverts.h>
#include <SimpleIni.h>
#include <eventhandling.h>
#include <maffs.h>
#include <papyrusfunc.h>
#include <presetmanager.h>

// Credit to Sairion for open sourcing OBody-SKSE, I used his code as reference in a number of places including:
//NORefit. It's a direct rip. Excellent trickery with the sliders, Sairion.
//event handling
//the MessageHandler skeleton & morph interface handshake
//morphman.h (specifically for getWeight as well as the conceptual idea of using a morph to mark an actor as genned).
namespace
{
	// this basically serves as our init function
	void MessageHandler(SKSE::MessagingInterface::Message* a_message)
	{
		auto morf = Bodygen::Morphman::GetInstance();
		switch (a_message->type) {
			// we set this messaging interface up so we can talk to SKEE
		case SKSE::MessagingInterface::kPostLoad:
			{
				SKEE::InterfaceExchangeMessage message;
				auto SKSEinterface = SKSE::GetMessagingInterface();
				SKSEinterface->Dispatch(
					SKEE::InterfaceExchangeMessage::kExchangeInterface, (void*)&message, sizeof(SKEE::InterfaceExchangeMessage*), "skee");
				// interface map contained within the message allows us to find the morph
				// interface. Like a treasure map.
				if (!message.interfaceMap) {
					logger::critical("Couldn't acquire interface map!");
					return;
				}

				// now we get the morph interface.
				auto morphInterface = static_cast<SKEE::IBodyMorphInterface*>(message.interfaceMap->QueryInterface("BodyMorph"));
				if (!morphInterface) {
					logger::critical("Couldn't acquire morph interface!");
					return;
				}
				logger::trace("Bodymorph Version {}", morphInterface->GetVersion());

				// now we pass the interface to our dearest friend morf.

				if (!morf->GetMorphInterface(morphInterface)) {
					logger::critical("BodyMorphInterface failed to pass to morphman! The mod will not work.");
				}

				Event::RegisterEvents();
				return;
			}
		case SKSE::MessagingInterface::kDataLoaded:
			{
				logger::trace("Data loaded!");

				logger::trace("checking config.");
				std::string path = Presets::Parsing::CheckConfig();

				auto presetcontainer = Presets::PresetContainer::GetInstance();

				Presets::Parsing::ParseAllInFolder(path, &presetcontainer->femaleMasterSet, &presetcontainer->maleMasterSet);
				logger::info("{} body presets were loaded into the female master list.", presetcontainer->femaleMasterSet.size());
				logger::info("{} body presets were loaded into the male master list.", presetcontainer->maleMasterSet.size());

				Presets::Parsing::CheckMorphConfig();
				morf->initClothingSliders();
				return;
			}
		}
	}

	// this hooks up the logfile output. Boilerplate.
	void InitializeLog()
	{
		// grabs SKSE's default log directory
		auto path = logger::log_directory();
		if (!path) {
			util::report_and_fail("Failed to find standard logging directory"sv);
		}

		*path /= "autoBody.log"sv;
		// basic file sink. Nothing special since I'm too stupid to get msvc to
		// work.
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

		const auto level = spdlog::level::info;

		auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
		log->set_level(level);
		log->flush_on(level);

		spdlog::set_default_logger(std::move(log));
		spdlog::set_pattern("autoBody v1: [%^%l%$] %v"s);
	}
}  // namespace

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;

	v.PluginVersion(Plugin::VERSION);
	v.PluginName(Plugin::NAME);

	v.UsesAddressLibrary(true);
	v.CompatibleVersions({ SKSE::RUNTIME_LATEST });

	return v;
}();

//basically int main()
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();

	logger::info("{} v{}"sv, "autoBody Plugin", Plugin::VERSION.string());
	SKSE::Init(a_skse);
	logger::trace("SKSE initialized.");
	auto message = SKSE::GetMessagingInterface();
	if (!message->RegisterListener(MessageHandler)) {
		return false;
	}

	//logger::trace("Clothingsliders is {} elements long in main", presetcontainer->clothingsliders.size());
	auto papyrus = SKSE::GetPapyrusInterface();
	if (!papyrus->Register(PapyrusBridging::BindAllFunctions)) {
		logger::critical("Papyrus bridge failed!");
		return false;
	} else {
		logger::trace("Papyrus bridge succeeded. Native functions should now be available.");
	}

	return true;
}
