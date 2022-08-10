#pragma once

#include "Forms.h"
#include "Settings.h"

namespace Hooks
{
	class AutoLockNative
	{
	public:
		inline static void InstallHooks()
		{
			hkPlayerHasKey<OFFSET(17485, 17887), OFFSET(0x0BA, 0x0BA)>::Install();
			hkPlayerHasKey<OFFSET(17521, 17922), OFFSET(0x223, 0x239)>::Install();
			hkTryUnlockObject<OFFSET(17485, 17887), OFFSET(0x1AC, 0x18A)>::Install();
			hkTryUnlockObject<OFFSET(17521, 17922), OFFSET(0x313, 0x32A)>::Install();
		}

		inline static std::vector<std::string> GetRollModifiers()
		{
			Settings::MCM::Update();

			auto Skill = GetRollModifier_Skill();
			auto Perks = GetRollModifier_Perks();
			auto LCKSM = GetRollModifier_LCKSM();
			auto Bonus = GetRollModifier_Bonus();
			auto Total = Skill + Perks + LCKSM + Bonus;

			return std::vector<std::string>{
				fmt::format(Skill >= 0 ? "+{:d}"sv : "{:d}"sv, Skill),
				fmt::format(Perks >= 0 ? "+{:d}"sv : "{:d}"sv, Perks),
				fmt::format(LCKSM >= 0 ? "+{:d}"sv : "{:d}"sv, LCKSM),
				fmt::format(Bonus >= 0 ? "+{:d}"sv : "{:d}"sv, Bonus),
				fmt::format(Total >= 0 ? "+{:d}"sv : "{:d}"sv, Total)
			};
		}

	private:
		using Random = effolkronium::random_thread_local;

		template<std::int32_t a_ID, std::int32_t a_OF>
		class hkPlayerHasKey
		{
		public:
			inline static void Install()
			{
				REL::Relocation<std::uintptr_t> target{ REL::ID(a_ID), a_OF };
				auto& trampoline = SKSE::GetTrampoline();
				_PlayerHasKey = trampoline.write_call<5>(target.address(), PlayerHasKey);
			}

		private:
			inline static bool PlayerHasKey(void* a_this, void* a_arg2, std::uint32_t a_arg3, std::int32_t a_arg4, std::int32_t a_arg5, bool& a_arg6)
			{
				if (Settings::MCM::General::bModEnabled && Settings::MCM::General::bIgnoreHasKey)
				{
					return false;
				}
				
				return _PlayerHasKey(a_this, a_arg2, a_arg3, a_arg4, a_arg5, a_arg6);
			}

			inline static REL::Relocation<decltype(PlayerHasKey)> _PlayerHasKey;
		};
		
		template<std::int32_t a_ID, std::int32_t a_OF>
		class hkTryUnlockObject
		{
		public:
			inline static void Install()
			{
				REL::Relocation<std::uintptr_t> target{ REL::ID(a_ID), a_OF };
				auto& trampoline = SKSE::GetTrampoline();
				_TryUnlockObject = trampoline.write_call<5>(target.address(), TryUnlockObject);
			}

		private:
			inline static void TryUnlockObject(RE::TESObjectREFR* a_refr)
			{
				if (Settings::MCM::m_FirstRun)
				{
					Settings::MCM::Update();
				}

				if (Settings::MCM::General::bModEnabled)
				{
					return TryUnlockObjectImpl(a_refr);
				}

				return _TryUnlockObject(a_refr);
			}

			inline static REL::Relocation<decltype(TryUnlockObject)> _TryUnlockObject;
		};

