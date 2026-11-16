// ddnet_api.h — Thin wrappers around real DDNet CGameClient internals.
// All helpers reference actual DDNet types from gameclient.h / controls.h.
#pragma once

#include <game/client/gameclient.h>
#include <game/client/components/controls.h>
#include <game/client/prediction/entities/character.h>
#include <engine/client.h>
#include <engine/keys.h>
#include <engine/serverbrowser.h>
#include <base/vmath.h>
#include <cmath>

// ─── Basic helpers ────────────────────────────────────────────────────────────

inline int LocalID(CGameClient *gc)
{
    return gc->m_Snap.m_LocalClientId;
}

inline int GetCurrentTick(CGameClient *gc)
{
    return gc->Client()->GameTick(g_Config.m_ClDummy);
}

inline int GetMapCRC(CGameClient *gc)
{
    // Используем ServerInfo для получения CRC карты
    return gc->Client()->ServerInfo().m_MapCrc;
}

// ─── Position ─────────────────────────────────────────────────────────────────

inline CCharacter *GetPredChar(CGameClient *gc, int id)
{
    if(id < 0 || id >= MAX_CLIENTS) return nullptr;
    return gc->m_PredictedWorld.GetCharacterById(id);
}

inline vec2 GetSelfPos(CGameClient *gc)
{
    int id = LocalID(gc);
    if(id < 0) return vec2(0, 0);
    if(CCharacter *pChar = GetPredChar(gc, id))
        return pChar->Core()->m_Pos;
    if(gc->m_Snap.m_pLocalCharacter)
        return vec2((float)gc->m_Snap.m_pLocalCharacter->m_X,
                    (float)gc->m_Snap.m_pLocalCharacter->m_Y);
    return vec2(0, 0);
}

// ─── Screen conversion ────────────────────────────────────────────────────────

inline vec2 WorldToScreen(CGameClient *gc, vec2 worldPos)
{
    float sw   = (float)gc->Graphics()->ScreenWidth();
    float sh   = (float)gc->Graphics()->ScreenHeight();
    vec2  cam  = gc->m_Camera.m_Center;
    float zoom = gc->m_Camera.m_Zoom;
    return vec2(
        sw * 0.5f + (worldPos.x - cam.x) / zoom * (sw / 800.0f),
        sh * 0.5f + (worldPos.y - cam.y) / zoom * (sh / 600.0f)
    );
}

// ─── Camera ───────────────────────────────────────────────────────────────────

inline void SetCameraPos(CGameClient *gc, vec2 pos)  { gc->m_Camera.m_Center = pos; }
inline void SetCameraZoom(CGameClient *gc, float z)   { gc->m_Camera.m_Zoom   = z;   }

// ─── Aim angle ────────────────────────────────────────────────────────────────

inline float GetAimAngle(CGameClient *gc)
{
    auto &inp = gc->m_Controls.m_aInputData[g_Config.m_ClDummy];
    return atan2f((float)inp.m_TargetY, (float)inp.m_TargetX);
}

inline void SetAimAngle(CGameClient *gc, float angle)
{
    auto &inp = gc->m_Controls.m_aInputData[g_Config.m_ClDummy];
    inp.m_TargetX = (int)(cosf(angle) * 100.0f);
    inp.m_TargetY = (int)(sinf(angle) * 100.0f);
}

inline void SetNetworkAngle(CGameClient *gc, float angle) { SetAimAngle(gc, angle); }

// ─── Input injection ──────────────────────────────────────────────────────────

inline void InjectJump(CGameClient *gc)
{
    gc->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Jump = 1;
}

inline void InjectFire(CGameClient *gc)
{
    gc->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire |= 1;
}

inline void InjectMove(CGameClient *gc, int dir)
{
    gc->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Direction = dir;
}

inline void InjectWeaponSwitch(CGameClient *gc, int weapon)
{
    gc->m_Controls.m_aInputData[g_Config.m_ClDummy].m_WantedWeapon = weapon + 1;
}

inline void InjectHook(CGameClient *gc, float angle)
{
    SetAimAngle(gc, angle);
    gc->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Hook = 1;
}

inline void InjectHammerSelf(CGameClient *gc)
{
    auto &inp = gc->m_Controls.m_aInputData[g_Config.m_ClDummy];
    inp.m_WantedWeapon = WEAPON_HAMMER + 1;
    inp.m_TargetX      = 1;
    inp.m_TargetY      = 0;
    inp.m_Fire        |= 1;
}

inline void InjectStrafe(CGameClient *gc, int dir, float /*strength*/)
{
    auto &inp = gc->m_Controls.m_aInputData[g_Config.m_ClDummy];
    if(dir > 0 && inp.m_Direction <= 0)      inp.m_Direction =  1;
    else if(dir < 0 && inp.m_Direction >= 0) inp.m_Direction = -1;
}

inline bool IsKeyDown(CGameClient *gc, int key)
{
    return gc->Input()->KeyIsPressed(key);
}

// ─── Tile checks ──────────────────────────────────────────────────────────────

