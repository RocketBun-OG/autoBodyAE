
#include <morphman.h>
#include <presetmanager.h>
namespace Presets
{
	// today i learned about LNK2005.
	bodypreset notfound{};
	std::vector<slider> nullsliders;
	std::vector<bodypreset> vecNotFound;

	PresetContainer* PresetContainer::GetInstance()
	{
		static PresetContainer instance;
		return std::addressof(instance);
	}

	bodypreset FindPresetByName(std::vector<bodypreset> searchable, std::string name)
	{
		//logger::trace("the searchable set has {} elements in it, and its first element is {}", searchable.size(), searchable.begin()[0].name);
		for (bodypreset searchitem : searchable) {
			//logger::trace("the items name is {} and the search target is {}", searchitem.name, name);
			if (searchitem.name == name) {
				//logger::trace("{} was found!", searchitem.name);
				return searchitem;
			}
		}
		// logger::info("Preset could not be found!");
		return notfound;
	}

	// selects a totally random preset from the list.
	bodypreset FindRandomPreset(std::vector<bodypreset> list)
	{
		//logger::trace("Attempting to pick a random preset.");
		int index = rand() % list.size();
		return list[index];
	}

	bool isInINI(RE::Actor* a_actor, bool racesex, bool faction)
	{
		bool found = false;

		auto container = Presets::PresetContainer::GetInstance();

		std::vector<categorizedList>* racelist{ new std::vector<categorizedList> };
		std::vector<categorizedList>* faclist{ new std::vector<categorizedList> };
		std::vector<categorizedList>* charlist{ new std::vector<categorizedList> };

		auto sexint = a_actor->GetActorBase()->GetSex();

		//first we check to see if the actor is male or female, to search through the right lists.
		std::string sex;
		if (sexint == 1) {
			//logger::trace("Sex is female.");
			racelist = &container->femaleRaceCategorySet;
			faclist = &container->femaleFactionCategorySet;
			charlist = &container->femaleCharacterCategorySet;

			sex = "Female";

		} else {
			//logger::trace("Sex is male");
			racelist = &container->maleRaceCategorySet;
			faclist = &container->maleFactionCategorySet;
			charlist = &container->maleCharacterCategorySet;

			sex = "Male";
		}

		if (racesex) {
			auto raceName = a_actor->GetActorBase()->GetRace()->GetFormEditorID();
			//logger::trace("Race name is: {}", raceName);
			for (categorizedList list : *racelist) {
				//logger::trace("list sex: {} list race: {}", list.sex, list.race);
				if (raceName == list.race) {
					logger::trace("Race match for actor {} found!", a_actor->GetName());
					found = true;
				}
			}
		}

		// Needs po3's tweaks to work or will CTD
		if (faction) {
			//logger::trace("Looking for a faction.");
			auto ranklist = a_actor->GetActorBase()->factions;

			// this function is a headache so here's a walkthrough
			// for each faction found in the actors list:
			for (RE::FACTION_RANK rank : ranklist) {
				// for each faction in the faction preset list:
				for (categorizedList fac : *faclist) {
					// look up the TESForm associated with the one in fac using the form
					// editor ID string contained in fac.faction
					auto target = RE::TESFaction::LookupByEditorID(fac.faction);
					//logger::trace("{} is what we're looking at in the faction loop", fac.faction);
					// if the form matches the form of the faction we're looking at in the
					// actors list, we have a match
					if (rank.faction == target) {
						logger::trace("faction match located: {}", fac.faction);
						found = true;
					}
				}
			}
		}

		uint32_t actorID = a_actor->GetActorBase()->formID;
		//logger::trace("ActorID is {}", actorID);
		for (categorizedList list : *charlist) {
			//logger::trace("FormID in the INI is {}", list.formID);
			if (actorID == list.formID) {
				logger::trace("{} found a matching config in the INI!", a_actor->GetName());
				found = true;
			}
		}

		return found;
	}

