#pragma once

#include "Hooks.h"

namespace Papyrus
{
	class AutoLockNative
	{
	public:
		inline static bool Register(RE::BSScript::IVirtualMachine* a_vm)
		{
			a_vm->RegisterFunction("GetRollModifiers", CLASS_NAME, GetRollModifiers);
			a_vm->RegisterFunction("GetVersion", CLASS_NAME, GetVersion, true);
			a_vm->RegisterFunction("UpdateSettings", CLASS_NAME, UpdateSettings);
			logger::info(FMT_STRING("Registered funcs for class {}"sv), CLASS_NAME);

			return true;
		}

	private:
		static constexpr char CLASS_NAME[] = "AutoLockNative";

		enum
		{
			kVersion = 2
		};

		inline static std::int32_t GetVersion(RE::StaticFunctionTag*)
		{
			return kVersion;
		}

		inline static std::vector<std::string> GetRollModifiers(RE::StaticFunctionTag*)
		{
			return Hooks::AutoLockNative::GetRollModifiers();
		}

		inline static void UpdateSettings(RE::StaticFunctionTag*)
		{
			Settings::MCM::Update();
		}
	};
}