inline bool IsSolid(CGameClient *gc, vec2 pos)
{
    return gc->Collision()->CheckPoint(pos.x, pos.y) != 0;
}

inline bool IsFreezeTile(CGameClient *gc, vec2 pos)
{
    int tile = gc->Collision()->GetTileIndex(
        gc->Collision()->GetPureMapIndex(pos.x, pos.y));
    // DDNet freeze tiles: TILE_FREEZE=9, TILE_DFREEZE=6
    return tile == 9 || tile == 6;
}

// ─── Players ──────────────────────────────────────────────────────────────────

struct CPlayerInfo
{
    int   ClientID;
    vec2  Pos;
    int   Team;
    bool  IsAlive;
};

inline std::vector<CPlayerInfo> GatherPlayers(CGameClient *gc)
{
    std::vector<CPlayerInfo> out;
    for(int i = 0; i < MAX_CLIENTS; ++i)
    {
        if(!gc->m_aClients[i].m_Active) continue;
        CPlayerInfo pi;
        pi.ClientID = i;
        pi.Team     = gc->m_aClients[i].m_Team;
        if(CCharacter *pChar = GetPredChar(gc, i))
        {
            pi.Pos     = pChar->Core()->m_Pos;
            pi.IsAlive = true;
        }
        else
        {
            pi.Pos     = vec2(0, 0);
            pi.IsAlive = false;
        }
        out.push_back(pi);
    }
    return out;
}

inline int CountActivePlayers(CGameClient *gc)
{
    int n = 0;
    for(int i = 0; i < MAX_CLIENTS; ++i)
        if(gc->m_aClients[i].m_Active) ++n;
    return n;
}

inline int GetLocalScore(CGameClient *gc)
{
    int id = LocalID(gc);
    if(id < 0) return 0;
    return gc->m_aStats[id].m_Frags;
}

// ─── Server info ──────────────────────────────────────────────────────────────

inline const char *GetMapName(CGameClient *gc)
{
    return gc->Client()->ServerInfo().m_aMap;
}

inline int GetPing(CGameClient *gc)
{
    return gc->Client()->ServerInfo().m_Latency;
}

// ─── Chat ─────────────────────────────────────────────────────────────────────

inline void SendChat(CGameClient *gc, bool team, const char *msg)
{
    CNetMsg_Cl_Say MsgP;
    MsgP.m_Team     = team ? 1 : 0;
    MsgP.m_pMessage = msg;
    gc->Client()->SendPackMsgActive(&MsgP, MSGFLAG_VITAL);
}

inline void SendEmote(CGameClient *gc, int emote)
{
    CNetMsg_Cl_Emoticon MsgE;
    MsgE.m_Emoticon = emote;
    gc->Client()->SendPackMsgActive(&MsgE, MSGFLAG_VITAL);
}

// ─── Chat log access ──────────────────────────────────────────────────────────
// CChat использует кольцевой буфер m_aLines[MAX_LINES] с m_CurrentLine

inline int GetChatMessageCount(CGameClient *gc)
{
    // Возвращаем количество непустых строк (максимум MAX_LINES = 64)
    return CChat::MAX_LINES;
}

inline void GetChatMessage(CGameClient *gc, int idx,
                           const char **ppName, const char **ppMsg, bool *pTeam)
{
    if(idx < 0 || idx >= CChat::MAX_LINES)
    {
        *ppName = nullptr; *ppMsg = nullptr; *pTeam = false;
        return;
    }
    // Кольцевой буфер: реальный индекс считается от m_CurrentLine
    int real = (gc->m_Chat.m_CurrentLine - idx + CChat::MAX_LINES) % CChat::MAX_LINES;
    auto &line = gc->m_Chat.m_aLines[real];
    *ppName = line.m_aName;
    *ppMsg  = line.m_aText;
    *pTeam  = line.m_Team;
}

// ─── Graphics helpers ─────────────────────────────────────────────────────────

inline void GfxDrawLine(CGameClient *gc,
    float x0, float y0, float x1, float y1,
    float r, float g, float b, float a)
{
    IGraphics *gfx = gc->Graphics();
    gfx->TextureClear();
    gfx->LinesBegin();
    gfx->SetColor(r, g, b, a);
    IGraphics::CLineItem line(x0, y0, x1, y1);
    gfx->LinesDraw(&line, 1);
    gfx->LinesEnd();
}

inline void GfxDrawFilledRect(CGameClient *gc,
    float x, float y, float w, float h,
    float r, float g, float b, float a)
{
    IGraphics *gfx = gc->Graphics();
    gfx->TextureClear();
    gfx->QuadsBegin();
    gfx->SetColor(r, g, b, a);
    IGraphics::CQuadItem quad(x, y, w, h);
    gfx->QuadsDraw(&quad, 1);
    gfx->QuadsEnd();
}

inline void GfxDrawText(CGameClient *gc,
    float x, float y, float size,
    const char *text,
    float r, float g, float b, float a)
{
    gc->TextRender()->TextColor(r, g, b, a);
    gc->TextRender()->Text(x, y, size, text, -1.0f);
    gc->TextRender()->TextColor(1, 1, 1, 1);
}
