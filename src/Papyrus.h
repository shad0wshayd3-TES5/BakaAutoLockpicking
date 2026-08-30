#pragma once

#include "Hooks.h"

namespace Papyrus::AutoLockNative
{
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
		a_vm->RegisterFunction("GetVersion"sv, "AutoLockNative"sv, GetVersion, true);
		a_vm->RegisterFunction("GetRollModifiers"sv, "AutoLockNative"sv, GetRollModifiers);
		a_vm->RegisterFunction("UpdateSettings"sv, "AutoLockNative"sv, UpdateSettings);
		REX::INFO("Registered funcs for class AutoLockNative"sv);

		return true;
	}
}
