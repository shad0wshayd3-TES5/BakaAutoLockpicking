#pragma once

#include "Forms.h"
#include "Settings.h"

namespace Hooks
{
	class detail
	{
	public:
		class Player
		{
		public:
			static auto GetPerkCount(RE::BGSListForm* a_formList)
			{
				std::int32_t result{ 0 };
				if (auto player = RE::PlayerCharacter::GetSingleton();
					player && a_formList)
				{
					a_formList->ForEachForm(
						[&](RE::TESForm* a_form)
						{
							if (auto perk = a_form->As<RE::BGSPerk>();
								perk && player->HasPerk(perk))
							{
								result++;
							}

							return RE::BSContainer::ForEachResult::kContinue;
						});
				}

				return result;
			}

			static auto GetValue(RE::ActorValue a_value)
			{
				if (auto player = RE::PlayerCharacter::GetSingleton())
				{
					return player->GetActorValue(a_value);
				}

				return 0.0f;
			}

			static bool HasPerk(RE::BGSListForm* a_formList)
			{
				auto result{ false };
				if (auto player = RE::PlayerCharacter::GetSingleton();
					player && a_formList)
				{
					a_formList->ForEachForm(
						[&](RE::TESForm* a_form)
						{
							if (auto perk = a_form->As<RE::BGSPerk>();
								perk && player->HasPerk(perk))
							{
								result = true;
								return RE::BSContainer::ForEachResult::kStop;
							}

							return RE::BSContainer::ForEachResult::kContinue;
						});
				}

				return result;
			}

			static bool HasPerk(RE::BGSPerk* a_perk)
			{
				if (auto player = RE::PlayerCharacter::GetSingleton();
					player && a_perk)
				{
					return player->HasPerk(a_perk);
				}

				return false;
			}

			static bool HasObject(RE::BGSListForm* a_formList)
			{
				auto result{ false };
				if (auto player = RE::PlayerCharacter::GetSingleton();
					player && a_formList)
				{
					a_formList->ForEachForm(
						[&](RE::TESForm* a_form)
						{
							if (auto object = a_form->As<RE::TESBoundObject>();
								object && player->GetItemCount(object))
							{
								result = true;
								return RE::BSContainer::ForEachResult::kStop;
							}

							return RE::BSContainer::ForEachResult::kContinue;
						});
				}

				return result;
			}

			static bool HasObject(RE::TESBoundObject* a_object)
			{
				if (auto player = RE::PlayerCharacter::GetSingleton();
					player && a_object)
				{
					return player->GetItemCount(a_object);
				}

				return false;
			}

			static bool HasBreakable()
			{
				if (Settings::MCM::General::bUnbreakableLockpicks)
					return false;
				if (HasObject(Forms::AutoLock_Items_SkeletonKey))
					return false;
				if (HasPerk(Forms::AutoLock_Perks_Unbreakable))
					return false;
				return true;
			}

			static bool HasLockpicks()
			{
				if (HasObject(Forms::AutoLock_Items_Lockpick))
					return true;
				if (HasObject(Forms::AutoLock_Items_SkeletonKey))
					return true;
				return false;
			}

			static bool HasWaxKey()
			{
				return HasPerk(Forms::AutoLock_Perks_WaxKey);
			}
		};

		static void ShowMessage(std::string_view a_setting, const char* a_format = nullptr, const char* a_sound = nullptr)
		{
			auto settings = RE::GameSettingCollection::GetSingleton();
			if (!settings)
				return;

			auto setting = settings->GetSetting(a_setting.data());
			if (!setting)
				return;

			auto message = setting->GetString();
			if (REX::STR::IS_EMPTY(message))
				return;

			if (a_format)
			{
				auto vformat = std::vformat(message, std::make_format_args(a_format));
				RE::SendHUDMessage::ShowHUDMessage(vformat.data(), a_sound);
			}
			else
			{
				RE::SendHUDMessage::ShowHUDMessage(message, a_sound);
			}
		}
	};

	class AutoLockNative
	{
	public:
		static std::vector<std::string> GetRollModifiers()
		{
			Settings::MCM::Update(false);

			std::int32_t stat = GetRollModStat();
			std::int32_t perk = GetRollModPerk();
			std::int32_t lksm = GetRollModLKSM();
			std::int32_t xtra = Settings::MCM::Rolls::iBonusPerBonus;
			std::int32_t mods = stat + perk + lksm + xtra;

			return std::vector<std::string>{
				stat >= 0 ? std::format("+{:d}"sv, stat) : std::format("{:d}"sv, stat),
				perk >= 0 ? std::format("+{:d}"sv, perk) : std::format("{:d}"sv, perk),
				lksm >= 0 ? std::format("+{:d}"sv, lksm) : std::format("{:d}"sv, lksm),
				xtra >= 0 ? std::format("+{:d}"sv, xtra) : std::format("{:d}"sv, xtra),
				mods >= 0 ? std::format("+{:d}"sv, mods) : std::format("{:d}"sv, mods)
			};
		}