		inline static void TryUnlockObjectImpl(RE::TESObjectREFR* a_refr)
		{
			if (!a_refr)
			{
				return;
			}

			if (!a_refr->GetLock())
			{
				return;
			}

			auto LockKey = a_refr->GetLock()->key;
			auto LockLevel = a_refr->GetLockLevel();
			switch (LockLevel)
			{
				case RE::LOCK_LEVEL::kVeryEasy:
				case RE::LOCK_LEVEL::kEasy:
				case RE::LOCK_LEVEL::kAverage:
				case RE::LOCK_LEVEL::kHard:
				case RE::LOCK_LEVEL::kVeryHard:
					break;
				default:
					return;
			}

			auto PlayerCharacter = RE::PlayerCharacter::GetSingleton();
			if (!PlayerCharacter)
			{
				return;
			}

			auto GameSettingColl = RE::GameSettingCollection::GetSingleton();
			if (!GameSettingColl)
			{
				return;
			}

			if (!PlayerHasLockpicks())
			{
				if (auto setting = GameSettingColl->GetSetting("sOutOfLockpicks"))
				{
					RE::DebugNotification(setting->GetString());
				}

				if (PlayerHasItem(LockKey))
				{
					UnlockObject(a_refr);
					HandleUnlockNotification(LockKey);
				}

				return;
			}

			auto LockVal = GetLockDifficultyClass(LockLevel);
			auto RollMin = Settings::MCM::Rolls::iPlayerDiceMin;
			auto RollMax = std::max(RollMin, Settings::MCM::Rolls::iPlayerDiceMax);
			auto RollMod = GetRollModifier();
			auto RollVal = Random::get<std::int32_t>(RollMin, RollMax);

			if (Settings::MCM::General::bShowRollResults)
			{
				auto result = fmt::format(
					Settings::MCM::General::sShowRollResults,
					LockVal,
					RollVal,
					RollMod);
				logger::info(result.c_str());
				RE::DebugNotification(result.c_str());
			}

			if (Settings::MCM::Rolls::bCriticalFailure)
			{
				if (RollMin == RollVal)
				{
					RE::DebugNotification(Settings::MCM::General::sCriticalFailure.c_str());
					PlayerCharacter->currentProcess->KnockExplosion(
						PlayerCharacter,
						PlayerCharacter->data.location,
						5.0f);
				}
			}

			if (Settings::MCM::Rolls::bCriticalSuccess)
			{
				if (RollMax == RollVal)
				{
					RE::DebugNotification(Settings::MCM::General::sCriticalSuccess.c_str());
					HandleExperience(RE::LOCK_LEVEL::kVeryHard);
				}
			}

			RollVal += RollMod;
			if (RollVal >= LockVal)
			{
				UnlockObject(a_refr);
				HandleExperience(LockLevel);
				HandleWaxKey(LockKey);

				if (Settings::MCM::General::bDetectionEventSuccess)
				{
					PlayerCharacter->currentProcess->SetActorsDetectionEvent(
						PlayerCharacter,
						a_refr->data.location,
						Settings::MCM::General::iDetectionEventSuccessLevel,
						a_refr);
				}
			}
			else
			{
				HandleLockpickRemoval();
				HandleExperience(RE::LOCK_LEVEL::kUnlocked);

				if (Settings::MCM::General::bDetectionEventFailure)
				{
					PlayerCharacter->currentProcess->SetActorsDetectionEvent(
						PlayerCharacter,
						a_refr->data.location,
						Settings::MCM::General::iDetectionEventFailureLevel,
						a_refr);
				}
			}

			if (Settings::MCM::General::bLockpickingCrimeCheck)
			{
				HandleCrime(a_refr);
			}
		}

		inline static std::int32_t GetItemCount(RE::TESForm* a_form)
		{
			auto PlayerCharacter = RE::PlayerCharacter::GetSingleton();
			if (!PlayerCharacter)
			{
				return 0;
			}

			if (a_form)
			{
				if (auto form = a_form->As<RE::TESBoundObject>())
				{
					return PlayerCharacter->GetItemCount(form);
				}
				else if (auto list = a_form->As<RE::BGSListForm>())
				{
					std::int32_t count{ 0 };
					for (auto& iter : list->forms)
					{
						if (auto object = iter->As<RE::TESBoundObject>())
						{
							count += PlayerCharacter->GetItemCount(object);
						}
					}

					return count;
				}
			}

			return 0;
		}

		inline static std::int32_t GetLockDifficultyClass(RE::LOCK_LEVEL a_lockLevel)
		{
			switch (a_lockLevel)
			{
				case RE::LOCK_LEVEL::kEasy:
					return Settings::MCM::Rolls::iDCApprentice;
				case RE::LOCK_LEVEL::kAverage:
					return Settings::MCM::Rolls::iDCAdept;
				case RE::LOCK_LEVEL::kHard:
					return Settings::MCM::Rolls::iDCExpert;
				case RE::LOCK_LEVEL::kVeryHard:
					return Settings::MCM::Rolls::iDCMaster;
				default:
					return Settings::MCM::Rolls::iDCNovice;
			}
		}

