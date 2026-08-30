#include "Forms.h"
#include "Hooks.h"
#include "Papyrus.h"

namespace
{
	void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
	{
		switch (a_msg->type)
		{
		case SKSE::MessagingInterface::kDataLoaded:
			Forms::Register();
			Settings::MCM::Update(true);
			break;
		default:
			break;
		}
	}
}

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse, { .trampoline = true, .trampolineSize = 32 });
	SKSE::GetMessagingInterface()->RegisterListener(MessageHandler);
	SKSE::GetPapyrusInterface()->Register(Papyrus::AutoLockNative::Register);
	return true;
}
