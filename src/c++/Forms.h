#pragma once

namespace Forms
{
	static constexpr char PLUGIN_NAME[] = "AutoLockpicking.esp";

	inline RE::BGSListForm* AutoLock_Perks_Base{ nullptr };
	inline RE::BGSListForm* AutoLock_Perks_Locksmith{ nullptr };
	inline RE::BGSListForm* AutoLock_Perks_Unbreakable{ nullptr };
	inline RE::BGSListForm* AutoLock_Perks_WaxKey{ nullptr };
	inline RE::BGSListForm* AutoLock_Items_Lockpick{ nullptr };
	inline RE::BGSListForm* AutoLock_Items_SkeletonKey{ nullptr };

	inline bool Register()
	{
		if (auto TESDataHandler = RE::TESDataHandler::GetSingleton(); TESDataHandler) {
			if (TESDataHandler->GetLoadedLightModIndex(PLUGIN_NAME)) {
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
