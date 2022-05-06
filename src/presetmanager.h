#pragma once
#include <libs.h>
using namespace rapidxml;
namespace Presets
{
	// raw slider struct
	struct slider
	{
		float max;
		float min;
		std::string name = "";
	};

	// weight-adjusted slider struct
	struct flattenedslider
	{
		float value;
		std::string name = "";
	};

	// raw body struct
	struct bodypreset
	{
		std::vector<slider> sliderlist;
		std::string name = "";

		// for comparing presets in CheckINIs
		bool operator!=(const bodypreset& c) { return (!c.name._Equal(name)); }
	};

	// weight-adjusted body struct
	struct completedbody
	{
		std::vector<flattenedslider> nodelist;
		std::string presetname = "";
		int weight = 0;
	};

	struct categorizedList
	{
		std::vector<bodypreset> categorizedSet;
		std::string sex = "Female";
		std::string race = "Nord";
		std::string faction = "";
		std::string owningMod = "";
		bool characterflag = false;
		uint32_t formID = 0;
		bool ignore = false;
	};

	class PresetContainer
	{
	public:
		// master lists of presets from autoBodyAE/presets
		std::vector<bodypreset> femaleMasterSet;
		std::vector<bodypreset> maleMasterSet;

		std::vector<bodypreset> femaleBackupSet;

		// master lists of all user-defined categories
		std::vector<categorizedList> femaleRaceCategorySet;
		std::vector<categorizedList> femaleFactionCategorySet;

		std::vector<categorizedList> maleRaceCategorySet;
		std::vector<categorizedList> maleFactionCategorySet;

		std::vector<categorizedList> characterCategorySet;

		std::vector<Presets::slider> clothingsliders;
		Presets::bodypreset clothingUnprocessed{ clothingsliders, "Clothing" };

		PresetContainer() = default;

		// just like morphman, this uses a singleton pattern.
		static PresetContainer* GetInstance();
	};

	extern bodypreset notfound;
	extern std::vector<slider> nullsliders;
	extern std::vector<bodypreset> vecNotFound;

	bodypreset FindPresetByName(std::vector<bodypreset> searchable, std::string name);

	// selects a totally random preset from the list.
	bodypreset FindRandomPreset(std::vector<bodypreset> list);

	bool isInINI(RE::Actor* a_actor, bool racesex = false, bool faction = false);

	RE::BSEventNotifyControl HandleGeneration(RE::Actor* a_actor, bool genOverride = false);

	std::vector<bodypreset> findActorInINI(RE::Actor* a_actor);
	std::vector<bodypreset> findFactionInINI(RE::Actor* a_actor);
	std::vector<bodypreset> findRaceSexInINI(RE::Actor* a_actor);
	// std::vector<bodypreset> findFactionInINI(RE::Actor *a_actor);
	namespace Parsing
	{
		void PrintPreset(bodypreset preset);

		void PrintPresetList(std::vector<bodypreset> setofsets);

		// takes the list as a pointer
		void ParsePreset(std::string filename, std::vector<bodypreset>* femalelist, std::vector<bodypreset>* malelist);

		// finds all the xmls in a target folder, and parses them into an xml vector.
		void ParseAllInFolder(std::string path, std::vector<bodypreset>* femalelist, std::vector<bodypreset>* malelist);

		std::string CheckConfig();

		void CheckMorphConfig();

		void SetConfigKey(const char* sectionname, const char* keyname, const char* value);

	}  // namespace Parsing

}  // namespace Presets
