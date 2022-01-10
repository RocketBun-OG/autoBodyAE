#pragma once

const std::vector<std::string> DefaultSliders = { "Breasts", "BreastsSmall", "NippleDistance", "NippleSize", "ButtCrack", "Butt", "ButtSmall", "Legs", "Arms", "ShoulderWidth" };

const std::unordered_map<std::string, int32_t> DefaultBreastScores = {
	{ "DoubleMelon", 30 },
	{ "BreastsFantasy", 28 },
	{ "Breasts", 25 },
	{ "BreastsNewSH", 45 },
	{ "BreastsFlatness", -30 },
	{ "BreastsFlatness2", -30 },
	{ "BreastsSmall", -15 },
	{ "BreastsSmall2", -15 },
	{ "BreastsGone", -25 },
	{ "BreastPerkiness", -10 },
	{ "OldBaseShape", 33 },
	{ "7B Upper", 50 },
	{ "VanillaSSEHi", 20 },
	{ "VanillaSSELo", -10 },
};

const std::unordered_map<std::string, int32_t> DefaultButtScores = { { "AppleCheeks", 30 }, { "BigButt", 20 }, { "ChubbyButt", 20 }, { "Butt", 15 }, { "ButtSaggy_v2", 5 }, { "ButtShape2", 5 }, { "RoundAss", 7 }, { "7B Lower", 15 },
	{ "ButtSmall", -5 }, { "ButtPressed_v2", -10 } };

const std::unordered_map<std::string, int32_t> DefaultWaistScores = { { "BigTorso", 20 }, { "ChubbyWaist", 30 }, { "Waist", 12 }, { "WideWaistLine", 15 }, { "MuscleMoreAbs_v2", 15 }, { "Belly", 2 }, { "BigBelly", 10 }, { "PregnancyBelly", 50 },
	{ "7B Lower", 25 }, { "Vanilla SSE High", 25 } };