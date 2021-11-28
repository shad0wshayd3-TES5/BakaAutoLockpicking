#pragma once
#include "Forms.h"
#include "Settings.h"

#include <effolkronium/random.hpp>

namespace Papyrus
{
	using Random = effolkronium::random_thread_local;

	class AutoLockNative
	{
	public:
		enum
		{
			kVersion = 1
		};

		static inline std::int32_t GetVersion(RE::StaticFunctionTag*)
		{
			return kVersion;
		}

		static inline void Unlock(RE::StaticFunctionTag*, RE::TESObjectREFR* a_reference)
		{
			const Settings::MCM mcm;
			if (!a_reference) {
				return;
			}

			if (!a_reference->GetLock()) {
				return;
			}

			auto lockLevel = a_reference->GetLockLevel();
			if (lockLevel == RE::LOCK_LEVEL::kUnlocked) {
				return;
			}

			auto player = RE::PlayerCharacter::GetSingleton();
			if (!player) {
				return;
			}

			auto a_key = a_reference->GetLock()->key;
			if (!mcm.GetModSettingBool("General", "bIgnoreHasKey", false)) {
				if (a_key && player->GetInventory([&](RE::TESBoundObject& a_item) { return a_item.GetFormID() == a_key->GetFormID(); }).size()) {
					if (mcm.GetModSettingBool("General", "bActivateAfterPick", true)) {
						a_reference->ActivateRef(player, 0, nullptr, 0, false);
					} else {
						RE::PlaySound("UILockpickingUnlock");
						a_reference->GetLock()->SetLocked(false);
						HandleActivateUpdate(a_reference);
					}
					return;
				}
			}

			auto diffClass = GetLockDifficultyClass(mcm, lockLevel);
			auto minRoll = mcm.GetModSettingInt("Rolls", "iPlayerDiceMin", 1);
			auto maxRoll = mcm.GetModSettingInt("Rolls", "iPlayerDiceMax", 20);
			maxRoll = std::max(minRoll, maxRoll);
			auto rollValue = Random::get(minRoll, maxRoll);

			auto rollBonus = GetRollModifier(mcm);
			logger::info("DC: {:d} | Roll(Bonus): {:d}({:d})", diffClass, rollValue, rollBonus);
			if (mcm.GetModSettingBool("General", "bShowRollResults", false)) {
				auto msg = fmt::format(Settings::MCM::m_ShowRollResults, diffClass, rollValue, rollBonus);
				RE::DebugNotification(msg.c_str());
			}

			if (mcm.GetModSettingBool("Rolls", "bCriticalFailure", false)) {
				if ((rollValue == 1) && (maxRoll > 1)) {
					RE::DebugNotification(Settings::MCM::m_CriticalFailure.c_str());
					player->currentProcess->KnockExplosion(player, player->data.location, 5.0f);
				}
			}

			if (mcm.GetModSettingBool("Rolls", "bCriticalSuccess", false)) {
				if (rollValue == maxRoll) {
					RE::DebugNotification(Settings::MCM::m_CriticalSuccess.c_str());
					HandleExperience(RE::LOCK_LEVEL::kVeryHard);
				}
			}

			rollValue += rollBonus;
			if (rollValue >= diffClass) {
				RE::PlaySound("UILockpickingUnlock");
				a_reference->GetLock()->SetLocked(false);
				if (mcm.GetModSettingBool("General", "bActivateAfterPick", true)) {
					a_reference->ActivateRef(player, 0, nullptr, 0, false);
				} else {
					HandleActivateUpdate(a_reference);
				}

				HandleExperience(lockLevel);
				if (a_key && HasWaxKey()) {
					if (!player->GetInventory([&](RE::TESBoundObject& a_item) { return a_item.GetFormID() == a_key->GetFormID(); }).size()) {
						if (auto containerChanges = player->extraList.GetContainerChanges(); containerChanges) {
							containerChanges->AddInventoryItem(a_key, nullptr, 1, nullptr);
							a_key->NotifyPickup(1, 1, true, nullptr);
						}
					}
				}

				if (mcm.GetModSettingBool("General", "bDetectionEventSuccess", false)) {
					player->currentProcess->SetActorsDetectionEvent(
						player,
						a_reference->data.location,
						mcm.GetModSettingInt("General", "iDetectionEventSuccessLevel", 20),
						a_reference);
				}

			} else {
				if (HasBreakableLockpick(mcm)) {
					RE::PlaySound("UILockpickingPickBreak");
					HandleExperience(RE::LOCK_LEVEL::kUnlocked);
					HandleLockpickRemoval();
				} else {
					RE::PlaySound("UILockpickingCylinderTurn");
				}

				if (mcm.GetModSettingBool("General", "bDetectionEventFailure", false)) {
					player->currentProcess->SetActorsDetectionEvent(
						player,
						a_reference->data.location,
						mcm.GetModSettingInt("General", "iDetectionEventFailureLevel", 20),
						a_reference);
				}
			}

			if (mcm.GetModSettingBool("General", "bLockpickingCrimeCheck", true)) {
				HandleCrime(a_reference);
			}
		}