	private:
		class hkPlayerHasKey
		{
		private:
			static bool PlayerHasKey(void* a_this, void* a_arg2, std::uint32_t a_arg3, std::int32_t a_arg4, std::int32_t a_arg5, bool& a_arg6)
			{
				if (Settings::MCM::General::bModEnabled && Settings::MCM::General::bIgnoreHasKey)
					return false;
				return _PlayerHasKey0(a_this, a_arg2, a_arg3, a_arg4, a_arg5, a_arg6);
			}

			inline static REL::THook _PlayerHasKey0{ REL::ID(17887), 0x0BA, PlayerHasKey };
			inline static REL::THook _PlayerHasKey1{ REL::ID(17922), 0x239, PlayerHasKey };
		};

		class hkTryUnlockObject
		{
		private:
			static void TryUnlockObject(RE::TESObjectREFR* a_refr)
			{
				if (Settings::MCM::General::bModEnabled)
					return TryUnlockObjectImpl(a_refr);
				return _TryUnlockObject0(a_refr);
			}

			inline static REL::THook _TryUnlockObject0{ REL::ID(17887), 0x18A, TryUnlockObject };
			inline static REL::THook _TryUnlockObject1{ REL::ID(17922), 0x32A, TryUnlockObject };
		};

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

			auto player = RE::PlayerCharacter::GetSingleton();
			if (!player)
			{
				return;
			}

			auto lockKey = a_refr->GetLock()->key;
			auto lockLvl = a_refr->GetLockLevel();
			switch (lockLvl)
			{
			case RE::LOCK_LEVEL::kVeryEasy:
			case RE::LOCK_LEVEL::kEasy:
			case RE::LOCK_LEVEL::kAverage:
			case RE::LOCK_LEVEL::kHard:
			case RE::LOCK_LEVEL::kVeryHard:
				break;

			case RE::LOCK_LEVEL::kRequiresKey:
			{
				if (detail::Player::HasObject(lockKey))
				{
					UnlockObject(a_refr, false);
					detail::ShowMessage("sOpenWithKey"sv, lockKey->GetFullName());
					return;
				}

				detail::ShowMessage("sImpossibleLock"sv);
				return;
			}

			default:
				return;
			}

			if (!detail::Player::HasLockpicks())
			{
				detail::ShowMessage("sOutOfLockpicks"sv);
				if (detail::Player::HasObject(lockKey))
				{
					UnlockObject(a_refr, false);
					detail::ShowMessage("sOpenWithKey"sv, lockKey->GetFullName());
				}

				return;
			}

			auto lockVal = GetLockDifficultyClass(lockLvl);
			auto rollMin = std::max<std::int32_t>(1, Settings::MCM::Rolls::iPlayerDiceMin);
			auto rollMax = std::max<std::int32_t>(rollMin, Settings::MCM::Rolls::iPlayerDiceMax);
			auto rollMod = GetRollMod();
			auto rollVal = GetRollRNG(rollMin, rollMax);

			if (Settings::MCM::General::bShowRollResults)
			{
				auto result = std::vformat(Settings::MCM::Runtime::sShowRollResults, std::make_format_args(lockVal, rollVal, rollMod));
				REX::INFO("{:s}"sv, result);
				RE::SendHUDMessage::ShowHUDMessage(result.data());
			}

			if (Settings::MCM::Rolls::bCriticalFailure &&
				rollMin == rollVal)
			{
				RE::SendHUDMessage::ShowHUDMessage(Settings::MCM::Runtime::sCriticalFailure.data());
				player->currentProcess->KnockExplosion(player, player->data.location, 5.0f);
			}

			if (Settings::MCM::Rolls::bCriticalSuccess &&
				rollMax == rollVal)
			{
				RE::SendHUDMessage::ShowHUDMessage(Settings::MCM::Runtime::sCriticalSuccess.data());
				HandleExperience(RE::LOCK_LEVEL::kVeryHard);
			}

			rollVal += rollMod;
			if (rollVal >= lockVal)
			{
				UnlockObject(a_refr, true);
				HandleExperience(lockLvl);
				HandleWaxKey(lockKey);

				if (Settings::MCM::General::bDetectionEventSuccess)
					HandleDetection(a_refr, Settings::MCM::General::iDetectionEventSuccessLevel);
			}
			else
			{
				HandleLockpickRemoval();
				HandleExperience(RE::LOCK_LEVEL::kUnlocked);

				if (Settings::MCM::General::bDetectionEventFailure)
					HandleDetection(a_refr, Settings::MCM::General::iDetectionEventFailureLevel);
			}
		}

