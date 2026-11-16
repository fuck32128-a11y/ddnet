// extra.h — New feature modules: Bhop, AutoStrafe, SpeedHack, ZoomHack,
//            GhostTrail, KillSay, ServerInfoHUD, PlayerTracker,
//            AutoLogin, ChatLogger, SelfUnfreeze
#pragma once
#include "cheat.h"
#include <string>
#include <vector>
#include <deque>
#include <fstream>

// ─────────────────────────────────────────────────────────────────────────────
// CBhop — perfect bunny-hop: press jump exactly on landing tick
// ─────────────────────────────────────────────────────────────────────────────
class CBhop : public CCheatModule
{
public:
    CBhop() : CCheatModule("Bhop") {}
    void OnTick() override;

    // Settings
    void SetAutoOnly(bool v) { m_AutoOnly = v; }   // only jump when space held
    bool AutoOnly() const    { return m_AutoOnly; }

private:
    bool m_AutoOnly = true;    // only bhop when space bar is held
    bool m_WasOnGround = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// CAutoStrafe — optimal air-strafing: adds small strafe input matching vel dir
// ─────────────────────────────────────────────────────────────────────────────
class CAutoStrafe : public CCheatModule
{
public:
    CAutoStrafe() : CCheatModule("AutoStrafe") {}
    void OnTick() override;

    void SetStrength(float v) { m_Strength = v; }
    float Strength() const    { return m_Strength; }

private:
    float m_Strength = 0.6f;   // 0..1 strength multiplier
};

// ─────────────────────────────────────────────────────────────────────────────
// CSpeedHack — velocity multiplier applied every tick
// ─────────────────────────────────────────────────────────────────────────────
class CSpeedHack : public CCheatModule
{
public:
    CSpeedHack() : CCheatModule("SpeedHack") {}
    void OnTick() override;

    void  SetMultiplier(float v) { m_Multiplier = v; }
    float Multiplier() const     { return m_Multiplier; }

private:
    float m_Multiplier = 1.5f;   // velocity scale (1.0 = normal)
};

// ─────────────────────────────────────────────────────────────────────────────
// CZoomHack — camera zoom in/out with configurable level
// ─────────────────────────────────────────────────────────────────────────────
class CZoomHack : public CCheatModule
{
public:
    CZoomHack() : CCheatModule("ZoomHack") {}
    void OnTick()   override;
    void OnRender() override;
    void OnInput(IInput *pInput) override;

    void  SetZoom(float v)   { m_Zoom = v; }
    float Zoom()       const { return m_Zoom; }
    void  SetKeyIn(int k)    { m_KeyIn = k; }
    void  SetKeyOut(int k)   { m_KeyOut = k; }

private:
    float m_Zoom    = 1.0f;    // 0.3 = very zoomed in, 2.0 = wide
    float m_Target  = 1.0f;
    int   m_KeyIn   = '+';
    int   m_KeyOut  = '-';
    bool  m_Applied = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// CGhostTrail — render last N positions as a fading colored trail
// ─────────────────────────────────────────────────────────────────────────────
class CGhostTrail : public CCheatModule
{
public:
    CGhostTrail() : CCheatModule("GhostTrail") {}
    void OnTick()   override;
    void OnRender() override;

    void SetLength(int v)  { m_MaxLen = v; }
    int  Length()    const { return m_MaxLen; }
    void SetWidth(float v) { m_Width = v; }

    // Color components
    float m_R = 0.4f, m_G = 0.8f, m_B = 1.0f;

private:
    int               m_MaxLen = 40;    // history frames
    float             m_Width  = 2.0f;
    std::deque<vec2>  m_Trail;
};

// ─────────────────────────────────────────────────────────────────────────────
// CKillSay — auto chat message on kill / death event
// ─────────────────────────────────────────────────────────────────────────────
class CKillSay : public CCheatModule
{
public:
    CKillSay() : CCheatModule("KillSay") {}
    void OnTick() override;

    void SetKillMsg(const std::string &s)  { m_KillMsg = s; }
    void SetDeathMsg(const std::string &s) { m_DeathMsg = s; }
    bool DoOnKill()   const { return m_OnKill; }
    bool DoOnDeath()  const { return m_OnDeath; }
    void SetOnKill(bool v)  { m_OnKill = v; }
    void SetOnDeath(bool v) { m_OnDeath = v; }

private:
    std::string m_KillMsg  = "ez";
    std::string m_DeathMsg = "gg";
    bool        m_OnKill   = true;
    bool        m_OnDeath  = false;
    int         m_LastKills = 0;
    int         m_LastDeaths= 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// CServerInfoHUD — overlay: server name, map, player count, your ping
// ─────────────────────────────────────────────────────────────────────────────
class CServerInfoHUD : public CCheatModule
{
public:
    CServerInfoHUD() : CCheatModule("ServerInfoHUD") {}
    void OnRender() override;