		static inline std::vector<std::string> GetRollModifiers(RE::StaticFunctionTag*)
		{
			Settings::MCM mcm;
			auto _skill = GetRollModifier_Skill(mcm);
			auto _perks = GetRollModifier_Perks(mcm);
			auto _lcksm = GetRollModifier_Lcksm(mcm);
			auto _bonus = GetRollModifier_Bonus(mcm);
			auto _total = _skill + _perks + _lcksm + _bonus;

			return std::vector<std::string>{
				fmt::format(_skill >= 0 ? "+{:d}" : "{:d}", _skill),
				fmt::format(_perks >= 0 ? "+{:d}" : "{:d}", _perks),
				fmt::format(_lcksm >= 0 ? "+{:d}" : "{:d}", _lcksm),
				fmt::format(_bonus >= 0 ? "+{:d}" : "{:d}", _bonus),
				fmt::format(_total >= 0 ? "+{:d}" : "{:d}", _total)
			};
		}

	public:
		static inline bool Register(RE::BSScript::IVirtualMachine* a_vm)
		{
			a_vm->RegisterFunction("GetVersion", CLASS_NAME, GetVersion, true);
			a_vm->RegisterFunction("Unlock", CLASS_NAME, Unlock);
			a_vm->RegisterFunction("GetRollModifiers", CLASS_NAME, GetRollModifiers);
			logger::info("Registered funcs for class {}", CLASS_NAME);

			Settings::MCM::GetFormatStrings();
			return true;
		}

	private:
		static constexpr char CLASS_NAME[] = "AutoLockNative";

		static inline std::int32_t GetLockDifficultyClass(const Settings::MCM& a_mcm, RE::LOCK_LEVEL a_lockLevel)
		{
			switch (a_lockLevel) {
			case RE::LOCK_LEVEL::kEasy:
				return a_mcm.GetModSettingInt("Rolls", "iDCApprentice", 12);
			case RE::LOCK_LEVEL::kAverage:
				return a_mcm.GetModSettingInt("Rolls", "iDCAdept", 15);
			case RE::LOCK_LEVEL::kHard:
				return a_mcm.GetModSettingInt("Rolls", "iDCExpert", 18);
			case RE::LOCK_LEVEL::kVeryHard:
				return a_mcm.GetModSettingInt("Rolls", "iDCMaster", 20);
			}

			return a_mcm.GetModSettingInt("Rolls", "iDCNovice", 10);
		}

		static inline std::int32_t GetRollModifier(const Settings::MCM& a_mcm)
		{
			std::int32_t result = 0;
			result += GetRollModifier_Skill(a_mcm);
			result += GetRollModifier_Perks(a_mcm);
			result += GetRollModifier_Lcksm(a_mcm);
			result += GetRollModifier_Bonus(a_mcm);
			return result;
		}

