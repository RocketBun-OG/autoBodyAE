#pragma once
#pragma system_header
#ifdef __cplusplus
#pragma warning(push)

#include <RE/Skyrim.h>
#include <REL/Relocation.h>
#include <SKEE.h>
#include <SKSE/SKSE.h>
#include <SimpleIni.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <rapidxml/rapidxml.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <string>
#include <vector>

#pragma warning(pop)

using namespace std::literals;

namespace logger = SKSE::log;

namespace util {
using SKSE::stl::report_and_fail;
}

#define DLLEXPORT __declspec(dllexport)

// fuck you microsoft and your shit-ass macro.
#ifdef GetObject
#undef GetObject
#define GetObject RE::BGSDefaultObjectManager::GetObject
#endif
#ifdef GetForm
#undef GetForm
#endif
#include "Plugin.h"

#endif
