#include "Forms.h"
#include "Hooks.h"
#include "Papyrus.h"

namespace
{
	void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
	{
		switch (a_msg->type)
		{
		case SKSE::MessagingInterface::kPostLoad:
		{
			Hooks::AutoLockNative::InstallHooks();
			break;
		}
		case SKSE::MessagingInterface::kDataLoaded:
		{
			Forms::Register();
			Settings::MCM::Update(true);
			break;
		}
		default:
		{
			break;
		}
		}
	}
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
	SKSE::AllocTrampoline(256);

	SKSE::GetMessagingInterface()->RegisterListener(MessageHandler);
	SKSE::GetPapyrusInterface()->Register(Papyrus::AutoLockNative::Register);

	return true;
}
