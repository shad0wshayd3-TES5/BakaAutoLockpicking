#pragma once

#include "RE/Skyrim.h"
#include "REX/REX/INI.h"
#include "SKSE/SKSE.h"

#include <effolkronium/random.hpp>

using namespace std::literals;

#ifdef SKYRIM_SUPPORT_AE
#	define OFFSET(se, ae) ae
#else
#	define OFFSET(se, ae) se
#endif
