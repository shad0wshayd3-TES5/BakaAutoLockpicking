#pragma once

namespace Forms
{
	static RE::BGSListForm* AutoLock_Perks_Base{ nullptr };
	static RE::BGSListForm* AutoLock_Perks_Locksmith{ nullptr };
	static RE::BGSListForm* AutoLock_Perks_Unbreakable{ nullptr };
	static RE::BGSListForm* AutoLock_Perks_WaxKey{ nullptr };
	static RE::BGSListForm* AutoLock_Items_Lockpick{ nullptr };
	static RE::BGSListForm* AutoLock_Items_SkeletonKey{ nullptr };

	static void Register()
	{
		if (auto TESDataHandler = RE::TESDataHandler::GetSingleton())
		{
			if (TESDataHandler->GetLoadedLightModIndex("AutoLockpicking.esp"sv))
			{
				AutoLock_Perks_Base =
					TESDataHandler->LookupForm<RE::BGSListForm>(0x802, "AutoLockpicking.esp"sv);
				AutoLock_Perks_Locksmith =
					TESDataHandler->LookupForm<RE::BGSListForm>(0x803, "AutoLockpicking.esp"sv);
				AutoLock_Perks_Unbreakable =
					TESDataHandler->LookupForm<RE::BGSListForm>(0x804, "AutoLockpicking.esp"sv);
				AutoLock_Perks_WaxKey =
					TESDataHandler->LookupForm<RE::BGSListForm>(0x805, "AutoLockpicking.esp"sv);
				AutoLock_Items_Lockpick =
					TESDataHandler->LookupForm<RE::BGSListForm>(0x806, "AutoLockpicking.esp"sv);
				AutoLock_Items_SkeletonKey =
					TESDataHandler->LookupForm<RE::BGSListForm>(0x807, "AutoLockpicking.esp"sv);
			}
		}
	}
}