	// called when we want to generate an actor.
	RE::BSEventNotifyControl HandleGeneration(RE::Actor* actor, bool genOverride)
	{
		RE::FormID playerID;

		auto presetcontainer = Presets::PresetContainer::GetInstance();
		auto defaultobj = RE::BGSDefaultObjectManager::GetSingleton();
		auto keywordNPC = defaultobj->GetObject<RE::BGSKeyword>(RE::DEFAULT_OBJECT::kKeywordNPC);

		if (actor->HasKeyword(keywordNPC)) {
			auto morphman = Bodygen::Morphman::GetInstance();
			logger::trace("NPC {} is being examined.", actor->GetName());

			if (!morphman->IsGenned(actor) || genOverride) {
				//logger::trace("We've entered the inner loop of handlegen.");
				auto player = RE::PlayerCharacter::GetSingleton();
				playerID = player->formID;
				//// if we're processing the player load, ignore it and mark the player as
				/// processed so we don't spam the debug log.
				if (actor->formID == playerID) {
					morphman->morphInterface->SetMorph(actor, "autobody_processed", "autoBody", 1.0f);
					//logger::trace("We've detected the player. Skipping preset application.");
					return RE::BSEventNotifyControl::kContinue;
				}

				// if an actor is specifically identified in the INI, apply their preset
				// over everything else and return.
				if (Presets::isInINI(actor)) {
					//logger::trace("The actor is in the INI.");
					morphman->ApplyPreset(actor, Presets::findActorInINI(actor));
					return RE::BSEventNotifyControl::kContinue;
				}

				else if (morphman->usingFaction && Presets::isInINI(actor, false, true)) {
					//logger::trace("The faction is in the INI.");
					morphman->ApplyPreset(actor, Presets::findFactionInINI(actor));
					// if faction priority is enabled, the control flow stops here if a
					// faction body is found. Otherwise, itll check for race too and that
					// preset will override any faction body that was applied.
					if (morphman->factionPriority) {
						return RE::BSEventNotifyControl::kContinue;
					}
				}

				// if their race/sex combo is identified in the INI, apply that preset
				// over everything else and return.
				else if (Presets::isInINI(actor, true, false) && morphman->usingRace) {
					//logger::trace("The race is in the INI");
					morphman->ApplyPreset(actor, Presets::findRaceSexInINI(actor));
					return RE::BSEventNotifyControl::kContinue;
				} else {
					//ogger::trace("Actor matched no categories.");
				}

				//if we don't know any info, we have to identify their sex.
				//then we just apply from their default pool of presets.
				auto sexint = actor->GetActorBase()->GetSex();

				if (sexint == 1) {
					morphman->ApplyPreset(actor, presetcontainer->femaleMasterSet);
				} else {
					morphman->ApplyPreset(actor, presetcontainer->maleMasterSet);
				}

			} else {
				logger::trace("Actor has already had a preset applied. Skipping preset application.");
			}
		}
		return RE::BSEventNotifyControl::kContinue;
	}

	std::vector<bodypreset> findActorInINI(RE::Actor* a_actor)
	{
		logger::trace("Entered findActorInINI");
		uint32_t actorID = a_actor->GetActorBase()->GetFormID();

		//sex check!
		auto sexint = a_actor->GetActorBase()->GetSex();
		std::vector<Presets::categorizedList> charlist;
		if (sexint == 1) {
			charlist = Presets::PresetContainer::GetInstance()->femaleCharacterCategorySet;
		} else {
			charlist = Presets::PresetContainer::GetInstance()->maleCharacterCategorySet;
		}

		// logger::trace("entered findactor successfully");
		// logger::trace("the size of the character list is {}",
		// characterCategorySet.size());
		for (categorizedList list : charlist) {
			// logger::trace("entered for loop successfully");
			if (actorID == list.formID) {
				logger::trace("Actor body preset is being passed.");
				return list.categorizedSet;
			}
		}
		// this should never happen.
		logger::trace("Something has gone horribly wrong with actors.");
		vecNotFound[0] = { nullsliders, "" };
		return vecNotFound;
	}

	// same basic structure as the faction-finder in isInINI. Check the comments
	// there if you want to know more.
	std::vector<bodypreset> findFactionInINI(RE::Actor* a_actor)
	{
		//logger::trace("Entered findfactioninINI");
		auto ranklist = a_actor->GetActorBase()->factions;

		//sex check!
		auto sexint = a_actor->GetActorBase()->GetSex();
		std::vector<Presets::categorizedList> faclist;
		if (sexint == 1) {
			faclist = Presets::PresetContainer::GetInstance()->femaleFactionCategorySet;
		} else {
			faclist = Presets::PresetContainer::GetInstance()->maleFactionCategorySet;
		}

		for (RE::FACTION_RANK rank : ranklist) {
			for (categorizedList fac : faclist) {
				auto target = RE::TESFaction::LookupByEditorID(fac.faction);
				if (rank.faction == target) {
					//logger::info("Faction body preset is being passed");
					return fac.categorizedSet;
				}
			}
		}
		// this should never happen.
		logger::trace("Something has gone horribly wrong with factions.");
		vecNotFound[0] = { nullsliders, "" };
		return vecNotFound;
	}

