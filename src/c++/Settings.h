#pragma once
#include <INIReader.h>

namespace Settings
{
	class MCM
	{
	public:
		std::int32_t GetModSettingInt(const std::string& a_section, const std::string& a_setting, const std::int32_t a_default) const
		{
			return static_cast<std::int32_t>(m_ini_user.GetInteger(a_section, a_setting, m_ini_base.GetInteger(a_section, a_setting, a_default)));
		}

		bool GetModSettingBool(const std::string& a_section, const std::string& a_setting, const bool a_default) const
		{
			return m_ini_user.GetBoolean(a_section, a_setting, m_ini_base.GetBoolean(a_section, a_setting, a_default));
		}

		inline static void GetFormatStrings()
		{
			if (auto BSGFxMgr = RE::BSScaleformManager::GetSingleton(); BSGFxMgr && BSGFxMgr->loader) {
				if (auto BSGFxTrns = BSGFxMgr->loader->GetState<RE::BSScaleformTranslator>(RE::GFxState::StateType::kTranslator); BSGFxTrns) {
					auto FetchTranslation = [](RE::BSScaleformTranslator* a_trns, const wchar_t* a_key, std::string& a_output) {
						RE::GFxTranslator::TranslateInfo TrnsInfo;
						RE::GFxWStringBuffer GFxBuffer;

						TrnsInfo.key = a_key;
						TrnsInfo.result = std::addressof(GFxBuffer);
						a_trns->Translate(std::addressof(TrnsInfo));

						a_output.resize(512);
						sprintf_s(a_output.data(), 512, "%ws", GFxBuffer.c_str());
					};

					FetchTranslation(BSGFxTrns.get(), L"$AL_Message_CriticalFailure", m_CriticalFailure);
					FetchTranslation(BSGFxTrns.get(), L"$AL_Message_CriticalSuccess", m_CriticalSuccess);
					FetchTranslation(BSGFxTrns.get(), L"$AL_Message_ShowRollResults", m_ShowRollResults);
				}
			}
		}

		inline static std::string m_CriticalFailure;
		inline static std::string m_CriticalSuccess;
		inline static std::string m_ShowRollResults;

	private:
		inline static INIReader m_ini_base{ "Data/MCM/Config/AutoLockpicking/settings.ini" };
		INIReader m_ini_user{ "Data/MCM/Settings/AutoLockpicking.ini" };
	};
}