		inline static std::int32_t GetRollModifier_Skill()
		{
			auto PlayerCharacter = RE::PlayerCharacter::GetSingleton();
			if (!PlayerCharacter)
			{
				return 0;
			}

			auto SkillMod = PlayerCharacter->GetActorValue(RE::ActorValue::kLockpickingModifier);
			auto SkillLvl = PlayerCharacter->GetActorValue(GetSkillFromIndex());
			auto SkillVal = SkillLvl * (1.0f + SkillMod / 100.0f);

			return static_cast<std::int32_t>(floorf(SkillVal / Settings::MCM::Rolls::iBonusPerSkills));
		}

		inline static std::int32_t GetRollModifier_Perks()
		{
			std::int32_t result{ 0 };
			for (auto& form : Forms::AutoLock_Perks_Base->forms)
			{
				if (PlayerHasPerk(form))
				{
					result += Settings::MCM::Rolls::iBonusPerPerks;
				}
			}

			return result;
		}

		inline static std::int32_t GetRollModifier_LCKSM()
		{
			for (auto& form : Forms::AutoLock_Perks_Locksmith->forms)
			{
				if (PlayerHasPerk(form))
				{
					return Settings::MCM::Rolls::iBonusPerLcksm;
				}
			}

			return 0;
		}

		inline static std::int32_t GetRollModifier_Bonus()
		{
			return Settings::MCM::Rolls::iBonusPerBonus;
		}

		inline static std::int32_t GetRollModifier()
		{
			std::int32_t result{ 0 };
			result += GetRollModifier_Skill();
			result += GetRollModifier_Perks();
			result += GetRollModifier_LCKSM();
			result += GetRollModifier_Bonus();
			return result;
		}

		inline static RE::ActorValue GetSkillFromIndex()
		{
			std::vector<RE::ActorValue> Skills = { RE::ActorValue::kOneHanded,
												   RE::ActorValue::kTwoHanded,
												   RE::ActorValue::kArchery,
												   RE::ActorValue::kBlock,
												   RE::ActorValue::kSmithing,
												   RE::ActorValue::kHeavyArmor,
												   RE::ActorValue::kLightArmor,
												   RE::ActorValue::kPickpocket,
												   RE::ActorValue::kLockpicking,
												   RE::ActorValue::kSneak,
												   RE::ActorValue::kAlchemy,
												   RE::ActorValue::kSpeech,
												   RE::ActorValue::kAlteration,
												   RE::ActorValue::kConjuration,
												   RE::ActorValue::kDestruction,
												   RE::ActorValue::kIllusion,
												   RE::ActorValue::kRestoration,
												   RE::ActorValue::kEnchanting };

			return Skills[Settings::MCM::General::iSkillIndex];
		}

		inline static void HandleActivateUpdate(RE::TESObjectREFR* a_refr)
		{
			RE::GFxValue HUDObject;

			if (auto UI = RE::UI::GetSingleton())
			{
				if (auto HUDMenu = UI->GetMenu<RE::HUDMenu>(); HUDMenu && HUDMenu->uiMovie)
				{
					HUDMenu->uiMovie->GetVariable(std::addressof(HUDObject), "_root.HUDMovieBaseInstance");
				}
			}

			if (HUDObject.IsObject())
			{
				std::array<RE::GFxValue, 10> args;
				args[0] = true;
				args[2] = true;
				args[3] = true;

				RE::BSString name;
				if (a_refr->data.objectReference)
				{
					a_refr->data.objectReference->GetActivateText(a_refr, name);
				}

				args[1] = name.empty() ? "" : name.c_str();
				HUDObject.Invoke("SetCrosshairTarget", args);
			}
		}

