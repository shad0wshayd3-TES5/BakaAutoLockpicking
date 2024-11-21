#pragma once

#include "Forms.h"
#include "Settings.h"

namespace Hooks
{
	class AutoLockNative
	{
	public:
		static void InstallHooks()
		{
			hkPlayerHasKey<OFFSET(17485, 17887), OFFSET(0x0BA, 0x0BA)>::Install();
			hkPlayerHasKey<OFFSET(17521, 17922), OFFSET(0x223, 0x239)>::Install();
			hkTryUnlockObject<OFFSET(17485, 17887), OFFSET(0x1AC, 0x18A)>::Install();
			hkTryUnlockObject<OFFSET(17521, 17922), OFFSET(0x313, 0x32A)>::Install();
		}

		static std::vector<std::string> GetRollModifiers()
		{
			Settings::MCM::Update();

			auto Skill = GetRollModifier_Skill();
			auto Perks = GetRollModifier_Perks();
			auto LCKSM = GetRollModifier_LCKSM();
			auto Bonus = GetRollModifier_Bonus();
			auto Total = Skill + Perks + LCKSM + Bonus;

			return std::vector<std::string>{
				Skill >= 0 ? std::format("+{:d}"sv, Skill) : std::format("{:d}"sv, Skill),
				Perks >= 0 ? std::format("+{:d}"sv, Perks) : std::format("{:d}"sv, Perks),
				LCKSM >= 0 ? std::format("+{:d}"sv, LCKSM) : std::format("{:d}"sv, LCKSM),
				Bonus >= 0 ? std::format("+{:d}"sv, Bonus) : std::format("{:d}"sv, Bonus),
				Total >= 0 ? std::format("+{:d}"sv, Total) : std::format("{:d}"sv, Total)
			};
		}

	private:
		template<std::int32_t a_ID, std::int32_t a_OF>
		class hkPlayerHasKey
		{
		public:
			static void Install()
			{
				REL::Relocation<std::uintptr_t> target{ REL::ID(a_ID), a_OF };
				auto& trampoline = SKSE::GetTrampoline();
				_PlayerHasKey = trampoline.write_call<5>(target.address(), PlayerHasKey);
			}

		private:
			static bool PlayerHasKey(void* a_this, void* a_arg2, std::uint32_t a_arg3, std::int32_t a_arg4, std::int32_t a_arg5, bool& a_arg6)
			{
				if (Settings::MCM::General::bModEnabled.GetValue() && Settings::MCM::General::bIgnoreHasKey.GetValue())
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
			static void Install()
			{
				REL::Relocation<std::uintptr_t> target{ REL::ID(a_ID), a_OF };
				auto& trampoline = SKSE::GetTrampoline();
				_TryUnlockObject = trampoline.write_call<5>(target.address(), TryUnlockObject);
			}

		private:
			static void TryUnlockObject(RE::TESObjectREFR* a_refr)
			{
				if (Settings::MCM::General::bModEnabled.GetValue())
				{
					return TryUnlockObjectImpl(a_refr);
				}

				return _TryUnlockObject(a_refr);
			}

			inline static REL::Relocation<decltype(TryUnlockObject)> _TryUnlockObject;
		};

		static void* FinalizeUnlock(RE::TESObjectREFR* a_refr)
		{
			using func_t = decltype(&FinalizeUnlock);
			REL::Relocation<func_t> func{ RELOCATION_ID(19110, 19512) };
			return func(a_refr);
		}

		static void TryUnlockObjectImpl(RE::TESObjectREFR* a_refr)
		{
			if (!a_refr)
			{
				return;
			}

			if (!a_refr->GetLock())
			{
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

				case RE::LOCK_LEVEL::kRequiresKey:
					{
						if (PlayerHasItem(LockKey))
						{
							UnlockObject(a_refr);
							HandleUnlockNotification(LockKey);
							return;
						}

						if (auto setting = GameSettingColl->GetSetting("sImpossibleLock"))
						{
							RE::DebugNotification(setting->GetString());
						}

						return;
					}

				default:
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
			auto RollMin = Settings::MCM::Rolls::iPlayerDiceMin.GetValue();
			auto RollMax = std::max(RollMin, Settings::MCM::Rolls::iPlayerDiceMax.GetValue());
			auto RollMod = GetRollModifier();
			auto RollVal = effolkronium::random_thread_local::get<std::int32_t>(RollMin, RollMax);

			if (Settings::MCM::General::bShowRollResults.GetValue())
			{
				auto result = std::vformat(Settings::MCM::General::sShowRollResults, std::make_format_args(LockVal, RollVal, RollMod));
				SKSE::log::info("{:s}"sv, result);
				RE::DebugNotification(result.c_str());
			}

			if (Settings::MCM::Rolls::bCriticalFailure.GetValue())
			{
				if (RollMin == RollVal)
				{
					RE::DebugNotification(Settings::MCM::General::sCriticalFailure.c_str());
					PlayerCharacter->currentProcess->KnockExplosion(PlayerCharacter,PlayerCharacter->data.location,5.0f);
				}
			}

			if (Settings::MCM::Rolls::bCriticalSuccess.GetValue())
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

				if (Settings::MCM::General::bDetectionEventSuccess.GetValue())
				{
					PlayerCharacter->currentProcess->SetActorsDetectionEvent(PlayerCharacter,a_refr->data.location,Settings::MCM::General::iDetectionEventSuccessLevel.GetValue(),a_refr);
				}
			}
			else
			{
				HandleLockpickRemoval();
				HandleExperience(RE::LOCK_LEVEL::kUnlocked);

				if (Settings::MCM::General::bDetectionEventFailure.GetValue())
				{
					PlayerCharacter->currentProcess->SetActorsDetectionEvent(PlayerCharacter,a_refr->data.location,Settings::MCM::General::iDetectionEventFailureLevel.GetValue(),a_refr);
				}
			}

			if (Settings::MCM::General::bLockpickingCrimeCheck.GetValue())
			{
				HandleCrime(a_refr);
			}
		}