		static inline std::int32_t GetRollModifier_Skill(const Settings::MCM& a_mcm)
		{
			auto lpMod = 1.0f + (RE::PlayerCharacter::GetSingleton()->GetActorValue(RE::ActorValue::kLockpickingModifier) / 100.0f);
			auto lpLvl = RE::PlayerCharacter::GetSingleton()->GetActorValue(RE::ActorValue::kLockpicking) * lpMod;
			return static_cast<std::int32_t>(floor(lpLvl / a_mcm.GetModSettingInt("Rolls", "iBonusPerSkills", 20)));
		}

		static inline std::int32_t GetRollModifier_Perks(const Settings::MCM& a_mcm)
		{
			std::int32_t result = 0;
			for (auto& form : Forms::AutoLock_Perks_Base->forms) {
				if (auto perk = form->As<RE::BGSPerk>(); perk) {
					if (RE::PlayerCharacter::GetSingleton()->HasPerk(perk)) {
						result += a_mcm.GetModSettingInt("Rolls", "iBonusPerPerks", 1);
					}
				}
			}

			return result;
		}

		static inline std::int32_t GetRollModifier_Lcksm(const Settings::MCM& a_mcm)
		{
			for (auto& form : Forms::AutoLock_Perks_Locksmith->forms) {
				if (auto perk = form->As<RE::BGSPerk>(); perk) {
					if (RE::PlayerCharacter::GetSingleton()->HasPerk(perk)) {
						return a_mcm.GetModSettingInt("Rolls", "iBonusPerLcksm", 1);
					}
				}
			}

			return 0;
		}

		static inline std::int32_t GetRollModifier_Bonus(const Settings::MCM& a_mcm)
		{
			return a_mcm.GetModSettingInt("Rolls", "iBonusPerBonus", 0);
		}

		static inline void HandleActivateUpdate(RE::TESObjectREFR* a_reference)
		{
			RE::GFxValue HUDObject;

			if (auto UI = RE::UI::GetSingleton(); UI) {
				if (auto HUDMenu = UI->GetMenu<RE::HUDMenu>(); HUDMenu && HUDMenu->uiMovie) {
					HUDMenu->uiMovie->GetVariable(std::addressof(HUDObject), "_root.HUDMovieBaseInstance");
				}
			}

			if (HUDObject.IsObject()) {
				std::array<RE::GFxValue, 10> args;
				args[0] = true;
				args[2] = true;
				args[3] = true;

				RE::BSString name;
				if (a_reference->data.objectReference) {
					a_reference->data.objectReference->GetActivateText(a_reference, name);
				}
				args[1] = name.empty() ? "" : name.c_str();

				HUDObject.Invoke("SetCrosshairTarget", args);
			}
		}

		static inline void HandleCrime(RE::TESObjectREFR* a_reference)
		{
			auto owner = a_reference->GetOwner();
			if (!owner && owner->GetFormType() == RE::FormType::Door) {
				if (auto extra = a_reference->extraList.GetByType<RE::ExtraTeleport>(); extra) {
					if (auto teleport = extra->teleportData; teleport) {
						if (auto linkedDoor = teleport->linkedDoor.get(); linkedDoor) {
							if (auto linkedCell = linkedDoor->GetParentCell(); linkedCell) {
								owner = linkedCell->GetOwner();
							}
						}
					}
				}
			}

			if (owner) {
				if (auto Player = RE::PlayerCharacter::GetSingleton(); Player) {
					if (auto ProcessLists = RE::ProcessLists::GetSingleton(); ProcessLists) {
						std::uint32_t LOSCount{ 1 };
						if (ProcessLists->RequestHighestDetectionLevelAgainstActor(Player, LOSCount) > 0) {
							auto chance{ 1.0f };
							RE::ApplyPerkEntries(RE::BGSEntryPoint::ENTRY_POINT::kModLockpickingCrimeChance, Player, a_reference, &chance);

							auto roll = Random::get(0.0f, 1.0f);
							if (roll < chance) {
								auto prison = Player->currentPrisonFaction;
								if (prison && prison->crimeData.crimevalues.escapeCrimeGold) {
									Player->SetEscaping(true, false);
								} else {
									Player->TrespassAlarm(a_reference, owner, -1);
								}
							}
						}
					}
				}
			}
		}

