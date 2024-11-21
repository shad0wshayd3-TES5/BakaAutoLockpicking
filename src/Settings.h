#pragma once

namespace Settings
{
	namespace MCM
	{
		namespace General
		{
			static REX::INI::Bool bActivateAfterPick{ "General"sv, "bActivateAfterPick"sv, true };
			static REX::INI::Bool bDetectionEventFailure{ "General"sv, "bDetectionEventFailure"sv, false };
			static REX::INI::Bool bDetectionEventSuccess{ "General"sv, "bDetectionEventSuccess"sv, false };
			static REX::INI::Bool bIgnoreHasKey{ "General"sv, "bIgnoreHasKey"sv, false };
			static REX::INI::Bool bLockpickingCrimeCheck{ "General"sv, "bLockpickingCrimeCheck"sv, true };
			static REX::INI::Bool bModEnabled{ "General"sv, "bModEnabled"sv, true };
			static REX::INI::Bool bShowRollResults{ "General"sv, "bShowRollResults"sv, false };
			static REX::INI::Bool bUnbreakableLockpicks{ "General"sv, "bUnbreakableLockpicks"sv, false };

			static REX::INI::I32 iDetectionEventFailureLevel{ "General"sv, "iDetectionEventFailureLevel"sv, 20 };
			static REX::INI::I32 iDetectionEventSuccessLevel{ "General"sv, "iDetectionEventSuccessLevel"sv, 20 };
			static REX::INI::I32 iSkillIndex{ "General"sv, "iSkillIndex"sv, 8 };

			static std::string sCriticalFailure;
			static std::string sCriticalSuccess;
			static std::string sShowRollResults;
		}

		namespace Rolls
		{
			static REX::INI::Bool bCriticalFailure{ "Rolls"sv, "bCriticalFailure"sv, false };
			static REX::INI::Bool bCriticalSuccess{ "Rolls"sv, "bCriticalSuccess"sv, true };

			static REX::INI::I32 iDCNovice{ "Rolls"sv, "iDCNovice"sv, 10 };
			static REX::INI::I32 iDCApprentice{ "Rolls"sv, "iDCApprentice"sv, 12 };
			static REX::INI::I32 iDCAdept{ "Rolls"sv, "iDCAdept"sv, 15 };
			static REX::INI::I32 iDCExpert{ "Rolls"sv, "iDCExpert"sv, 18 };
			static REX::INI::I32 iDCMaster{ "Rolls"sv, "iDCMaster"sv, 20 };
			static REX::INI::I32 iPlayerDiceMin{ "Rolls"sv, "iPlayerDiceMin"sv, 1 };
			static REX::INI::I32 iPlayerDiceMax{ "Rolls"sv, "iPlayerDiceMax"sv, 20 };
			static REX::INI::I32 iBonusPerBonus{ "Rolls"sv, "iBonusPerBonus"sv, 0 };
			static REX::INI::I32 iBonusPerLcksm{ "Rolls"sv, "iBonusPerLcksm"sv, 1 };
			static REX::INI::I32 iBonusPerPerks{ "Rolls"sv, "iBonusPerPerks"sv, 1 };
			static REX::INI::I32 iBonusPerSkills{ "Rolls"sv, "iBonusPerSkills"sv, 20 };
		}

		static void Update(bool a_firstRun)
		{
			if (a_firstRun)
			{
				GetFormatStrings();
			}

			const auto ini = REX::INI::SettingStore::GetSingleton();
			ini->Init(
				"Data/SKSE/plugins/BakaAutoLockpicking.ini",
				"Data/SKSE/plugins/BakaAutoLockpickingCustom.ini");
			ini->Load();
		}

		static void GetFormatStrings()
		{
			if (auto BSGFxMgr = RE::BSScaleformManager::GetSingleton(); BSGFxMgr && BSGFxMgr->loader)
			{
				if (auto BSGFxTrns = BSGFxMgr->loader->GetState<RE::BSScaleformTranslator>(RE::GFxState::StateType::kTranslator))
				{
					auto FetchTranslation = [](RE::BSScaleformTranslator* a_trns, const wchar_t* a_key, std::string& a_output)
					{
						RE::GFxTranslator::TranslateInfo TrnsInfo;
						RE::GFxWStringBuffer GFxBuffer;

						TrnsInfo.key = a_key;
						TrnsInfo.result = std::addressof(GFxBuffer);
						a_trns->Translate(std::addressof(TrnsInfo));

						a_output.resize(512);
						sprintf_s(a_output.data(), 512, "%ws", GFxBuffer.c_str());
					};

					FetchTranslation(BSGFxTrns.get(), L"$AL_Message_CriticalFailure", General::sCriticalFailure);
					FetchTranslation(BSGFxTrns.get(), L"$AL_Message_CriticalSuccess", General::sCriticalSuccess);
					FetchTranslation(BSGFxTrns.get(), L"$AL_Message_ShowRollResults", General::sShowRollResults);
				}
			}
		}
	}
}
