#pragma once

namespace Forms
{
	static constexpr std::string_view PLUGIN_NAME{ "AutoLockpicking.esp"sv };

	static RE::BGSListForm* AutoLock_Perks_Base{ nullptr };
	static RE::BGSListForm* AutoLock_Perks_Locksmith{ nullptr };
	static RE::BGSListForm* AutoLock_Perks_Unbreakable{ nullptr };
	static RE::BGSListForm* AutoLock_Perks_WaxKey{ nullptr };
	static RE::BGSListForm* AutoLock_Items_Lockpick{ nullptr };
	static RE::BGSListForm* AutoLock_Items_SkeletonKey{ nullptr };

	static bool Register()
	{
		if (auto TESDataHandler = RE::TESDataHandler::GetSingleton())
		{
			if (TESDataHandler->GetLoadedLightModIndex(PLUGIN_NAME))
			{
				AutoLock_Perks_Base =
					TESDataHandler->LookupForm<RE::BGSListForm>(0x802, PLUGIN_NAME);
				AutoLock_Perks_Locksmith =
					TESDataHandler->LookupForm<RE::BGSListForm>(0x803, PLUGIN_NAME);
				AutoLock_Perks_Unbreakable =
					TESDataHandler->LookupForm<RE::BGSListForm>(0x804, PLUGIN_NAME);
				AutoLock_Perks_WaxKey =
					TESDataHandler->LookupForm<RE::BGSListForm>(0x805, PLUGIN_NAME);
				AutoLock_Items_Lockpick =
					TESDataHandler->LookupForm<RE::BGSListForm>(0x806, PLUGIN_NAME);
				AutoLock_Items_SkeletonKey =
					TESDataHandler->LookupForm<RE::BGSListForm>(0x807, PLUGIN_NAME);
			}
		}

		return true;
	}
}
