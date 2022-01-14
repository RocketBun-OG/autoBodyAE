
#include <Inverts.h>
#include <morphman.h>
#include <presetmanager.h>

namespace Presets
{
	// today i learned about LNK2005.
	bodypreset notfound{};
	std::vector<slider> nullsliders;
	std::vector<bodypreset> vecNotFound;

	//followes a singleton format. You want access to the lists, use GetInstance().
	PresetContainer* PresetContainer::GetInstance()
	{
		static PresetContainer instance;
		return std::addressof(instance);
	}

	bodypreset FindPresetByName(std::vector<bodypreset> searchable, std::string name)
	{
		logger::trace("entered findpresetbyname");
		logger::trace("the searchable set has {} elements in it.", searchable.size());
		for (bodypreset searchitem : searchable) {
			//logger::trace("the items name is {} and the search target is {}", searchitem.name, name);
			if (searchitem.name == name) {
				logger::trace("{} was found!", searchitem.name);
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

	//master "can we do shit with this actor" function. Searches through the categories ingested with
	//morphs.ini and returns true if there is a match found. Configurable to search just actorIDs, race, or faction -- or any of those in combination.
	bool isInINI(RE::Actor* a_actor, bool racesex, bool faction)
	{
		bool found = false;

		auto container = Presets::PresetContainer::GetInstance();

		//must be pointers or shit goes wrong.
		std::vector<categorizedList>* racelist{ new std::vector<categorizedList> };
		std::vector<categorizedList>* faclist{ new std::vector<categorizedList> };
		std::vector<categorizedList>* charlist{ new std::vector<categorizedList> };

		auto sexint = a_actor->GetActorBase()->GetSex();

		charlist = &container->characterCategorySet;

		//first we check to see if the actor is male or female, to search through the right lists.
		std::string sex;
		if (sexint == 1) {
			//logger::trace("Sex is female.");
			racelist = &container->femaleRaceCategorySet;
			faclist = &container->femaleFactionCategorySet;

			sex = "Female";

		} else {
			//logger::trace("Sex is male");
			racelist = &container->maleRaceCategorySet;
			faclist = &container->maleFactionCategorySet;

			sex = "Male";
		}

		//for each category in the racelist, compare it against the actor's race. If there's a match, return true.
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

		//for each formID in the actor preset list, check it against the actor's ID. If there's a match, return true.
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
		bool factionapplied = false;
		auto presetcontainer = Presets::PresetContainer::GetInstance();
		auto defaultobj = RE::BGSDefaultObjectManager::GetSingleton();
		auto keywordNPC = defaultobj->GetObject<RE::BGSKeyword>(RE::DEFAULT_OBJECT::kKeywordNPC);

		if (actor->HasKeyword(keywordNPC)) {
			auto morphman = Bodygen::Morphman::GetInstance();
			logger::trace("NPC {} is being examined.", actor->GetName());
			//runs the generation algorithm if they havent been generated, or if the mod has been commanded to override their previous generation.
			//Only things that override right now are the regen spell and the UIExtensions morph menu.
			if (!morphman->IsGenned(actor) || genOverride) {
				//logger::trace("We've entered the inner loop of handlegen.");
				auto player = RE::PlayerCharacter::GetSingleton();
				playerID = player->formID;
				// if we're processing the player load, ignore it and mark the player as processed so we don't spam the debug log.
				if (actor->formID == playerID) {
					morphman->morphInterface->SetMorph(actor, "autobody_processed", "autoBody_processed", 1.0f);
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
					factionapplied = true;
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
				//guard to prevent actor from being regenerated if they've already had a faction body stuck on them.
				if (!factionapplied) {
					if (sexint == 1) {
						morphman->ApplyPreset(actor, presetcontainer->femaleMasterSet);
					} else {
						morphman->ApplyPreset(actor, presetcontainer->maleMasterSet);
					}
				}
			} else {
				logger::trace("Actor has already had a preset applied. Skipping preset application.");
			}
		}
		return RE::BSEventNotifyControl::kContinue;
	}

	//locates and returns the preset list for a specific actor, if one exists.
	std::vector<bodypreset> findActorInINI(RE::Actor* a_actor)
	{
		logger::trace("Entered findActorInINI");
		uint32_t actorID = a_actor->GetActorBase()->GetFormID();

		//no need for a sex check since we're directly comparing formIDs.
		auto charlist = Presets::PresetContainer::GetInstance()->characterCategorySet;

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

	//can you guess what this one does? :)
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

	//everything to do with managing external files.
	namespace Parsing
	{
		// utility functions for morphs.ini parsing
		//https://stackoverflow.com/questions/12966957/
		std::vector<std::string> explode(std::string const& s, char delim)
		{
			std::vector<std::string> result;
			std::istringstream iss(s);

			for (std::string token; std::getline(iss, token, delim);) { result.push_back(std::move(token)); }
			return result;
		}

		bool contains(std::vector<std::string> input, std::vector<std::string> comparisonitems)
		{
			for (std::string i : input) {
				for (std::string j : comparisonitems) {
					if (i.find(j) != -1) {
						return true;
					}
				}
			}
			return false;
		}

		std::string seek(std::vector<std::string> input, std::vector<std::string> comparisonitems)
		{
			for (std::string i : input) {
				for (std::string j : comparisonitems) {
					if (i.find(j) != -1) {
						return i;
					}
				}
			}
			return "";
		}

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
			std::string* bodytype{ new std::string };
			std::vector<std::string>* presetgroups{ new std::vector<std::string> };

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
			for (xml_node<>* preset_node = root_node->first_node("Preset"); preset_node; preset_node = preset_node->next_sibling("Preset")) {
				std::vector<slider>* sliderset{ new std::vector<slider> };

				// we need this for the set of sets, to identify them.
				*presetname = preset_node->first_attribute()->value();

				//fill the group vector with groups to identify the sex of the preset.
				for (xml_node<>* cat_node = preset_node->first_node("Group"); cat_node; cat_node = cat_node->next_sibling("Group")) {
					//logger::trace("Adding {} to group list!", cat_node->first_attribute()->value());
					presetgroups->push_back(cat_node->first_attribute()->value());
				}

				logger::trace("preset name being loaded is {}", *presetname);
				std::vector<std::string> discardkeywords{ "Cloth", "cloth", "Outfit", "outfit", "NeverNude", "Bikini", "Feet", "Hands", "OUTFIT", "push", "Push", "Cleavage", "Armor", "Bra" };
				// discard clothed presets.
				if (Presets::Parsing::contains({ *presetname }, discardkeywords)) {
					logger::trace("Clothed preset found. Discarding.");
					continue;
				}

				//for finding if we need to check for inverted sliders
				bool UUNP = false;
				*bodytype = preset_node->last_attribute()->value();

				if (bodytype->find("UNP") != -1) {
					logger::trace("UUNP preset found. Now inverting sliders.");
					UUNP = true;
				}

				//logger::trace("{} has entered the outer for loop", *presetname);
				std::string* slidername{ new std::string };
				std::string* previousslidername{ new std::string("") };

				std::string* size{ new std::string("") };
				std::string* lastSize{ new std::string("") };
				float defaultvalue = 0;
				float* sizevalue{ new float{ 0 } };
				float* lastsizevalue{ new float{ 0 } };
				// go through the sliders
				for (xml_node<>* slider_node = preset_node->first_node("SetSlider"); slider_node; slider_node = slider_node->next_sibling("SetSlider")) {
					// store the current values of this node
					*slidername = slider_node->first_attribute()->value();
					*size = slider_node->first_attribute("size")->value();
					auto printable = *slidername;
					//logger::trace("The slider being looked at is {} and it is {}", printable, *size);
					// convert the size to a morphable value (those are -1 to 1.)
					*sizevalue = std::stoi(slider_node->first_attribute("value")->value()) / 100.0f;
					//logger::trace("The converted value of that slider is {}", *sizevalue);
					//if we detect that a preset is UUNP based, invert the sliders.
					bool inverted = false;
					if (UUNP) {
						for (std::string slider : DefaultSliders) {
							if (*slidername == slider) {
								logger::trace("UUNP backwards slider found! Inverting...");
								*sizevalue = 1.0f - *sizevalue;
								inverted = true;
								break;
							}
						}
					}
					// if a pair is found, push it into the sliderset vector as a full struct.
					if (*slidername == *previousslidername) {
						//logger::trace("{} is a paired slider and is being pushed back with a pair of values of {} and {}", *slidername, *lastsizevalue, *sizevalue);

						sliderset->push_back({ *lastsizevalue, *sizevalue, *slidername });
					}

					// if a pair is not found, evaluate which half it is and then push it as a
					// full struct with default values where they belong.
					else if (*slidername != *previousslidername) {
						if (!slider_node->next_sibling() || slider_node->next_sibling()->first_attribute()->value() != *slidername) {
							//logger::trace("slider {} is a singlet", *slidername);
							if (inverted) {
								defaultvalue -= 1.0f;
							}

							if (*size == "big") {
								//logger::trace("It's a biggun");
								sliderset->push_back({ *sizevalue, defaultvalue, *slidername });
							} else {
								//logger::trace("It's a smallun;");
								sliderset->push_back({ defaultvalue, *sizevalue, *slidername });
							}
							//yet another inversion check for UUNP support.
						}
					}

					*previousslidername = *slidername;
					*lastSize = *size;
					*lastsizevalue = *sizevalue;
					//logger::trace("At the end of pushback we have a slider name of {} and a value of {}", *slidername, *sizevalue);
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

				// push the preset into the master list, then erase the sliderset to startagain.
				for (std::string item : *presetgroups) {
					if (item.find("CBBE") != -1 || item.find("3BBB") != -1 || item.find("CBAdvanced") != -1 || (item.find("UNP") != -1)) {
						logger::info("{} has just been ingested into the female master preset list.", *presetname);
						//logger::trace("Female preset found!");
						//PrintPreset(bodypreset{ *sliderset, *presetname });
						femalelist->push_back(bodypreset{ *sliderset, *presetname });
						break;
					} else if (item._Equal("HIMBO")) {
						logger::info("{} has just been ingested into the male master preset list.", *presetname);
						//logger::trace("Male preset found!");
						malelist->push_back(bodypreset{ *sliderset, *presetname });
						break;
					} else {
						logger::info("{} is not assigned to any supported preset groups! It will not be loaded.", *presetname);
					}
				}
				delete sliderset;
			}
			//  clean up memory
			delete presetgroups;
			presetgroups = nullptr;

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
				return "FAILED";
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

				morf->usingRace = RaceToggle;
			}

			switch (FactionToggle) {
			case -1:
				failureswitch = true;
				break;
			default:

				morf->usingFaction = FactionToggle;
			}

			switch (ClothesToggle) {
			case -1:
				failureswitch = true;
				break;
			default:

				morf->usingClothes = ClothesToggle;
			}

			switch (LazyInstall) {
			case -1:
				failureswitch = true;
				break;
			default:
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
				morf->factionPriority = FactionPriority;
			}

			switch (WeightBias) {
			case -1:
				failureswitch = true;
				break;
			case 1:
				morf->biasamount = atoi(configINI.GetValue("Options", "bWeightBiasAmount"));
			default:
				morf->enableWeightBias = WeightBias;
			}

			switch (weightOptions) {
			case -1:
				failureswitch = true;
				break;

			default:
				morf->weightOptions = weightOptions;
			}

			switch (weightSpecific) {
			case -1:
				failureswitch = true;
				break;
			default:
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

			// reading Morphs.ini
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

			bool maleMasterReplaced = false;
			bool femaleMasterReplaced = false;

			//first, see if there are any master preset lines in there.
			for (std::list<CSimpleIniA::Entry>::iterator keylistIter = keylist.begin(); keylistIter != keylist.end(); keylistIter++) {
				if (maleMasterReplaced && femaleMasterReplaced) {
					logger::trace("Both masters defined in the config! breaking out of the loop.");
					break;
				}

				std::string name = keylistIter->pItem;
				std::string value = morphsINI.GetValue("", keylistIter->pItem);

				std::vector<Presets::bodypreset>* oldmaster{ new std::vector<Presets::bodypreset> };
				std::vector<Presets::bodypreset> newbodies;

				std::vector<std::string> masterbodies = explode(value, '|');
				if (masterbodies.size() < 1) {
					logger::error("ERROR: This line has no entries! Ignoring...");
					continue;
				}

				auto categories = explode(name, '|');
				if (contains(categories, { "All" }) && categories.size() == 2) {
					logger::trace("Found a master replacer!");
					//if we find a definition of one of the master sets in the ini, rebuild the master set. Then, push it into the preset container's master set.
					if (contains(categories, { "Female" })) {
						femaleMasterReplaced = true;
						oldmaster = &container->femaleMasterSet;

					} else if (contains(categories, { "Male" })) {
						maleMasterReplaced = true;
						oldmaster = &container->maleMasterSet;
					}

					//logger::trace("the master set is {} bodies large", oldmaster->size());
					bool notempty = false;
					//for each body in the master set
					for (int i = 0; i < oldmaster->size(); ++i) {
						//logger::trace("{} is the preset we are currently looking at, with an i of {}.", oldmaster->at(i).name, i);
						bool match = false;
						//for each body in the config
						for (std::string masterbodyname : masterbodies) {
							//if the config body matches, push it into our new master list.
							if (masterbodyname == oldmaster->at(i).name) {
								notempty = true;
								//logger::trace("{} was found in both the config and the actual master set! It will not be trimmed.", masterbodyname);
								match = true;
								bool duplicate = false;

								//avoids inserting duplicates into the master list.
								for (Presets::bodypreset alreadypushed : newbodies) {
									if (alreadypushed.name == masterbodyname) {
										duplicate = true;
										break;
									}
								}
								if (!duplicate) {
									newbodies.push_back(oldmaster->at(i));
								}
							}
						}
					}

					if (notempty) {
						//log the event and then push our new master list into the container.
						if (contains(categories, { "Female" })) {
							logger::info("Female master list has been redefined in morphs.ini. New size is {}", newbodies.size());
						} else if (contains(categories, { "Male" })) {
							logger::info("Male master list has been redefined in morphs.ini. New size is {}", newbodies.size());
						}
						*oldmaster = newbodies;
					}
				}
			}

			for (int i = 0; i < container->femaleMasterSet.size(); ++i) { logger::trace("{} is contained in the master list.", container->femaleMasterSet[i].name); }
			// for each key, parse it into race, sex, faction, or character categories and
			// pass the associated bodies to their respective lists.
			for (std::list<CSimpleIniA::Entry>::iterator keylistIter = keylist.begin(); keylistIter != keylist.end(); keylistIter++) {
				bool notempty = false;
				bool character = false;
				bool race = false;
				bool faction = false;

				std::vector<categorizedList>* characterCategorySet{ new std::vector<categorizedList> };
				std::vector<categorizedList>* raceCategorySet{ new std::vector<categorizedList> };
				std::vector<categorizedList>* factionCategorySet{ new std::vector<categorizedList> };
				std::vector<bodypreset>* masterSet{ new std::vector<bodypreset> };

				characterCategorySet = &container->characterCategorySet;

				std::string name = keylistIter->pItem;

				size_t eraseamount;

				categorizedList parsedlist;

				std::string value = morphsINI.GetValue("", keylistIter->pItem);
				std::string preset;
				bodypreset bodysliders;

				//nifty delimiting hack
				auto categories = explode(name, '|');

				//now the parsing starts. First, see if this line in the INI is male, female, or neither. If it's neither, it's a character.

				// sex identification
				if (contains(categories, { "Female" })) {
					logger::trace("Female preset identified in morphs.ini");
					raceCategorySet = &container->femaleRaceCategorySet;
					factionCategorySet = &container->femaleFactionCategorySet;
					masterSet = &container->femaleMasterSet;
					parsedlist.sex = "Female";

				}

				else if (contains(categories, { "Male" })) {
					logger::trace("male preset identified in morphs.ini.");
					raceCategorySet = &container->maleRaceCategorySet;
					factionCategorySet = &container->maleFactionCategorySet;
					masterSet = &container->maleMasterSet;

					parsedlist.sex = "Male";

				} else {
					// if there's no sex value, it must be a character ID
					character = true;

					RE::Actor* IDOwner;

					std::string stringID;
					std::string owningMod;

					//if we have a character, dissect their owning mod and their ID. write it down.
					for (std::string item : categories) {
						if (!contains({ item }, { ".esp", ".esm" })) {
							stringID = item;
							logger::trace("{} is the character ID", stringID);
						} else if (contains({ item }, { ".esp", ".esm" })) {
							logger::trace("{} is the owning mod.", item);
							owningMod = item;
						}
					}

					//then convert the hexadecimal string in the INI into a uint32_t, as this is how skyrim sees these IDs internally.
					uint32_t hexnumber;
					sscanf_s(stringID.c_str(), "%x", &hexnumber);

					auto datahandler = RE::TESDataHandler::GetSingleton();

					//LookupForm gives us a TESForm, which we can then use to get their full length formID. This is good.

					auto actorform = datahandler->LookupForm(hexnumber, owningMod);
					if (!actorform) {
						logger::error("{} is not a valid key!", name);
						continue;
					}
					//We have to use this full-length ID in order to identify them with IsInINI and FindActorInINI.
					auto ID = actorform->GetFormID();
					logger::trace("{} is their ID", ID);

					//retrieve the NPC via the truncated form ID and their owning mod.
					RE::TESNPC* matchedNPC = datahandler->LookupForm<RE::TESNPC>(hexnumber, owningMod);

					//if that NPC is female, align masterSet with the female master set. Otherwise, align it with the male master set.
					if (matchedNPC->GetSex() == true) {
						masterSet = &container->femaleMasterSet;
					} else {
						masterSet = &container->maleMasterSet;
					}
					//logger::trace("Sex is Female? {}", matchedNPC->GetSex());

					//now we pass the data to the list struct. owningMod is likely unused for now, but hey, it's cool right?
					parsedlist.formID = ID;
					parsedlist.owningMod = owningMod;
				}  //end character-detection branch

				// if its got "race" in the name, it's a race.
				if (contains(categories, { "Race" })) {
					parsedlist.race = seek(categories, { "Race" });
					logger::trace("Race is {}", parsedlist.race);
					race = true;

				}
				//if its not a race, and not a character, then its either a faction, a master set, or malformed. This block checks for all three.
				else if (!character) {
					if (categories.size() == 3) {
						//faction is always the third element in the list when formatted correctly
						faction = true;
						parsedlist.faction = categories[2];
						logger::trace("Faction is {}", categories[2]);

					}
					//ignore any master redefinition lines, as they've already been handled.
					else if (categories.size() == 2 && contains(categories, { "All" }) && contains(categories, { "Female", "Male" })) {
						logger::trace("Master redefinition was found.");
						continue;
					} else {
						logger::info("ERROR: The category {} is not formatted correctly. It will be ignored.", name);
						continue;
					}
				}

				//now it's time to handle the right side of the = sign: the bodies.
				//first, separate the bodies using the | delimeter.
				std::vector<std::string> bodylist = explode(value, '|');

				//then, we sift through that newly created vector and ensure that each one has a corresponding entry in the master list. If it doesn't, then we ignore it.
				for (std::string item : bodylist) {
					logger::trace("body we're lookin at is {}", item);
					bodysliders = FindPresetByName(*masterSet, item);
					if (bodysliders != notfound) {
						notempty = true;
						logger::trace("found a body match!");
						parsedlist.categorizedSet.push_back(bodysliders);

						logger::trace("{}.xml was identified as a preset for {}", item, name);
					}
				}

				// safety guard to make sure we can't load presets we don't have the xmls for.
				if (notempty) {
					// if its a character preset, it goes in the character list.
					logger::trace("Beginning pass of parsedList!");
					if (character) {
						logger::trace("formID {} is being passed into the character list!", parsedlist.formID);
						characterCategorySet->push_back(parsedlist);
					}

					// if its a race preset, it goes into the race list.
					else if (race) {
						logger::trace("{} is being passed to the raceSex list!", parsedlist.race);
						raceCategorySet->push_back(parsedlist);
					}

					// if it's a faction preset, it goes into the faction list.
					else if (faction) {
						logger::trace("{} is being passed to the faction list!", parsedlist.faction);
						factionCategorySet->push_back(parsedlist);
					}
					//otherwise, it doesn't match any category, so the user must have formatted it wrong. In this case, that line of the ini is ignored.
					else {
						logger::trace("pass failed due to malformed INI.");
					}
				}
				//or, if there were no entries in bodylist that matched a preset in the master list, that line of the ini is ignored.
				else {
					logger::trace("Preset category was empty! We're not adding it.");
				}
			}
		}

		//set an arbitrary key in the INI.
		void SetConfigKey(const char* sectionname, const char* keyname, const char* value)
		{
			CSimpleIniA configINI;
			SI_Error rc = configINI.LoadFile("./Data/autoBody/Config/autoBodyConfig.ini");
			if (rc < 0) {
				logger::critical("config not found! AutoBody will probably break.");
			}
			configINI.SetValue(sectionname, keyname, value);

			// this lets us update the values in morphman after we set the values.
			CheckConfig();
		}
	}  // namespace Parsing

}  // namespace Presets
