// module_registry.cpp — регистрирует только Extra-модули (те что реально есть).
// Остальные модули (aim, hook, esp и т.д.) добавляются позже когда их файлы созданы.
#include "cheat.h"
#include "extra.h"

void CCheat::RegisterModules()
{
    const CProfile *p = m_Config.ActiveProfile();

    // ── Self Unfreeze ─────────────────────────────────────────────────────────
    {
        auto m = std::make_unique<CSelfUnfreeze>();
        m->SetMode(p->Extra_UnfreezeMode);
        m->SetShowTimer(p->Extra_UnfreezeTimer);
        m->SetAutoAngle(p->Extra_UnfreezeAutoAngle);
        m_Modules.push_back(std::move(m));
    }

    // ── Bhop ──────────────────────────────────────────────────────────────────
    {
        auto m = std::make_unique<CBhop>();
        m->SetAutoOnly(p->Extra_BhopAutoOnly);
        m_Modules.push_back(std::move(m));
    }

    // ── Auto Strafe ───────────────────────────────────────────────────────────
    {
        auto m = std::make_unique<CAutoStrafe>();
        m->SetStrength(p->Extra_StrafStrength);
        m_Modules.push_back(std::move(m));
    }

    // ── Speed Hack ────────────────────────────────────────────────────────────
    {
        auto m = std::make_unique<CSpeedHack>();
        m->SetMultiplier(p->Extra_SpeedMult);
        m_Modules.push_back(std::move(m));
    }

    // ── Zoom Hack ─────────────────────────────────────────────────────────────
    {
        auto m = std::make_unique<CZoomHack>();
        m->SetZoom(p->Extra_ZoomLevel);
        m_Modules.push_back(std::move(m));
    }

    // ── Ghost Trail ───────────────────────────────────────────────────────────
    {
        auto m = std::make_unique<CGhostTrail>();
        m->SetLength(p->Extra_TrailLength);
        m->m_R = p->Extra_TrailR;
        m->m_G = p->Extra_TrailG;
        m->m_B = p->Extra_TrailB;
        m_Modules.push_back(std::move(m));
    }

    // ── Kill Say ──────────────────────────────────────────────────────────────
    {
        auto m = std::make_unique<CKillSay>();
        m->SetKillMsg(p->Extra_KillMsg);
        m->SetDeathMsg(p->Extra_DeathMsg);
        m->SetOnKill(p->Extra_KillSayOnKill);
        m->SetOnDeath(p->Extra_KillSayOnDeath);
        m_Modules.push_back(std::move(m));
    }

    // ── Server Info HUD ───────────────────────────────────────────────────────
    m_Modules.emplace_back(std::make_unique<CServerInfoHUD>());

    // ── Player Tracker ────────────────────────────────────────────────────────
    {
        auto m = std::make_unique<CPlayerTracker>();
        m->SetTarget(p->Extra_TrackerTarget);
        m->SetFollow(p->Extra_TrackerFollow);
        m_Modules.push_back(std::move(m));
    }

    // ── Auto Login ────────────────────────────────────────────────────────────
    {
        auto m = std::make_unique<CAutoLogin>();
        m->SetPassword(p->Extra_LoginPw);
        m_Modules.push_back(std::move(m));
    }

    // ── Chat Logger ───────────────────────────────────────────────────────────
    {
        auto m = std::make_unique<CChatLogger>();
        m->SetPath(p->Extra_ChatLogPath);
        m_Modules.push_back(std::move(m));
    }
}