		static std::int32_t GetItemCount(RE::TESForm* a_form)
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

		static std::int32_t GetLockDifficultyClass(RE::LOCK_LEVEL a_lockLevel)
		{
			switch (a_lockLevel)
			{
				case RE::LOCK_LEVEL::kEasy:
					return Settings::MCM::Rolls::iDCApprentice.GetValue();
				case RE::LOCK_LEVEL::kAverage:
					return Settings::MCM::Rolls::iDCAdept.GetValue();
				case RE::LOCK_LEVEL::kHard:
					return Settings::MCM::Rolls::iDCExpert.GetValue();
				case RE::LOCK_LEVEL::kVeryHard:
					return Settings::MCM::Rolls::iDCMaster.GetValue();
				default:
					return Settings::MCM::Rolls::iDCNovice.GetValue();
			}
		}

		static std::int32_t GetRollModifier_Skill()
		{
			auto PlayerCharacter = RE::PlayerCharacter::GetSingleton();
			if (!PlayerCharacter)
			{
				return 0;
			}

			auto SkillMod = PlayerCharacter->GetActorValue(RE::ActorValue::kLockpickingModifier);
			auto SkillPwr = PlayerCharacter->GetActorValue(RE::ActorValue::kLockpickingPowerModifier);
			auto SkillLvl = PlayerCharacter->GetActorValue(GetSkillFromIndex());
			auto SkillVal = SkillLvl * (1.0f + ((SkillMod + SkillPwr) / 100.0f));

			return static_cast<std::int32_t>(floorf(SkillVal / Settings::MCM::Rolls::iBonusPerSkills.GetValue()));
		}

		static std::int32_t GetRollModifier_Perks()
		{
			std::int32_t result{ 0 };
			for (auto& form : Forms::AutoLock_Perks_Base->forms)
			{
				if (PlayerHasPerk(form))
				{
					result += Settings::MCM::Rolls::iBonusPerPerks.GetValue();
				}
			}

			return result;
		}

		static std::int32_t GetRollModifier_LCKSM()
		{
			for (auto& form : Forms::AutoLock_Perks_Locksmith->forms)
			{
				if (PlayerHasPerk(form))
				{
					return Settings::MCM::Rolls::iBonusPerLcksm.GetValue();
				}
			}

			return 0;
		}

		static std::int32_t GetRollModifier_Bonus()
		{
			return Settings::MCM::Rolls::iBonusPerBonus.GetValue();
		}

		static std::int32_t GetRollModifier()
		{
			std::int32_t result{ 0 };
			result += GetRollModifier_Skill();
			result += GetRollModifier_Perks();
			result += GetRollModifier_LCKSM();
			result += GetRollModifier_Bonus();
			return result;
		}

		static RE::ActorValue GetSkillFromIndex()
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

			return Skills[Settings::MCM::General::iSkillIndex.GetValue()];
		}

		static void HandleActivateUpdate(RE::TESObjectREFR* a_refr)
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

		static void HandleCrime(RE::TESObjectREFR* a_refr)
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
					RE::BGSEntryPoint::HandleEntryPoint(RE::BGSEntryPoint::ENTRY_POINT::kModLockpickingCrimeChance,PlayerCharacter,a_refr,&Crime);

					auto Chance = effolkronium::random_thread_local::get<float>(0.0f, 1.0f);
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

		static void HandleExperience(RE::LOCK_LEVEL a_lockLevel)
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

		static void HandleLockpickRemoval()
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
						PlayerCharacter->RemoveItem(form->As<RE::TESBoundObject>(),1,RE::ITEM_REMOVE_REASON::kRemove,nullptr,nullptr);
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

		static void HandleUnlockNotification(RE::TESKey* a_key)
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
					if (!result.empty())
					{
						result = std::vformat(result, std::make_format_args(NAME));
						RE::DebugNotification(result.c_str());
					}
				}
			}
		}

		static void HandleWaxKey(RE::TESKey* a_key)
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
							auto result = std::format("{} {}"sv, NAME, setting->GetString());
							RE::DebugNotification(result.c_str(),SNDR ? SNDR->GetFormEditorID() : "ITMKeyUpSD");
							return;
						}
					}

					RE::PlaySound(SNDR ? SNDR->GetFormEditorID() : "ITMKeyUpSD");
				}
			}
		}

		static bool PlayerHasBreakable()
		{
			if (Settings::MCM::General::bUnbreakableLockpicks.GetValue())
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

		static bool PlayerHasItem(RE::TESForm* a_item)
		{
			if (!a_item)
			{
				return false;
			}

			return GetItemCount(a_item) > 0;
		}

		static bool PlayerHasLockpicks()
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

		static bool PlayerHasPerk(RE::TESForm* a_perk)
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

		static bool PlayerHasWaxKey()
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

		static void UnlockObject(RE::TESObjectREFR* a_refr)
		{
			a_refr->GetLock()->SetLocked(false);
			FinalizeUnlock(a_refr);

			RE::PlaySound("UILockpickingUnlock");
			HandleActivateUpdate(a_refr);

			if (Settings::MCM::General::bActivateAfterPick.GetValue())
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
