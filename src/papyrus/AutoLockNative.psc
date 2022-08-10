ScriptName AutoLockNative

; @brief Fetch Stringified Roll Modifiers from native code
; @return Returns array with size 5 in order of Skill/Perks/Lcksm/Bonus/Total with pos/neg sign
string[] Function GetRollModifiers() Native Global

; @brief Returns the API version.
; @return Returns 0 if not installed, else returns the API version.
int Function GetVersion() Native Global

; @brief Update values from MCM Helper settings files.
Function UpdateSettings() Native Global
