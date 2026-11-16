// extra_impl.cpp — New modules: Bhop, AutoStrafe, SpeedHack, ZoomHack,
//                  GhostTrail, KillSay, ServerInfoHUD, PlayerTracker,
//                  AutoLogin, ChatLogger, SelfUnfreeze
#include "extra.h"
#include "cheat.h"
#include "ddnet_api.h"
#include <cmath>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <algorithm>

static constexpr float PI = 3.14159265f;

// GetPredChar — объявлена в ddnet_api.h

// ─────────────────────────────────────────────────────────────────────────────
// CBhop — perfect bunny hop
// ─────────────────────────────────────────────────────────────────────────────
// Инжектируем Jump точно на тик приземления.
// AutoOnly=true: только когда игрок держит пробел.

void CBhop::OnTick()
{
    if(!Enabled()) return;
    CGameClient *gc = CCheat::Get().Client();
    int selfID = LocalID(gc);
    CCharacter *pChar = GetPredChar(gc, selfID);
    if(!pChar) return;

    // Определяем «на земле»: хук в нейтральном состоянии и velocity.y ≈ 0
    // В DDNet CCharacterCore::m_Jumped == 0 — на земле
    bool onGround = (pChar->Core()->m_Jumped == 0);

    // KEY_SPACE нет в DDNet — проверяем через Raw input (ключ 44 = SDL_SCANCODE_SPACE)
    // Используем стандартный KEY_SPACE (32 в ASCII, но в DDNet это scancode 44)
    bool spaceHeld = m_AutoOnly
        ? gc->Input()->KeyIsPressed(KEY_SPACE)
        : true;

    if(onGround && !m_WasOnGround && spaceHeld)
        InjectJump(gc);

    m_WasOnGround = onGround;
}

// ─────────────────────────────────────────────────────────────────────────────
// CAutoStrafe
// ─────────────────────────────────────────────────────────────────────────────

void CAutoStrafe::OnTick()
{
    if(!Enabled()) return;
    CGameClient *gc = CCheat::Get().Client();
    int selfID = LocalID(gc);
    CCharacter *pChar = GetPredChar(gc, selfID);
    if(!pChar) return;

    // Только в воздухе
    bool onGround = (pChar->Core()->m_Jumped == 0);
    if(onGround) return;

    float vx       = pChar->Core()->m_Vel.x;
    float vy       = pChar->Core()->m_Vel.y;
    float aimAngle = GetAimAngle(gc);

    float aimDx = cosf(aimAngle), aimDy = sinf(aimAngle);
    float cross  = vx * aimDy - vy * aimDx;

    if(fabsf(cross) < 0.5f) return;
    int strafe = (cross > 0.0f) ? 1 : -1;
    InjectStrafe(gc, strafe, m_Strength);
}

// ─────────────────────────────────────────────────────────────────────────────
// CSpeedHack — масштабирует локальную скорость персонажа
// ─────────────────────────────────────────────────────────────────────────────

void CSpeedHack::OnTick()
{
    if(!Enabled()) return;
    CGameClient *gc = CCheat::Get().Client();
    int selfID = LocalID(gc);
    CCharacter *pChar = GetPredChar(gc, selfID);
    if(!pChar) return;

    CCharacterCore core = pChar->GetCore();
    core.m_Vel.x *= m_Multiplier;
    core.m_Vel.y *= m_Multiplier;
    pChar->SetCore(core);
}

// ─────────────────────────────────────────────────────────────────────────────
// CZoomHack
// ─────────────────────────────────────────────────────────────────────────────

void CZoomHack::OnInput(IInput *pInput)
{
    if(!Enabled()) return;
    if(pInput->KeyIsPressed(m_KeyIn))
        m_Target = std::max(0.3f, m_Target - 0.05f);
    if(pInput->KeyIsPressed(m_KeyOut))
        m_Target = std::min(2.5f, m_Target + 0.05f);
}

void CZoomHack::OnTick()
{
    if(!Enabled())
    {
        if(m_Applied) { SetCameraZoom(CCheat::Get().Client(), 1.0f); m_Applied = false; }
        return;
    }
    m_Zoom += (m_Target - m_Zoom) * 0.15f;
}