    // Position
    float m_X = 0.5f;   // 0..1 of screen width (anchored top-right by default)
    float m_Y = 0.01f;
};

// ─────────────────────────────────────────────────────────────────────────────
// CPlayerTracker — highlight and optionally follow a named player
// ─────────────────────────────────────────────────────────────────────────────
class CPlayerTracker : public CCheatModule
{
public:
    CPlayerTracker() : CCheatModule("PlayerTracker") {}
    void OnTick()   override;
    void OnRender() override;

    void SetTarget(const std::string &name) { m_TargetName = name; }
    const std::string &Target() const       { return m_TargetName; }
    void SetFollow(bool v)                  { m_Follow = v; }
    bool Follow()                     const { return m_Follow; }

private:
    std::string m_TargetName;
    bool        m_Follow    = false;
    int         m_TargetID  = -1;
};

// ─────────────────────────────────────────────────────────────────────────────
// CAutoLogin — auto-type stored password when server requests it
// ─────────────────────────────────────────────────────────────────────────────
class CAutoLogin : public CCheatModule
{
public:
    CAutoLogin() : CCheatModule("AutoLogin") {}
    void OnTick() override;

    void SetPassword(const std::string &pw) { m_Password = pw; }
    const std::string &Password()     const { return m_Password; }

private:
    std::string m_Password;
    bool        m_Sent = false;
    int         m_Delay = 20;    // ticks after connect before sending
    int         m_Timer = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// CSelfUnfreeze — автоматически размораживает игрока:
//   • Режим Hammer: бьёт хаммером по себе как только заморожен
//   • Режим Hook: отстреливает хук в сторону чтобы размотаться
//   • Режим Move: спамит движением для ускоренного размораживания
// ─────────────────────────────────────────────────────────────────────────────
class CSelfUnfreeze : public CCheatModule
{
public:
    CSelfUnfreeze() : CCheatModule("SelfUnfreeze") {}
    void OnTick() override;
    void OnRender() override;    // рисует таймер заморозки над головой

    // Режимы
    enum EMode {
        MODE_HAMMER = 0,   // бьём хаммером по себе
        MODE_HOOK   = 1,   // хук в противоположную сторону
        MODE_MOVE   = 2,   // спам движением
        MODE_LASER  = 3,   // стреляем лазером в стену чтобы отскочил в себя
        MODE_GUN    = 4,   // стреляем пистолетом по ближайшей поверхности
    };
    void SetMode(int m)      { m_Mode = m; }
    int  Mode()        const { return m_Mode; }

    // Показывать таймер заморозки над головой
    void SetShowTimer(bool v) { m_ShowTimer = v; }
    bool ShowTimer()    const { return m_ShowTimer; }

    // Для лазер-режима: автоматически найти лучший угол отскока
    void SetAutoAngle(bool v) { m_AutoAngle = v; }
    bool AutoAngle()    const { return m_AutoAngle; }

private:
    int  m_Mode      = MODE_LASER;  // лазер — самый надёжный по умолчанию
    bool m_ShowTimer = true;
    bool m_AutoAngle = true;        // автоматически вычислять угол отскока

    // внутреннее состояние
    int  m_HammerSpamTimer = 0;
    int  m_HookSpamTimer   = 0;
    int  m_LaserTimer      = 0;
    int  m_GunTimer        = 0;
    bool m_WasFrozen       = false;

    // вычислить угол для отскока лазера в себя
    // возвращает угол или -999.f если нет подходящей поверхности
    float CalcLaserBounceAngle(CGameClient *gc) const;

    void DoHammerUnfreeze(CGameClient *gc);
    void DoHookUnfreeze(CGameClient *gc);
    void DoMoveUnfreeze(CGameClient *gc);
    void DoLaserUnfreeze(CGameClient *gc);
    void DoGunUnfreeze(CGameClient *gc);
};

// ─────────────────────────────────────────────────────────────────────────────
// CChatLogger — save all incoming chat messages to file
// ─────────────────────────────────────────────────────────────────────────────
class CChatLogger : public CCheatModule
{
public:
    CChatLogger() : CCheatModule("ChatLogger") {}
    void SetEnabled(bool v) override;
    void OnTick() override;

    void SetPath(const std::string &path) { m_Path = path; }
    const std::string &Path() const       { return m_Path; }

private:
    std::string   m_Path      = "ddnet_cheat/chat.log";
    std::ofstream m_File;
    int           m_LastLine = -1;    // последний обработанный m_CurrentLine

    void OpenFile();
    void LogMessage(const char *pName, const char *pMsg, bool team);
};
