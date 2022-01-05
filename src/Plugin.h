#pragma once

#include <string_view>

namespace Plugin
{
	using namespace std::literals;

	inline constexpr REL::Version VERSION{
		// clang-format off
		1u,
		0u,
		0u,
		// clang-format on
	};

	inline constexpr auto NAME = "autoBodyAE"sv;
}  // namespace Plugin