		inline static void HandleCrime(RE::TESObjectREFR* a_refr)
		{
			auto PlayerCharacter = RE::PlayerCharacter::GetSingleton();
			if (!PlayerCharacter)
			{
				return;
			}

			auto owner = a_refr->GetOwner();
			if (!owner)
			{
				if (a_refr->GetFormType() != RE::FormType::Door)
				{
					return;
				}

				if (auto ExtraTeleport = a_refr->extraList.GetByType<RE::ExtraTeleport>())
				{
					if (auto ETeleportData = ExtraTeleport->teleportData)
					{
						if (auto LinkedDoorREF = ETeleportData->linkedDoor.get())
						{
							if (auto LinkedCellREF = LinkedDoorREF->GetParentCell())
							{
								owner = LinkedCellREF->GetOwner();
							}
						}
					}
				}
			}

			if (!owner)
			{
				return;
			}

			if (auto ProcessLists = RE::ProcessLists::GetSingleton())
			{
				std::uint32_t LOSCount{ 1 };
				if (ProcessLists->RequestHighestDetectionLevelAgainstActor(PlayerCharacter, LOSCount) > 0)
				{
					auto Crime{ 1.0f };
					RE::BGSEntryPoint::HandleEntryPoint(
						RE::BGSEntryPoint::ENTRY_POINT::kModLockpickingCrimeChance,
						PlayerCharacter,
						a_refr,
						&Crime);

					auto Chance = Random::get<float>(0.0f, 1.0f);
					if (Chance < Crime)
					{
						auto Prison = PlayerCharacter->currentPrisonFaction;
						if (Prison && Prison->crimeData.crimevalues.escapeCrimeGold)
						{
							PlayerCharacter->SetEscaping(true, false);
						}
						else
						{
							PlayerCharacter->TrespassAlarm(a_refr, owner, -1);
						}
					}
				}
			}
		}

		inline static void HandleExperience(RE::LOCK_LEVEL a_lockLevel)
		{
			auto PlayerCharacter = RE::PlayerCharacter::GetSingleton();
			if (!PlayerCharacter)
			{
				return;
			}

			auto GameSettingColl = RE::GameSettingCollection::GetSingleton();
			if (!GameSettingColl)
			{
				return;
			}

			auto LockpickingSkill = GetSkillFromIndex();
			switch (a_lockLevel)
			{
				case RE::LOCK_LEVEL::kVeryEasy:
					if (auto setting = GameSettingColl->GetSetting("fSkillUsageLockPickVeryEasy"))
					{
						PlayerCharacter->AddSkillExperience(LockpickingSkill, setting->GetFloat());
					}
					return;

				case RE::LOCK_LEVEL::kEasy:
					if (auto setting = GameSettingColl->GetSetting("fSkillUsageLockPickEasy"))
					{
						PlayerCharacter->AddSkillExperience(LockpickingSkill, setting->GetFloat());
					}
					return;

				case RE::LOCK_LEVEL::kAverage:
					if (auto setting = GameSettingColl->GetSetting("fSkillUsageLockPickAverage"))
					{
						PlayerCharacter->AddSkillExperience(LockpickingSkill, setting->GetFloat());
					}
					return;

				case RE::LOCK_LEVEL::kHard:
					if (auto setting = GameSettingColl->GetSetting("fSkillUsageLockPickHard"))
					{
						PlayerCharacter->AddSkillExperience(LockpickingSkill, setting->GetFloat());
					}
					return;

				case RE::LOCK_LEVEL::kVeryHard:
					if (auto setting = GameSettingColl->GetSetting("fSkillUsageLockPickVeryHard"))
					{
						PlayerCharacter->AddSkillExperience(LockpickingSkill, setting->GetFloat());
					}
					return;

				default:
					if (auto setting = GameSettingColl->GetSetting("fSkillUsageLockPickBroken"))
					{
						PlayerCharacter->AddSkillExperience(LockpickingSkill, setting->GetFloat());
					}
					return;
			}
		}

		inline static void HandleLockpickRemoval()
		{
			auto PlayerCharacter = RE::PlayerCharacter::GetSingleton();
			if (!PlayerCharacter)
			{
				return;
			}
			
			if (PlayerHasBreakable())
			{
				for (auto& form : Forms::AutoLock_Items_Lockpick->forms)
				{
					if (PlayerHasItem(form))
					{
						RE::PlaySound("UILockpickingPickBreak");
						PlayerCharacter->RemoveItem(
							form->As<RE::TESBoundObject>(),
							1,
							RE::ITEM_REMOVE_REASON::kRemove,
							nullptr,
							nullptr);

						HandleExperience(RE::LOCK_LEVEL::kUnlocked);
						break;
					}
				}
			}
			else
			{
				RE::PlaySound("UILockpickingCylinderTurn");
			}
		}