		static std::int32_t GetLockDifficultyClass(RE::LOCK_LEVEL a_lockLevel)
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

		static std::int32_t GetRollModStat()
		{
			auto lvl = detail::Player::GetValue(GetSkillFromIndex());
			auto mod = detail::Player::GetValue(RE::ActorValue::kLockpickingModifier);
			auto pwr = detail::Player::GetValue(RE::ActorValue::kLockpickingPowerModifier);
			auto val = lvl * (1.0f + ((mod + pwr) / 100.0f));
			return static_cast<std::int32_t>(floorf(val / Settings::MCM::Rolls::iBonusPerSkills));
		}

		static std::int32_t GetRollModPerk()
		{
			auto value = detail::Player::GetPerkCount(Forms::AutoLock_Perks_Base);
			return static_cast<std::int32_t>(Settings::MCM::Rolls::iBonusPerPerks * value);
		}

		static std::int32_t GetRollModLKSM()
		{
			auto value = detail::Player::GetPerkCount(Forms::AutoLock_Perks_Locksmith);
			return static_cast<std::int32_t>(Settings::MCM::Rolls::iBonusPerLcksm * value);
		}

		static std::int32_t GetRollMod()
		{
			auto result{ 0 };
			result += GetRollModStat();
			result += GetRollModPerk();
			result += GetRollModLKSM();
			result += Settings::MCM::Rolls::iBonusPerBonus;
			return result;
		}

		static std::int32_t GetRollRNG(std::int32_t a_min, std::int32_t a_max)
		{
			static REX::RNG::I32 rng;
			return rng.Generate(a_min, a_max);
		}

		static RE::ActorValue GetSkillFromIndex()
		{
			std::vector<RE::ActorValue> skill = {
				RE::ActorValue::kOneHanded,
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
				RE::ActorValue::kEnchanting
			};

			return skill[Settings::MCM::General::iSkillIndex];
		}

		static void HandleActivateUpdate(RE::TESObjectREFR* a_refr)
		{
			RE::GFxValue hudObject;
			if (auto ui = RE::UI::GetSingleton())
			{
				if (auto hud = ui->GetMenu<RE::HUDMenu>();
					hud && hud->uiMovie)
				{
					hud->uiMovie->GetVariable(&hudObject, "_root.HUDMovieBaseInstance");
				}
			}

			if (hudObject.IsObject())
			{
				std::array<RE::GFxValue, 10> args;
				args[0] = true;
				args[2] = true;
				args[3] = true;

				RE::BSString name;
				if (a_refr->data.objectReference)
					a_refr->data.objectReference->GetActivateText(a_refr, name);

				args[1] = name.empty() ? "" : name.c_str();
				hudObject.Invoke("SetCrosshairTarget", args);
			}
		}

		static void HandleCrime(RE::TESObjectREFR* a_refr)
		{
			auto player = RE::PlayerCharacter::GetSingleton();
			if (!player)
				return;

			auto owner = a_refr->GetOwner();
			if (!owner)
			{
				if (a_refr->GetFormType() != RE::FormType::Door)
					return;

				if (auto xtra = a_refr->extraList.GetByType<RE::ExtraTeleport>())
				{
					if (auto data = xtra->teleportData)
					{
						if (auto door = data->linkedDoor.get())
						{
							if (auto cell = door->GetParentCell())
							{
								owner = cell->GetOwner();
							}
						}
					}
				}
			}

			if (!owner)
				return;

			if (auto processLists = RE::ProcessLists::GetSingleton())
			{
				std::uint32_t count{ 1 };
				if (processLists->RequestHighestDetectionLevelAgainstActor(player, count))
				{
					auto crime{ 1.0f };
					RE::BGSEntryPoint::HandleEntryPoint(RE::BGSEntryPoint::ENTRY_POINT::kModLockpickingCrimeChance, player, a_refr, &crime);

					static REX::RNG::F32 rng;
					if (rng.Generate(0.0f, 1.0f) < crime)
					{
						auto prison = player->currentPrisonFaction;
						if (prison && prison->crimeData.crimevalues.escapeCrimeGold)
						{
							player->SetEscaping(true, false);
						}
						else
						{
							player->TrespassAlarm(a_refr, owner, -1);
							detail::ShowMessage("sLockpickingCaught"sv);
						}
					}
				}
			}
		}

		static void HandleDetection(RE::TESObjectREFR* a_refr, std::int32_t a_value)
		{
			if (a_refr && a_value > 0)
			{
				if (auto player = RE::PlayerCharacter::GetSingleton();
					player && player->currentProcess)
				{
					player->currentProcess->SetActorsDetectionEvent(player, a_refr->data.location, a_value, a_refr);
				}
			}
		}

