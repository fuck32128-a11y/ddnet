// cheat.cpp — CCheat controller v2: added Extra module ApplyProfile entries
#include "cheat.h"
#include "extra.h"

void CCheat::Init(CGameClient *pClient)
{
    m_pClient = pClient;
    m_Config.LoadAll("ddnet_cheat/profiles");
    RegisterModules();
    ApplyProfile();
}

void CCheat::OnTick()
{
    for(auto &m : m_Modules)
        m->OnTick();
}

void CCheat::OnRender()
{
    for(auto &m : m_Modules)
        m->OnRender();
}

void CCheat::OnInput(IInput *pInput)
{
    for(auto &m : m_Modules)
        m->OnInput(pInput);

    for(auto &[key, cb] : m_KeyBinds)
        (void)key, (void)cb;
}

CCheatModule *CCheat::Module(const std::string &name)
{
    for(auto &m : m_Modules)
        if(m->Name() == name) return m.get();
    return nullptr;
}

void CCheat::BindKey(int key, std::function<void()> cb)
{
    m_KeyBinds.push_back({key, std::move(cb)});
}

// ─── ApplyProfile ─────────────────────────────────────────────────────────────

void CCheat::ApplyProfile()
{
    CProfile *p = m_Config.ActiveProfile();

    auto en = [&](const char *name, bool v) {
        if(auto *m = Module(name)) m->SetEnabled(v);
    };

    // Aimbot
    en("AimBot",          p->AimBot_Enabled);
    en("HammerAssist",    p->Hammer_Assist || p->Hammer_Auto);

    // Hook
    en("HookAssist",      p->Hook_Auto || p->Hook_Assist);
    en("AutoJump",        p->Hook_AutoJump);
    en("FreezeIndicator", p->Hook_FreezeIndicator);

    // ESP
    en("PlayerESP",       p->ESP_Players);
    en("EntityESP",       p->ESP_Entities);
    en("Tracers",         p->ESP_Tracers);
    en("ExtendedRadar",   p->ESP_ExtRadar);
    en("DamageIndicators",p->ESP_DamageIndicators);

    // HUD
    en("CustomHUD",       p->HUD_Custom);
    en("HUDNotifications",p->HUD_Notifications);

    // Misc
    en("Spinbot",         p->Misc_Spinbot);
    en("NameSpoof",       p->Misc_NameSpoof);
    en("AntiPing",        p->Misc_AntiPing);
    en("AutoShoot",       p->Misc_AutoShoot);

    // Visual
    en("FakeAim",         p->Visual_FakeAim);
    en("RobotAim",        p->Visual_RobotAim);
    en("RandomAim",       p->Visual_RandomAim);
    en("FakeAngles",      p->Visual_FakeAngles);
    en("DrawAimLines",    p->Visual_DrawAimLines);
    en("DrawBulletLines", p->Visual_DrawBulletLines);
    en("RainbowTee",      p->Visual_RainbowTee);
    en("RainbowHook",     p->Visual_RainbowHook);
    en("SmoothCamera",    p->Visual_SmoothCamera);
    en("EmoteSpam",       p->Visual_EmoteSpam);
    en("ModDetector",     p->Visual_ModDetector);
    en("CustomWatermark", p->Visual_Watermark);
    en("DiscordRPC",      p->Visual_DiscordRPC);

    // Movement
    en("MoonWalk",        p->Mov_MoonWalk);
    en("AutoRehook",      p->Mov_AutoRehook);
    en("FallPrediction",  p->Mov_FallPrediction);
    en("AutoEdge",        p->Mov_AutoEdge);
    en("UnfreezeBot",     p->Mov_UnfreezeBot);
    en("DummyFly",        p->Mov_DummyFly);
    en("JetRide",         p->Mov_JetRide);
    en("FlowfieldEditor", p->Mov_FlowfieldEditor);

    // TAS
    en("ReplayManager",        p->TAS_ReplayManager);
    en("AutoRewind",           p->TAS_AutoRewind);
    en("AutoForward",          p->TAS_AutoForward);
    en("AutoPause",            p->TAS_AutoPause);
    en("ReplayPathPrediction", p->TAS_PathPrediction);
    en("PathFeatures",         p->TAS_PathFeatures);
    en("ReplayValidation",     p->TAS_Validation);

    // Extra
    en("SelfUnfreeze",   p->Extra_SelfUnfreeze);
    en("Bhop",           p->Extra_Bhop);
    en("AutoStrafe",     p->Extra_AutoStrafe);
    en("SpeedHack",      p->Extra_SpeedHack);
    en("ZoomHack",       p->Extra_ZoomHack);
    en("GhostTrail",     p->Extra_GhostTrail);
    en("KillSay",        p->Extra_KillSay);
    en("ServerInfoHUD",  p->Extra_ServerInfoHUD);
    en("PlayerTracker",  p->Extra_PlayerTracker);
    en("AutoLogin",      p->Extra_AutoLogin);
    en("ChatLogger",     p->Extra_ChatLogger);

    // Apply Extra parameters (after enable)
    auto applyExtra = [&]()
    {
        if(auto *m = static_cast<CBhop*>(Module("Bhop")))
            m->SetAutoOnly(p->Extra_BhopAutoOnly);

        if(auto *m = static_cast<CAutoStrafe*>(Module("AutoStrafe")))
            m->SetStrength(p->Extra_StrafStrength);

        if(auto *m = static_cast<CSpeedHack*>(Module("SpeedHack")))
            m->SetMultiplier(p->Extra_SpeedMult);

        if(auto *m = static_cast<CZoomHack*>(Module("ZoomHack")))
            m->SetZoom(p->Extra_ZoomLevel);

        if(auto *m = static_cast<CGhostTrail*>(Module("GhostTrail")))
        {
            m->SetLength(p->Extra_TrailLength);
            m->m_R = p->Extra_TrailR;
            m->m_G = p->Extra_TrailG;
            m->m_B = p->Extra_TrailB;
        }

        if(auto *m = static_cast<CKillSay*>(Module("KillSay")))
        {
            m->SetKillMsg(p->Extra_KillMsg);
            m->SetDeathMsg(p->Extra_DeathMsg);
            m->SetOnKill(p->Extra_KillSayOnKill);
            m->SetOnDeath(p->Extra_KillSayOnDeath);
        }

        if(auto *m = static_cast<CPlayerTracker*>(Module("PlayerTracker")))
        {
            m->SetTarget(p->Extra_TrackerTarget);
            m->SetFollow(p->Extra_TrackerFollow);
        }

        if(auto *m = static_cast<CAutoLogin*>(Module("AutoLogin")))
            m->SetPassword(p->Extra_LoginPw);

        if(auto *m = static_cast<CChatLogger*>(Module("ChatLogger")))
            m->SetPath(p->Extra_ChatLogPath);

        if(auto *m = static_cast<CSelfUnfreeze*>(Module("SelfUnfreeze")))
        {
            m->SetMode(p->Extra_UnfreezeMode);
            m->SetShowTimer(p->Extra_UnfreezeTimer);
            m->SetAutoAngle(p->Extra_UnfreezeAutoAngle);
        }
    };
    applyExtra();
}

// ─── CCheatConfigManager implementation ──────────────────────────────────────

CCheatConfigManager::CCheatConfigManager()
{
    // Default profile
    m_Profiles.emplace_back();
    m_Active = 0;
}

void CCheatConfigManager::LoadAll(const std::string &dir)
{
    // stub: just ensure at least one profile exists
    if(m_Profiles.empty())
        m_Profiles.emplace_back();
    m_Active = 0;
}

void CCheatConfigManager::SaveProfile(const CProfile &p, const std::string &dir)
{
    // stub: no-op for now
    (void)p; (void)dir;
}