		inline static void HandleUnlockNotification(RE::TESKey* a_key)
		{
			auto GameSettingColl = RE::GameSettingCollection::GetSingleton();
			if (!GameSettingColl)
			{
				return;
			}

			auto NAME = std::string{ a_key->GetFullName() };
			if (!NAME.empty())
			{
				if (auto setting = GameSettingColl->GetSetting("sOpenWithKey"))
				{
					auto result = std::string{ setting->GetString() };
					if (auto idx = result.find("%s"sv); idx != std::string::npos)
					{
						result.replace(idx, 2, "{:s}"sv);
						result = fmt::format(result, NAME);

						RE::DebugNotification(result.c_str());
					}
				}
			}
		}
		
		inline static void HandleWaxKey(RE::TESKey* a_key)
		{
			auto PlayerCharacter = RE::PlayerCharacter::GetSingleton();
			if (!PlayerCharacter)
			{
				return;
			}

			auto GameSettingColl = RE::GameSettingCollection::GetSingleton();
			if (!GameSettingColl)
			{
				return;
			}

			if (a_key && PlayerHasWaxKey())
			{
				if (!PlayerHasItem(a_key))
				{
					PlayerCharacter->AddObjectToContainer(a_key, nullptr, 1, nullptr);

					auto NAME = std::string{ a_key->GetFullName() };
					auto SNDR = a_key->pickupSound;
					
					if (!NAME.empty())
					{
						if (auto setting = GameSettingColl->GetSetting("sAddItemtoInventory"))
						{
							auto result = fmt::format(
								FMT_STRING("{:s} {:s}"sv),
								NAME.c_str(),
								setting->GetString());

							RE::DebugNotification(
								result.c_str(),
								SNDR ? SNDR->GetFormEditorID() : "ITMKeyUpSD");

							return;
						}
					}

					RE::PlaySound(
						SNDR ? SNDR->GetFormEditorID() : "ITMKeyUpSD");
				}
			}
		}

		inline static bool PlayerHasBreakable()
		{
			if (Settings::MCM::General::bUnbreakableLockpicks)
			{
				return false;
			}

			if (PlayerHasItem(Forms::AutoLock_Items_SkeletonKey))
			{
				return false;
			}

			for (auto& form : Forms::AutoLock_Perks_Unbreakable->forms)
			{
				if (PlayerHasPerk(form))
				{
					return false;
				}
			}

			return true;
		}

		inline static bool PlayerHasItem(RE::TESForm* a_item)
		{
			if (!a_item)
			{
				return false;
			}

			return GetItemCount(a_item) > 0;
		}

		inline static bool PlayerHasLockpicks()
		{
			if (PlayerHasItem(Forms::AutoLock_Items_Lockpick))
			{
				return true;
			}

			if (PlayerHasItem(Forms::AutoLock_Items_SkeletonKey))
			{
				return true;
			}

			return false;
		}
		
		inline static bool PlayerHasPerk(RE::TESForm* a_perk)
		{
			auto PlayerCharacter = RE::PlayerCharacter::GetSingleton();
			if (!PlayerCharacter)
			{
				return false;
			}

			if (auto perk = a_perk->As<RE::BGSPerk>())
			{
				if (PlayerCharacter->HasPerk(perk))
				{
					return true;
				}
			}

			return false;
		}

		inline static bool PlayerHasWaxKey()
		{
			for (auto& form : Forms::AutoLock_Perks_WaxKey->forms)
			{
				if (PlayerHasPerk(form))
				{
					return true;
				}
			}

			return false;
		}

		inline static void UnlockObject(RE::TESObjectREFR* a_refr)
		{
			a_refr->GetLock()->SetLocked(false);
			RE::PlaySound("UILockpickingUnlock");
			HandleActivateUpdate(a_refr);

			if (Settings::MCM::General::bActivateAfterPick)
			{
				auto PlayerCharacter = RE::PlayerCharacter::GetSingleton();
				if (!PlayerCharacter)
				{
					return;
				}

				a_refr->ActivateRef(PlayerCharacter, 0, nullptr, 0, false);
			}
		}
	};
}
