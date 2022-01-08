#pragma once

#include <string_view>

namespace Plugin
{
	using namespace std::literals;

	inline constexpr REL::Version VERSION{
		// clang-format off
		1u,
		2u,
		2u,
		// clang-format on
	};

	inline constexpr auto NAME = "AutoBodyAE"sv;
}  // namespace Plugin