		static void HandleExperience(RE::LOCK_LEVEL a_lockLevel)
		{
			if (auto player = RE::PlayerCharacter::GetSingleton())
			{
				if (auto settings = RE::GameSettingCollection::GetSingleton())
				{
					switch (a_lockLevel)
					{
					case RE::LOCK_LEVEL::kVeryEasy:
						if (auto setting = settings->GetSetting("fSkillUsageLockPickVeryEasy"))
							player->AddSkillExperience(GetSkillFromIndex(), setting->GetFloat());
						break;
					case RE::LOCK_LEVEL::kEasy:
						if (auto setting = settings->GetSetting("fSkillUsageLockPickEasy"))
							player->AddSkillExperience(GetSkillFromIndex(), setting->GetFloat());
						break;
					case RE::LOCK_LEVEL::kAverage:
						if (auto setting = settings->GetSetting("fSkillUsageLockPickAverage"))
							player->AddSkillExperience(GetSkillFromIndex(), setting->GetFloat());
						break;
					case RE::LOCK_LEVEL::kHard:
						if (auto setting = settings->GetSetting("fSkillUsageLockPickHard"))
							player->AddSkillExperience(GetSkillFromIndex(), setting->GetFloat());
						break;
					case RE::LOCK_LEVEL::kVeryHard:
						if (auto setting = settings->GetSetting("fSkillUsageLockPickVeryHard"))
							player->AddSkillExperience(GetSkillFromIndex(), setting->GetFloat());
						break;
					default:
						if (auto setting = settings->GetSetting("fSkillUsageLockPickBroken"))
							player->AddSkillExperience(GetSkillFromIndex(), setting->GetFloat());
						break;
					}
				}
			}
		}

		static void HandleLockpickRemoval()
		{
			if (!detail::Player::HasBreakable())
			{
				RE::PlaySound("UILockpickingCylinderTurn");
				return;
			}

			if (Forms::AutoLock_Items_Lockpick)
			{
				if (Forms::AutoLock_Items_Lockpick->forms.size() == 0 &&
					Forms::AutoLock_Items_Lockpick->scriptAddedFormCount == 0)
				{
					RE::PlaySound("UILockpickingCylinderTurn");
					return;
				}

				Forms::AutoLock_Items_Lockpick->ForEachForm(
					[](RE::TESForm* a_form)
					{
						if (auto object = a_form->As<RE::TESBoundObject>();
							object && detail::Player::HasObject(object))
						{
							RE::PlaySound("UILockpickingPickBreak");
							RE::PlayerCharacter::GetSingleton()->RemoveItem(
								object,
								1,
								RE::ITEM_REMOVE_REASON::kRemove,
								nullptr,
								nullptr);
							HandleExperience(RE::LOCK_LEVEL::kUnlocked);
							return RE::BSContainer::ForEachResult::kStop;
						}

						return RE::BSContainer::ForEachResult::kContinue;
					});
			}
		}

		static void HandleWaxKey(RE::TESKey* a_key)
		{
			if (a_key && !detail::Player::HasObject(a_key) && detail::Player::HasWaxKey())
			{
				if (auto player = RE::PlayerCharacter::GetSingleton())
					player->AddObjectToContainer(a_key, nullptr, 1, nullptr);

				auto sound = a_key->pickupSound->GetFormEditorID();
				if (REX::STR::IS_EMPTY(sound))
					sound = "ITMKeyUpSD";

				auto name = a_key->GetFullName();
				if (!REX::STR::IS_EMPTY(name))
				{
					if (auto setting = RE::GameSettingCollection::GetSingleton())
					{
						if (auto format = setting->GetSetting("sAddItemtoInventory"))
						{
							auto result = std::format("{} {}"sv, name, format->GetString());
							RE::SendHUDMessage::ShowHUDMessage(result.c_str(), sound);
							return;
						}
					}
				}

				RE::PlaySound(sound);
			}
		}

		static void UnlockObject(RE::TESObjectREFR* a_refr, bool a_picked)
		{
			a_refr->GetLock()->SetLocked(false);
			a_refr->AddLockChange();

			if (a_picked)
			{
				RE::LocksPicked::Event event{};
				if (auto source = RE::LocksPicked::QEventSource())
					source->SendEvent(&event);
			}

			if (Settings::MCM::General::bLockpickingCrimeCheck)
				HandleCrime(a_refr);

			RE::PlaySound("UILockpickingUnlock");
			HandleActivateUpdate(a_refr);

			if (auto player = RE::PlayerCharacter::GetSingleton();
				player && Settings::MCM::General::bActivateAfterPick)
			{
				a_refr->ActivateRef(player, 0, nullptr, 0, false);
			}
		}
	};
}