	std::vector<bodypreset> findRaceSexInINI(RE::Actor* a_actor)
	{
		//sex check!
		auto sexint = a_actor->GetActorBase()->GetSex();
		std::vector<Presets::categorizedList> racelist;
		if (sexint == 1) {
			racelist = Presets::PresetContainer::GetInstance()->femaleRaceCategorySet;
		} else {
			racelist = Presets::PresetContainer::GetInstance()->maleRaceCategorySet;
		}

		std::string sex;
		if (sexint == 1) {
			sex = "Female";
		} else {
			sex = "Male";
		}
		auto raceName = a_actor->GetActorBase()->GetRace()->GetFormEditorID();
		for (categorizedList list : racelist) {
			if (sex == list.sex && raceName == list.race) {
				logger::info("Race body preset is being passed.");
				return list.categorizedSet;
			}
		}
		// this should never happen
		logger::trace("Something has gone horribly wrong with race.");
		vecNotFound[0] = { nullsliders, "" };
		return vecNotFound;
	}
	// std::vector<bodypreset> findFactionInINI(RE::Actor *a_actor) {}
	namespace Parsing
	{
		void PrintPreset(bodypreset preset)
		{
			logger::info("Now displaying preset: {}", preset.name);
			for (slider bar : preset.sliderlist) { logger::info(" > {}: [Small: {}] [Big: {}]", bar.name, bar.min, bar.max); }
		}

		void PrintPresetList(std::vector<bodypreset> setofsets)
		{
			logger::trace("Trying to print the preset list.");
			for (bodypreset body : setofsets) { logger::info("[ {} ] Number of sliders: [ {} ]", body.name, body.sliderlist.size()); }
		}