		static inline void HandleExperience(RE::LOCK_LEVEL a_lockLevel)
		{
			switch (a_lockLevel) {
			case RE::LOCK_LEVEL::kVeryEasy:
				RE::PlayerCharacter::GetSingleton()->AddSkillExperience(
					RE::ActorValue::kLockpicking,
					RE::GameSettingCollection::GetSingleton()->GetSetting("fSkillUsageLockPickVeryEasy")->GetFloat());
				return;
			case RE::LOCK_LEVEL::kEasy:
				RE::PlayerCharacter::GetSingleton()->AddSkillExperience(
					RE::ActorValue::kLockpicking,
					RE::GameSettingCollection::GetSingleton()->GetSetting("fSkillUsageLockPickEasy")->GetFloat());
				return;
			case RE::LOCK_LEVEL::kAverage:
				RE::PlayerCharacter::GetSingleton()->AddSkillExperience(
					RE::ActorValue::kLockpicking,
					RE::GameSettingCollection::GetSingleton()->GetSetting("fSkillUsageLockPickAverage")->GetFloat());
				return;
			case RE::LOCK_LEVEL::kHard:
				RE::PlayerCharacter::GetSingleton()->AddSkillExperience(
					RE::ActorValue::kLockpicking,
					RE::GameSettingCollection::GetSingleton()->GetSetting("fSkillUsageLockPickHard")->GetFloat());
				return;
			case RE::LOCK_LEVEL::kVeryHard:
				RE::PlayerCharacter::GetSingleton()->AddSkillExperience(
					RE::ActorValue::kLockpicking,
					RE::GameSettingCollection::GetSingleton()->GetSetting("fSkillUsageLockPickVeryHard")->GetFloat());
				return;
			}

			RE::PlayerCharacter::GetSingleton()->AddSkillExperience(
				RE::ActorValue::kLockpicking,
				RE::GameSettingCollection::GetSingleton()->GetSetting("fSkillUsageLockPickBroken")->GetFloat());
		}

		static inline void HandleLockpickRemoval()
		{
			for (auto& form : Forms::AutoLock_Items_Lockpick->forms) {
				if (auto object = form->As<RE::TESBoundObject>(); object) {
					if (RE::PlayerCharacter::GetSingleton()->GetInventory([&](RE::TESBoundObject& a_item) { return a_item.GetFormID() == object->GetFormID(); }).size()) {
						RE::PlayerCharacter::GetSingleton()->RemoveItem(
							object,
							1,
							RE::ITEM_REMOVE_REASON::kRemove,
							nullptr,
							nullptr);
						return;
					}
				}
			}
		}

		static inline bool HasBreakableLockpick(const Settings::MCM& a_mcm)
		{
			for (auto& form : Forms::AutoLock_Items_SkeletonKey->forms) {
				if (RE::PlayerCharacter::GetSingleton()->GetInventory([&](RE::TESBoundObject& a_item) { return a_item.GetFormID() == form->GetFormID(); }).size()) {
					return false;
				}
			}

			for (auto& form : Forms::AutoLock_Perks_Unbreakable->forms) {
				if (auto perk = form->As<RE::BGSPerk>(); perk) {
					if (RE::PlayerCharacter::GetSingleton()->HasPerk(perk)) {
						return false;
					}
				}
			}

			if (a_mcm.GetModSettingBool("General", "bUnbreakableLockpicks", false)) {
				return false;
			}

			return true;
		}

		static inline bool HasWaxKey()
		{
			for (auto& form : Forms::AutoLock_Perks_WaxKey->forms) {
				if (auto perk = form->As<RE::BGSPerk>(); perk) {
					if (RE::PlayerCharacter::GetSingleton()->HasPerk(perk)) {
						return true;
					}
				}
			}

			return false;
		}
	};

	inline bool Register(RE::BSScript::IVirtualMachine* a_vm)
	{
		AutoLockNative::Register(a_vm);
		return true;
	}
}
