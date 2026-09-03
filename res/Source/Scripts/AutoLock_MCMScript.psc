Scriptname AutoLock_MCMScript extends MCM_ConfigBase

Event OnConfigInit()
	MCM_ModEnabled = GetModSettingBool("bModEnabled:General")

	If (!MCM_PerkDisabled)
		If (PlayerRef.HasPerk(AutoLock_ActivateOverridePerk))
			PlayerRef.RemovePerk(AutoLock_ActivateOverridePerk)
		EndIf
		MCM_PerkDisabled = true
	EndIf
EndEvent

Event OnConfigOpen()
	UpdateStatsValues()
EndEvent

Event OnConfigClose()
	AutoLockNative.UpdateSettings()
EndEvent

Event OnSettingChange(string a_ID)
	If (a_ID == "bModEnabled:General")
		MCM_ModEnabled = GetModSettingBool("bModEnabled:General")
		RefreshMenu()
		return
	EndIf

	If (a_ID == "iSkillIndex:General" || MCM_StatsValues.Find(a_ID) >= 0)
		UpdateStatsValues()
		RefreshMenu()
	EndIf
EndEvent

Function UpdateStatsValues()
	string[] modifiers = AutoLockNative.GetRollModifiers()
	MCM_Stats_Skill = modifiers[0]
	MCM_Stats_Perks = modifiers[1]
	MCM_Stats_Lcksm = modifiers[2]
	MCM_Stats_Bonus = modifiers[3]
	MCM_Stats_Total = modifiers[4]
EndFunction

; MCM Properties ----------------------------------------------------------------------------------
bool Property MCM_DummyValue Auto Hidden
bool Property MCM_ModEnabled Auto Hidden
string Property MCM_Stats_Skill Auto Hidden
string Property MCM_Stats_Perks Auto Hidden
string Property MCM_Stats_Lcksm Auto Hidden
string Property MCM_Stats_Bonus Auto Hidden
string Property MCM_Stats_Total Auto Hidden

; Form Properties ---------------------------------------------------------------------------------
Actor Property PlayerRef Auto
Perk Property AutoLock_ActivateOverridePerk Auto

; Update 3.0 Properties -------------------------------------------------------------------------
bool Property MCM_PerkDisabled Auto Hidden
string[] Property MCM_StatsValues Auto