void CZoomHack::OnRender()
{
    if(!Enabled()) return;
    CGameClient *gc = CCheat::Get().Client();
    SetCameraZoom(gc, m_Zoom);
    m_Applied = true;

    char buf[32];
    snprintf(buf, sizeof(buf), "zoom %.2fx", m_Zoom);
    float sc = (float)gc->Graphics()->ScreenHeight() / 1080.0f;
    GfxDrawText(gc, 8.0f*sc, 8.0f*sc, 10.0f*sc, buf, 0.9f, 0.9f, 0.3f, 0.7f);
}

// ─────────────────────────────────────────────────────────────────────────────
// CGhostTrail
// ─────────────────────────────────────────────────────────────────────────────

void CGhostTrail::OnTick()
{
    if(!Enabled()) { m_Trail.clear(); return; }
    CGameClient *gc = CCheat::Get().Client();
    vec2 pos = GetSelfPos(gc);
    m_Trail.push_front(pos);
    while((int)m_Trail.size() > m_MaxLen)
        m_Trail.pop_back();
}

void CGhostTrail::OnRender()
{
    if(!Enabled() || m_Trail.size() < 2) return;
    CGameClient *gc = CCheat::Get().Client();

    for(int i = 0; i + 1 < (int)m_Trail.size(); ++i)
    {
        float t0 = 1.0f - (float)i / (float)m_Trail.size();
        vec2 s0 = WorldToScreen(gc, m_Trail[i]);
        vec2 s1 = WorldToScreen(gc, m_Trail[i+1]);
        GfxDrawLine(gc, s0.x, s0.y, s1.x, s1.y, m_R, m_G, m_B, t0 * 0.75f);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// CKillSay
// ─────────────────────────────────────────────────────────────────────────────

void CKillSay::OnTick()
{
    if(!Enabled()) return;
    CGameClient *gc = CCheat::Get().Client();
    int selfID = LocalID(gc);

    bool isDead = (GetPredChar(gc, selfID) == nullptr);

    if(m_OnDeath && isDead && m_LastDeaths == 0)
    {
        SendChat(gc, false, m_DeathMsg.c_str());
        m_LastDeaths = 1;
    }
    else if(!isDead)
    {
        m_LastDeaths = 0;

        if(m_OnKill)
        {
            int score = GetLocalScore(gc);
            if(score > m_LastKills)
            {
                SendChat(gc, false, m_KillMsg.c_str());
                m_LastKills = score;
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// CServerInfoHUD
// ─────────────────────────────────────────────────────────────────────────────

void CServerInfoHUD::OnRender()
{
    if(!Enabled()) return;
    CGameClient *gc = CCheat::Get().Client();

    float sw = (float)gc->Graphics()->ScreenWidth();
    float sh = (float)gc->Graphics()->ScreenHeight();
    float sc = sh / 1080.0f;

    // Используем доступные данные из снэпшота
    int   players = CountActivePlayers(gc);
    int   ping    = GetPing(gc);

    char line1[256], line2[64], line3[64];
    snprintf(line1, sizeof(line1), "Map: %s", GetMapName(gc));
    snprintf(line2, sizeof(line2), "Players: %d", players);
    snprintf(line3, sizeof(line3), "Ping: %d ms", ping);

    float x  = sw * 0.72f;
    float y  = sh * m_Y;
    float fs = 11.0f * sc;

    GfxDrawText(gc, x, y,           fs, line1, 0.9f, 0.9f, 0.9f, 0.85f);
    GfxDrawText(gc, x, y + fs*1.4f, fs, line2, 0.7f, 0.9f, 0.7f, 0.85f);
    GfxDrawText(gc, x, y + fs*2.8f, fs, line3, 0.9f, 0.7f, 0.7f, 0.85f);
}

// ─────────────────────────────────────────────────────────────────────────────
// CPlayerTracker
// ─────────────────────────────────────────────────────────────────────────────

void CPlayerTracker::OnTick()
{
    if(!Enabled()) return;
    if(m_TargetName.empty()) return;

    CGameClient *gc = CCheat::Get().Client();
    m_TargetID = -1;

    for(int i = 0; i < MAX_CLIENTS; ++i)
    {
        if(!gc->m_aClients[i].m_Active) continue;
        if(m_TargetName == gc->m_aClients[i].m_aName)
        {
            m_TargetID = i;
            break;
        }
    }

    if(m_Follow && m_TargetID >= 0)
    {
        CCharacter *pChar = GetPredChar(gc, m_TargetID);
        if(pChar)
            SetCameraPos(gc, pChar->Core()->m_Pos);
    }
}

void CPlayerTracker::OnRender()
{
    if(!Enabled() || m_TargetID < 0) return;
    CGameClient *gc = CCheat::Get().Client();
    CCharacter *pChar = GetPredChar(gc, m_TargetID);
    if(!pChar) return;

    vec2 pos = pChar->Core()->m_Pos;
    vec2 scr = WorldToScreen(gc, pos);

    float sc = (float)gc->Graphics()->ScreenHeight() / 1080.0f;
    float sw = (float)gc->Graphics()->ScreenWidth();
    float sh = (float)gc->Graphics()->ScreenHeight();

    bool offscreen = scr.x < 0 || scr.x > sw || scr.y < 0 || scr.y > sh;

    if(offscreen)
    {
        vec2  center = {sw * 0.5f, sh * 0.5f};
        float angle  = atan2f(scr.y - center.y, scr.x - center.x);
        float r      = std::min(sw, sh) * 0.45f;
        float ax     = center.x + cosf(angle) * r;
        float ay     = center.y + sinf(angle) * r;
        for(int i = -2; i <= 2; ++i)
        {
            float a2 = angle + i * 0.3f;
            GfxDrawLine(gc, ax, ay,
                        ax - cosf(a2)*12.0f*sc,
                        ay - sinf(a2)*12.0f*sc,
                        1.0f, 0.9f, 0.2f, 0.9f);
        }
    }
    else
    {
        float hs = 24.0f * sc;
        GfxDrawLine(gc, scr.x-hs, scr.y-hs, scr.x+hs, scr.y-hs, 1.0f,0.9f,0.2f,0.8f);
        GfxDrawLine(gc, scr.x+hs, scr.y-hs, scr.x+hs, scr.y+hs, 1.0f,0.9f,0.2f,0.8f);
        GfxDrawLine(gc, scr.x+hs, scr.y+hs, scr.x-hs, scr.y+hs, 1.0f,0.9f,0.2f,0.8f);
        GfxDrawLine(gc, scr.x-hs, scr.y+hs, scr.x-hs, scr.y-hs, 1.0f,0.9f,0.2f,0.8f);

        GfxDrawText(gc, scr.x - hs, scr.y - hs - 14.0f*sc,
                    10.0f*sc, m_TargetName.c_str(), 1.0f,0.9f,0.2f,0.9f);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// CAutoLogin
// ─────────────────────────────────────────────────────────────────────────────

void CAutoLogin::OnTick()
{
    if(!Enabled() || m_Password.empty()) return;
    CGameClient *gc = CCheat::Get().Client();

    static int s_LastMapCrc = 0;
    int mapCrc = GetMapCRC(gc);
    if(mapCrc != s_LastMapCrc)
    {
        s_LastMapCrc = mapCrc;
        m_Sent  = false;
        m_Timer = 0;
    }

    if(m_Sent) return;

    ++m_Timer;
    if(m_Timer < m_Delay) return;

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "/login %s", m_Password.c_str());
    SendChat(gc, false, cmd);
    m_Sent = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// CChatLogger
// ─────────────────────────────────────────────────────────────────────────────

void CChatLogger::SetEnabled(bool v)
{
    CCheatModule::SetEnabled(v);
    if(v) OpenFile(); else m_File.close();
}

void CChatLogger::OpenFile()
{
    m_File.open(m_Path, std::ios::app);
    if(m_File.is_open())
    {
        time_t t = time(nullptr);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
        m_File << "\n=== Session started: " << buf << " ===\n";
        m_File.flush();
    }
}

void CChatLogger::OnTick()
{
    if(!Enabled() || !m_File.is_open()) return;
    CGameClient *gc = CCheat::Get().Client();

    // Слежём за m_CurrentLine чтобы поймать новые сообщения в кольцевом буфере
    int cur = gc->m_Chat.m_CurrentLine;
    if(cur != m_LastLine)
    {
        // Вычисляем сколько новых сообщений появилось
        int diff = (cur - m_LastLine + CChat::MAX_LINES) % CChat::MAX_LINES;
        for(int i = diff; i > 0; --i)
        {
            const char *pName = nullptr;
            const char *pMsg  = nullptr;
            bool        team  = false;
            GetChatMessage(gc, i - 1, &pName, &pMsg, &team);
            if(pName && pMsg && pName[0] != '\0')
                LogMessage(pName, pMsg, team);
        }
        m_LastLine = cur;
    }
}

void CChatLogger::LogMessage(const char *pName, const char *pMsg, bool team)
{
    time_t t = time(nullptr);
    char timebuf[32];
    strftime(timebuf, sizeof(timebuf), "%H:%M:%S", localtime(&t));
    m_File << "[" << timebuf << "] ";
    if(team) m_File << "[TEAM] ";
    m_File << pName << ": " << pMsg << "\n";
    m_File.flush();
}

// ─────────────────────────────────────────────────────────────────────────────
// CSelfUnfreeze
// ─────────────────────────────────────────────────────────────────────────────

void CSelfUnfreeze::OnTick()
{
    if(!Enabled()) return;
    CGameClient *gc = CCheat::Get().Client();
    int selfID = LocalID(gc);
    CCharacter *pChar = GetPredChar(gc, selfID);
    if(!pChar) return;

    // Определяем заморозку через m_FreezeEnd
    bool frozen = (pChar->Core()->m_FreezeEnd > GetCurrentTick(gc));

    if(!frozen)
    {
        m_WasFrozen       = false;
        m_HammerSpamTimer = 0;
        m_HookSpamTimer   = 0;
        m_LaserTimer      = 0;
        m_GunTimer        = 0;
        return;
    }

    m_WasFrozen = true;

    switch(m_Mode)
    {
    case MODE_HAMMER: DoHammerUnfreeze(gc); break;
    case MODE_HOOK:   DoHookUnfreeze(gc);   break;
    case MODE_MOVE:   DoMoveUnfreeze(gc);   break;
    case MODE_LASER:  DoLaserUnfreeze(gc);  break;
    case MODE_GUN:    DoGunUnfreeze(gc);    break;
    default: break;
    }
}

void CSelfUnfreeze::DoHammerUnfreeze(CGameClient *gc)
{
    ++m_HammerSpamTimer;
    if(m_HammerSpamTimer < 3) return;
    m_HammerSpamTimer = 0;
    InjectHammerSelf(gc);
}

void CSelfUnfreeze::DoHookUnfreeze(CGameClient *gc)
{
    ++m_HookSpamTimer;
    if(m_HookSpamTimer < 15) return;
    m_HookSpamTimer = 0;

    CCharacter *pChar = GetPredChar(gc, LocalID(gc));
    if(!pChar) return;

    float vlen = sqrtf(pChar->Core()->m_Vel.x * pChar->Core()->m_Vel.x +
                       pChar->Core()->m_Vel.y * pChar->Core()->m_Vel.y);
    float angle;
    if(vlen > 0.5f)
        angle = atan2f(-pChar->Core()->m_Vel.y, -pChar->Core()->m_Vel.x);
    else
        angle = PI * 0.5f;

    InjectHook(gc, angle);
}

void CSelfUnfreeze::DoMoveUnfreeze(CGameClient *gc)
{
    static int s_Dir = 1, s_Timer = 0;
    ++s_Timer;
    if(s_Timer >= 2) { s_Dir = -s_Dir; s_Timer = 0; }
    InjectMove(gc, s_Dir);
}

// ─── Laser unfreeze ───────────────────────────────────────────────────────────

float CSelfUnfreeze::CalcLaserBounceAngle(CGameClient *gc) const
{
    int selfID = LocalID(gc);
    CCharacter *pChar = GetPredChar(gc, selfID);
    if(!pChar) return -999.f;
    vec2 selfPos = GetSelfPos(gc);

    const float MAX_DIST = 300.0f;
    const int   NUM_DIRS = 16;

    float bestAngle = -999.f;
    float bestDist  = 1e9f;

    for(int i = 0; i < NUM_DIRS; ++i)
    {
        float shootAngle = (float)i / NUM_DIRS * 2.0f * PI;
        vec2  dir = { cosf(shootAngle), sinf(shootAngle) };

        // Трассируем луч до стены
        float wallDist = 0.0f;
        vec2  wallPos  = selfPos;
        for(float d = 8.0f; d <= MAX_DIST; d += 8.0f)
        {
            vec2 p = { selfPos.x + dir.x * d, selfPos.y + dir.y * d };
            if(IsSolid(gc, p))
            {
                wallDist = d;
                wallPos  = p;
                break;
            }
        }
        if(wallDist < 8.0f) continue;

        // Вычисляем угол отражения (reflection)
        vec2 normals[2] = { {0, -dir.y > 0 ? 1.f : -1.f},
                            {-dir.x > 0 ? 1.f : -1.f, 0} };
        for(auto &n : normals)
        {
            float dotN = dir.x*n.x + dir.y*n.y;
            vec2  r    = { dir.x - 2*dotN*n.x, dir.y - 2*dotN*n.y };

            vec2 toSelf = { selfPos.x - wallPos.x, selfPos.y - wallPos.y };
            float len   = sqrtf(toSelf.x*toSelf.x + toSelf.y*toSelf.y);
            if(len < 1.0f) continue;
            toSelf.x /= len; toSelf.y /= len;
            float dot2 = r.x*toSelf.x + r.y*toSelf.y;
            if(dot2 > 0.85f && wallDist < bestDist)
            {
                bestDist  = wallDist;
                bestAngle = shootAngle;
                break;
            }
        }
    }

    // Fallback: стреляем вниз (стандартный 1-tick unfreeze)
    if(bestAngle < -900.0f)
        bestAngle = PI * 0.5f;

    return bestAngle;
}

void CSelfUnfreeze::DoLaserUnfreeze(CGameClient *gc)
{
    ++m_LaserTimer;
    if(m_LaserTimer < 10) return;
    m_LaserTimer = 0;

    float angle = m_AutoAngle
        ? CalcLaserBounceAngle(gc)
        : PI * 0.5f;

    if(angle < -900.0f) return;

    InjectWeaponSwitch(gc, WEAPON_LASER);
    SetAimAngle(gc, angle);
    InjectFire(gc);
}

void CSelfUnfreeze::DoGunUnfreeze(CGameClient *gc)
{
    ++m_GunTimer;
    if(m_GunTimer < 5) return;
    m_GunTimer = 0;

    CCharacter *pChar = GetPredChar(gc, LocalID(gc));
    if(!pChar) return;
    vec2 selfPos = GetSelfPos(gc);

    float bestAngle = PI * 0.5f;
    float bestDist  = 1e9f;
    for(int dy = -3; dy <= 3; ++dy)
    for(int dx = -3; dx <= 3; ++dx)
    {
        vec2 tp = { selfPos.x + dx*32.0f, selfPos.y + dy*32.0f };
        if(IsFreezeTile(gc, tp))
        {
            float d = sqrtf((float)(dx*dx + dy*dy));
            if(d < bestDist)
            {
                bestDist  = d;
                bestAngle = atan2f(-(float)dy, -(float)dx);
            }
        }
    }

    InjectWeaponSwitch(gc, WEAPON_GUN);
    SetAimAngle(gc, bestAngle);
    InjectFire(gc);
}

void CSelfUnfreeze::OnRender()
{
    if(!Enabled() || !m_ShowTimer) return;
    CGameClient *gc = CCheat::Get().Client();
    int selfID = LocalID(gc);
    CCharacter *pChar = GetPredChar(gc, selfID);
    if(!pChar) return;

    int curTick = GetCurrentTick(gc);
    int end     = pChar->Core()->m_FreezeEnd;
    if(end <= curTick) return;

    int   left = end - curTick;
    float secs = left / 50.0f;   // DDNet = 50 Hz

    vec2 pos = GetSelfPos(gc);
    pos.y -= 32.0f;
    vec2 scr = WorldToScreen(gc, pos);

    char buf[32];
    snprintf(buf, sizeof(buf), "%.1fs", secs);

    float sc    = (float)gc->Graphics()->ScreenHeight() / 1080.0f;
    float ratio = std::min(1.0f, secs / 3.0f);
    float r = 1.0f - ratio, g = ratio, b = 0.2f;
    GfxDrawText(gc, scr.x - 12.0f*sc, scr.y, 11.0f*sc, buf, r, g, b, 1.0f);
}
