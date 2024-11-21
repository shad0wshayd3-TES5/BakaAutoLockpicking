#pragma once

namespace Settings
{
	class MCM
	{
	public:
		class General
		{
		public:
			inline static bool bActivateAfterPick{ true };
			inline static bool bDetectionEventFailure{ false };
			inline static bool bDetectionEventSuccess{ false };
			inline static bool bIgnoreHasKey{ false };
			inline static bool bLockpickingCrimeCheck{ true };
			inline static bool bModEnabled{ true };
			inline static bool bShowRollResults{ false };
			inline static bool bUnbreakableLockpicks{ false };

			inline static std::int32_t iDetectionEventFailureLevel{ 20 };
			inline static std::int32_t iDetectionEventSuccessLevel{ 20 };
			inline static std::int32_t iSkillIndex{ 8 };

			inline static std::string sCriticalFailure;
			inline static std::string sCriticalSuccess;
			inline static std::string sShowRollResults;
		};

		class Rolls
		{
		public:
			inline static bool bCriticalFailure{ false };
			inline static bool bCriticalSuccess{ true };

			inline static std::int32_t iDCNovice{ 10 };
			inline static std::int32_t iDCApprentice{ 12 };
			inline static std::int32_t iDCAdept{ 15 };
			inline static std::int32_t iDCExpert{ 18 };
			inline static std::int32_t iDCMaster{ 20 };
			inline static std::int32_t iPlayerDiceMin{ 1 };
			inline static std::int32_t iPlayerDiceMax{ 20 };
			inline static std::int32_t iBonusPerBonus{ 0 };
			inline static std::int32_t iBonusPerLcksm{ 1 };
			inline static std::int32_t iBonusPerPerks{ 1 };
			inline static std::int32_t iBonusPerSkills{ 20 };
		};

		inline static void Update()
		{
			if (m_FirstRun)
			{
				GetFormatStrings();
				m_FirstRun = false;
			}

			m_ini_base.LoadFile("Data/MCM/Config/AutoLockpicking/settings.ini");
			m_ini_user.LoadFile("Data/MCM/Settings/AutoLockpicking.ini");

			// General
			GetModSettingBool("General", "bActivateAfterPick", General::bActivateAfterPick);
			GetModSettingBool("General", "bDetectionEventFailure", General::bDetectionEventFailure);
			GetModSettingBool("General", "bDetectionEventSuccess", General::bDetectionEventSuccess);
			GetModSettingBool("General", "bIgnoreHasKey", General::bIgnoreHasKey);
			GetModSettingBool("General", "bLockpickingCrimeCheck", General::bLockpickingCrimeCheck);
			GetModSettingBool("General", "bModEnabled", General::bModEnabled);
			GetModSettingBool("General", "bShowRollResults", General::bShowRollResults);
			GetModSettingBool("General", "bUnbreakableLockpicks", General::bUnbreakableLockpicks);
			GetModSettingLong("General", "iDetectionEventFailureLevel", General::iDetectionEventFailureLevel);
			GetModSettingLong("General", "iDetectionEventSuccessLevel", General::iDetectionEventSuccessLevel);
			GetModSettingLong("General", "iSkillIndex", General::iSkillIndex);

			// Rolls
			GetModSettingBool("Rolls", "bCriticalFailure", Rolls::bCriticalFailure);
			GetModSettingBool("Rolls", "bCriticalSuccess", Rolls::bCriticalSuccess);
			GetModSettingLong("Rolls", "iDCNovice", Rolls::iDCNovice);
			GetModSettingLong("Rolls", "iDCApprentice", Rolls::iDCApprentice);
			GetModSettingLong("Rolls", "iDCAdept", Rolls::iDCAdept);
			GetModSettingLong("Rolls", "iDCExpert", Rolls::iDCExpert);
			GetModSettingLong("Rolls", "iDCMaster", Rolls::iDCMaster);
			GetModSettingLong("Rolls", "iPlayerDiceMin", Rolls::iPlayerDiceMin);
			GetModSettingLong("Rolls", "iPlayerDiceMax", Rolls::iPlayerDiceMax);
			GetModSettingLong("Rolls", "iBonusPerBonus", Rolls::iBonusPerBonus);
			GetModSettingLong("Rolls", "iBonusPerLcksm", Rolls::iBonusPerLcksm);
			GetModSettingLong("Rolls", "iBonusPerPerks", Rolls::iBonusPerPerks);
			GetModSettingLong("Rolls", "iBonusPerSkills", Rolls::iBonusPerSkills);

			m_ini_base.Reset();
			m_ini_user.Reset();
		}

		inline static bool m_FirstRun{ true };

	private:
		inline static void GetModSettingBool(const std::string& a_section, const std::string& a_setting, bool& a_value)
		{
			auto base = m_ini_base.GetBoolValue(a_section.c_str(), a_setting.c_str(), a_value);
			auto user = m_ini_user.GetBoolValue(a_section.c_str(), a_setting.c_str(), base);
			a_value = user;
		}

		inline static void GetModSettingLong(const std::string& a_section, const std::string& a_setting, std::int32_t& a_value)
		{
			auto base = m_ini_base.GetLongValue(a_section.c_str(), a_setting.c_str(), a_value);
			auto user = m_ini_user.GetLongValue(a_section.c_str(), a_setting.c_str(), base);
			a_value = static_cast<std::int32_t>(user);
		}

		inline static void GetFormatStrings()
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

		inline static CSimpleIniA m_ini_base{ true };
		inline static CSimpleIniA m_ini_user{ true };
	};
}
