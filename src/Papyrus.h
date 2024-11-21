#pragma once

#include "Hooks.h"

namespace Papyrus::AutoLockNative
{
	static constexpr std::string_view CLASS_NAME{ "AutoLockNative"sv };

	enum
	{
		kVersion = 2
	};

	static std::int32_t GetVersion(RE::StaticFunctionTag*)
	{
		return kVersion;
	}

	static std::vector<std::string> GetRollModifiers(RE::StaticFunctionTag*)
	{
		return Hooks::AutoLockNative::GetRollModifiers();
	}

	static void UpdateSettings(RE::StaticFunctionTag*)
	{
		Settings::MCM::Update(false);
	}

	static bool Register(RE::BSScript::IVirtualMachine* a_vm)
	{
		a_vm->RegisterFunction("GetVersion"sv, CLASS_NAME, GetVersion, true);
		a_vm->RegisterFunction("GetRollModifiers"sv, CLASS_NAME, GetRollModifiers);
		a_vm->RegisterFunction("UpdateSettings"sv, CLASS_NAME, UpdateSettings);
		SKSE::log::info("Registered funcs for class {:s}"sv, CLASS_NAME);

		return true;
	}
}