		// takes the list as a pointer
		void ParsePreset(std::string filename, std::vector<bodypreset>* femalelist, std::vector<bodypreset>* malelist)
		{
			std::string* presetname{ new std::string };

			std::vector<std::string>* presetgroups{ new std::vector<std::string> };

			std::vector<slider>* sliderset{ new std::vector<slider> };

			xml_document<>* preset{ new xml_document };
			xml_node<>* root_node = NULL;

			// logger::trace("Beginning xml parse...");

			// suck the xml into the buffer
			std::ifstream inputFile(filename);
			std::vector<char> buffer((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
			buffer.push_back('\0');

			// parse the buffer
			preset->parse<0>(&buffer[0]);

			// logger::trace("Buffer has been parsed.");
			//  find the root
			root_node = preset->first_node("SliderPresets");

			// parse!
			// for each preset, grab its name and go through the sliders.
			for (xml_node<>* preset_node = root_node->first_node("Preset"); preset_node; preset_node = preset_node->next_sibling()) {
				// we need this for the set of sets, to identify them.
				*presetname = preset_node->first_attribute()->value();

				//fill the group vector with groups to identify the sex of the preset.
				for (xml_node<>* cat_node = preset_node->first_node("Group"); cat_node; cat_node = cat_node->next_sibling("Group")) {
					//logger::trace("Adding {} to group list!", cat_node->first_attribute()->value());
					presetgroups->push_back(cat_node->first_attribute()->value());
				}

				//logger::trace("preset name being loaded is {}", *presetname);
				// discard clothed presets.
				if (presetname->find("Cloth") != -1 || presetname->find("cloth") != -1 || presetname->find("Outfit") != -1 ||
					presetname->find("outfit") != -1) {
					logger::trace("Clothed preset found. Discarding.");
					continue;
				}

				// logger::trace("{} has entered the outer for loop", *presetname);
				std::string* slidername{ new std::string };
				std::string* previousslidername{ new std::string("") };

				std::string* size{ new std::string("") };
				std::string* lastSize{ new std::string("") };

				float* sizevalue{ new float{ 0 } };
				float* lastsizevalue{ new float{ 0 } };
				// go through the sliders
				for (xml_node<>* slider_node = preset_node->first_node("SetSlider"); slider_node; slider_node = slider_node->next_sibling()) {
					// store the current values of this node
					*slidername = slider_node->first_attribute()->value();
					*size = slider_node->first_attribute("size")->value();

					// convert the size to a morphable value (those are -1 to 1.)
					*sizevalue = std::stoi(slider_node->first_attribute("value")->value()) / 100.0f;

					// if a pair is found, push it into the sliderset vector as a full struct.
					if (*slidername == *previousslidername) {
						// logger::trace("{} is a paired slider and is being pushed back",
						// *slidername);
						sliderset->push_back({ *lastsizevalue, *sizevalue, *slidername });
					}

					// if a pair is not found, evaluate which half it is and then push it as a
					// full struct with null values where they belong.
					else if (*slidername != *previousslidername && slider_node->next_sibling()) {
						if (slider_node->next_sibling()->first_attribute()->value() != *slidername) {
							if (*size == "big") {
								sliderset->push_back({ *sizevalue, 0, *slidername });
							} else {
								sliderset->push_back({ 0, *sizevalue, *slidername });
							}
						}
					}
					// push the values back for the next
					*previousslidername = *slidername;
					*lastSize = *size;
					*lastsizevalue = *sizevalue;
					// std::cout << " values: " << slidername << ",  " << size << ",  " <<
					// sizevalue << std::endl;
				}

				// clean up memory
				delete slidername;
				delete previousslidername;
				delete sizevalue;
				delete lastsizevalue;
				delete size;
				delete lastSize;

				slidername = nullptr;
				previousslidername = nullptr;
				sizevalue = nullptr;
				lastsizevalue = nullptr;
				size = nullptr;
				lastSize = nullptr;

				logger::info("{} has just been ingested into the master preset list.", *presetname);

				// push the preset into the master list, then erase the sliderset to start
				// again.
				for (std::string item : *presetgroups) {
					if (item._Equal("CBBE") || item._Equal("3BBB") || item._Equal("CBAdvanced")) {
						//logger::trace("Female preset found!");
						femalelist->push_back(bodypreset{ *sliderset, *presetname });
						break;
					} else if (item._Equal("HIMBO")) {
						//logger::trace("Male preset found!");
						malelist->push_back(bodypreset{ *sliderset, *presetname });
						break;
					}
				}
			}
			//  clean up memory
			delete presetgroups;
			presetgroups = nullptr;
			delete sliderset;
			sliderset = nullptr;
			return;
		}

		// finds all the xmls in a target folder, and parses them into an xml vector.
		void ParseAllInFolder(std::string path, std::vector<bodypreset>* femalelist, std::vector<bodypreset>* malelist)
		{
			std::string filename;
			for (auto const& dir_entry : std::filesystem::directory_iterator(path)) {
				filename = dir_entry.path().string();
				if (filename.substr(filename.find_last_of(".") + 1) == "xml") {
					size_t eraseamount = filename.find_last_of("/");
					filename.erase(0, eraseamount + 1);

					logger::trace("Now parsing {}...", filename);
					ParsePreset(filename, femalelist, malelist);
				}
			}

			return;
		}

		// Opens the config INI, reads off the settings, and inserts them where they
		// belong.

		std::string CheckConfig()
		{
			// we return this at the end to help the parser find the path.
			std::string presetPath;
			// reading main config
			CSimpleIniA configINI;

			configINI.SetUnicode();

			// load the file in
			SI_Error rc = configINI.LoadFile("./Data/autoBody/Config/autoBodyConfig.ini");
			if (rc < 0) {
				logger::critical("Config INI not found! Please make sure it is in /Data/autoBody/Config/");
			}

			bool failureswitch = false;
			//features
			int RaceToggle = -1;
			int FactionToggle = -1;
			int ClothesToggle = -1;

			int LazyInstall = -1;

			//options
			int FactionPriority = -1;
			int WeightBias = -1;
			int biasamount = -1;
			int weightOptions = -1;
			int weightSpecific = -1;

			auto morf = Bodygen::Morphman::GetInstance();
			logger::trace("reading off values...");
			// read our toggles and pass them to morphman
			RaceToggle = atoi(configINI.GetValue("Features", "bEnableRaceBodies"));
			FactionToggle = atoi(configINI.GetValue("Features", "bEnableFactionBodies"));
			ClothesToggle = atoi(configINI.GetValue("Features", "bEnableClothingRefit"));

			LazyInstall = atoi(configINI.GetValue("Features", "bEnableLazyInstall"));

			FactionPriority = atoi(configINI.GetValue("Options", "bFactionGenPriority"));
			WeightBias = atoi(configINI.GetValue("Options", "bEnableWeightBias"));
			biasamount = atoi(configINI.GetValue("Options", "bWeightBiasAmount"));
			weightOptions = atoi(configINI.GetValue("Options", "bWeightOptions"));
			weightSpecific = atoi(configINI.GetValue("Options", "bWeightSpecific"));

			// if a toggle comes up as -1, it means it wasn't found in the INI.
			// if this happens for any value, we throw a critical error.
			logger::trace("Values read in!");
			switch (RaceToggle) {
			case -1:
				failureswitch = true;
				break;
			default:
				logger::trace("One");
				morf->usingRace = RaceToggle;
			}

			switch (FactionToggle) {
			case -1:
				failureswitch = true;
				break;
			default:
				logger::trace("Two");
				morf->usingFaction = FactionToggle;
			}

			switch (ClothesToggle) {
			case -1:
				failureswitch = true;
				break;
			default:
				logger::trace("Three");
				morf->usingClothes = ClothesToggle;
			}

			switch (LazyInstall) {
			case -1:
				failureswitch = true;
				break;
			default:
				logger::trace("Four");
				morf->lazyInstall = LazyInstall;
				// if the lazy install was toggled on, just read the presets from the
				// calientetools master folder. Otherwise, read from our mod folder.
				if (morf->lazyInstall) {
					presetPath = "Data\\CalienteTools\\BodySlide\\SliderPresets";
				} else {
					presetPath = "Data\\autoBody\\Presets";
				}
			}

			switch (FactionPriority) {
			case -1:
				failureswitch = true;
				break;
			default:
				logger::trace("Five");
				morf->factionPriority = FactionPriority;
			}

			switch (WeightBias) {
			case -1:
				failureswitch = true;
				break;
			case 1:
				morf->biasamount = atoi(configINI.GetValue("Options", "bWeightBiasAmount"));
			default:
				logger::trace("Six");
				morf->enableWeightBias = WeightBias;
			}

			switch (weightOptions) {
			case -1:
				failureswitch = true;
				break;

			default:
				logger::trace("Seven");
				morf->weightOptions = weightOptions;
			}

			switch (weightSpecific) {
			case -1:
				failureswitch = true;
				break;
			default:
				logger::trace("Eight.");
				morf->weightSpecific = weightSpecific;
			}

			if (failureswitch)
				logger::critical("The INI is missing values! Please redownload it.");

			return presetPath;
		}

		// opens the morph ini, reads it, and ingests the data.
		void CheckMorphConfig()
		{
			CSimpleIniA morphsINI;
			using namespace Presets;

			auto container = PresetContainer::GetInstance();

			// reading NOMorphs.ini
			morphsINI.SetUnicode();
			morphsINI.SetMultiKey();

			// load the file. Optional separate name to avoid collision with bodygen inis.
			SI_Error rc = morphsINI.LoadFile("./Data/autoBody/Config/autoMorphs.ini");
			if (rc < 0) {
				rc = morphsINI.LoadFile("./Data/autoBody/Config/morphs.ini");
				if (rc < 0) {
					logger::critical("Bodygen INI not found! Please make sure it is in /Data/autoBody/Config/");
				}
			}

			std::list<CSimpleIniA::Entry> keylist;
			// acquire all the keys in the file
			morphsINI.GetAllKeys("", keylist);

			// for each key, parse it into race, sex, faction, or character categories and
			// pass those values to their respective lists.
			for (std::list<CSimpleIniA::Entry>::iterator keylistIter = keylist.begin(); keylistIter != keylist.end(); keylistIter++) {
				bool multipreset = false;
				bool notempty = false;
				bool faction = false;
				bool race = false;

				std::vector<categorizedList>* characterCategorySet{ new std::vector<categorizedList> };
				std::vector<categorizedList>* raceCategorySet{ new std::vector<categorizedList> };
				std::vector<categorizedList>* factionCategorySet{ new std::vector<categorizedList> };
				std::vector<bodypreset>* masterSet{ new std::vector<bodypreset> };

				std::string name = keylistIter->pItem;

				// trim the module name off the end of specific character lists
				size_t eraseamount = name.find(".esm|");

				if (eraseamount != -1) {
					name.erase(0, eraseamount + 4);
				}
				eraseamount = name.find(".esp|");
				if (eraseamount != -1) {
					name.erase(0, eraseamount + 4);
				}

				categorizedList parsedlist;

				std::string value = morphsINI.GetValue("", keylistIter->pItem);
				std::string preset;
				bodypreset bodysliders;

				// identifying discriminators

				// sex identification
				if (name.find("Female") != -1) {
					logger::trace("Female preset identified in morphs.ini");
					characterCategorySet = &container->femaleCharacterCategorySet;
					raceCategorySet = &container->femaleRaceCategorySet;
					factionCategorySet = &container->femaleFactionCategorySet;
					masterSet = &container->femaleMasterSet;

					// sex identification
					parsedlist.sex = "Female";
					eraseamount = name.find("Female");
					name.erase(0, eraseamount + 7);

				}

				else if (name.find("Male") != -1) {
					logger::trace("male preset identified in morphs.ini.");
					characterCategorySet = &container->maleCharacterCategorySet;
					raceCategorySet = &container->maleRaceCategorySet;
					factionCategorySet = &container->maleFactionCategorySet;
					masterSet = &container->maleMasterSet;

					parsedlist.sex = "Male";
					eraseamount = name.find("Male");
					name.erase(0, eraseamount + 5);
				} else {
					// if there's no sex value, it must be a character ID
					// trim leading brace
					name.erase(0, 1);

					// begin form ID conversion
					uint32_t hexnumber;
					sscanf_s(name.c_str(), "%x", &hexnumber);
					parsedlist.formID = hexnumber;
					// auto actor = RE::TESObjectREFR::LookupByID<RE::Actor>(hexnumber);
				}

				// if we spot a faction name, fill the faction tag. If we spot a race
				// name, fill the race tag.
				if (name.find("Faction") != -1) {
					logger::trace("Faction is: {}", name);
					parsedlist.faction = name;
					faction = true;

				} else if (name.find("Race") != -1) {
					logger::trace("Race is: {}", name);
					parsedlist.race = name;
					race = true;
				}

				// chop up the presets into substrings
				while (value.find("|") != -1) {
					eraseamount = value.find_first_of("|");
					preset = value.substr(0, eraseamount);
					//logger::trace("attempting to find preset {} in the master list", preset);
					//logger::trace("the master set has {} elements in it, and its first element is {}", masterSet.size(), masterSet.begin()[0].name);
					bodysliders = FindPresetByName(*masterSet, preset);

					// push the preset body in if there is one.
					if (bodysliders != notfound) {
						// logger::trace("Pushing preset into categorizedSet!");
						notempty = true;
						parsedlist.categorizedSet.push_back(bodysliders);
					} else {
						// logger::trace("Preset was not pushed into categorizedSet because it
						// was not found.");
					}

					value.erase(0, eraseamount + 1);
					multipreset = true;
				}
				// if it doesn't activate the loop a single time, there must be only one
				// preset on the key. that's what this check is for.
				if (!multipreset) {
					//logger::trace("Attempting to find preset {} in the master list (singular)", value);
					//logger::trace("the master set has {} elements in it, and its first element is {}", masterSet.size(), masterSet.begin()[0].name);

					bodysliders = FindPresetByName(*masterSet, value);
					if (bodysliders != notfound) {
						logger::trace("Pushing singular preset into categorizedSet!");
						parsedlist.categorizedSet.push_back(bodysliders);
						notempty = true;
					} else {
						// logger::trace("Preset was not pushed into categorizedSet because it
						// was not found.");
					}
					// logger::trace("{}.xml was identified as a preset for {}", value, name);
				}

				parsedlist.formID = 0;
				// to help us tell where the preset belongs

				// safety guard to make sure we can't load presets we don't have the xmls
				// for.
				if (notempty) {
					// if its a character preset, it goes in the character list.
					logger::trace("Beginning pass of parsedList!");
					if (parsedlist.formID != 0) {
						logger::trace("formID {} is being passed into the character list!", name);
						characterCategorySet->push_back(parsedlist);
					}

					// if its a race preset, it goes into the race list.
					else if (race) {
						logger::trace("{} is being passed to the raceSex list!", parsedlist.race);
						raceCategorySet->push_back(parsedlist);
					}

					// if it's a faction preset, it goes into the faction list.
					else if (faction) {
						logger::trace("The faction data is {} elements long", parsedlist.categorizedSet.size());
						logger::trace("{} is being passed to the faction list!", parsedlist.faction);
						factionCategorySet->push_back(parsedlist);
					}
				} else {
					logger::trace("Preset category was empty! We're not adding it.");
				}
			}
		}

		void SetConfigKey(const char* sectionname, const char* keyname, const char* value)
		{
			CSimpleIniA configINI;
			SI_Error rc = configINI.LoadFile("./Data/autoBody/Config/autoBodyConfig.ini");
			if (rc < 0) {
				logger::critical("config not found! shit will break.");
			}
			configINI.SetValue(sectionname, keyname, value);

			// this lets us update the values in morphman after we set the values.
			CheckConfig();
		}
	}  // namespace Parsing

}  // namespace Presets
