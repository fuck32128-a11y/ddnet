// cheat.h — master header, module registry, DDNet cheat client
// v2: added Extra module profile fields (Bhop, AutoStrafe, SpeedHack,
//     ZoomHack, GhostTrail, KillSay, ServerInfoHUD, PlayerTracker,
//     AutoLogin, ChatLogger)
#pragma once
#include <base/vmath.h>

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>

// Forward declarations
class CGameClient;
class CNetObj_Character;
struct CNetObj_PlayerInfo;
class CRenderTools;
class IGraphics;
class IInput;

// ─── Module base ──────────────────────────────────────────────────────────────

class CCheatModule
{
public:
    explicit CCheatModule(const char *pName) : m_Name(pName), m_Enabled(false) {}
    virtual ~CCheatModule() = default;

    virtual void OnTick()   {}
    virtual void OnRender() {}
    virtual void OnInput(IInput *pInput) {}

    const std::string &Name()   const { return m_Name; }
    bool              Enabled() const { return m_Enabled; }
    virtual void SetEnabled(bool v)   { m_Enabled = v; }
    void Toggle()                     { m_Enabled = !m_Enabled; }

protected:
    std::string m_Name;
    bool        m_Enabled;
};

// ─── Config / profile ─────────────────────────────────────────────────────────

struct CProfile
{
    std::string Name;

    // ── Aimbot ────────────────────────────────────────────────────────────────
    bool  AimBot_Enabled      = false;
    float AimBot_FOV          = 180.0f;
    bool  AimBot_AimAssist    = false;
    bool  AimBot_SilentAim    = false;
    bool  AimBot_AutoFire     = false;
    bool  AimBot_Prediction   = true;
    int   AimBot_TargetMode   = 0;

    // ── Hammer ───────────────────────────────────────────────────────────────
    bool  Hammer_Assist       = false;
    bool  Hammer_Auto         = false;
    bool  Hammer_Target       = false;
    bool  Hammer_Indicator    = true;
    float Hammer_Range        = 32.0f;

    // ── Hook ──────────────────────────────────────────────────────────────────
    bool  Hook_Auto           = false;
    bool  Hook_Assist         = false;
    bool  Hook_AutoJump       = false;
    bool  Hook_Prediction     = true;
    bool  Hook_FreezeIndicator= true;

    // ── ESP ───────────────────────────────────────────────────────────────────
    bool  ESP_Players         = true;
    bool  ESP_Entities        = false;
    bool  ESP_HookLines       = true;
    bool  ESP_Tracers         = false;
    bool  ESP_ExtRadar        = true;
    bool  ESP_DamageIndicators= true;
    float ESP_TracerAlpha     = 0.6f;

    // ── HUD ───────────────────────────────────────────────────────────────────
    bool  HUD_Custom          = true;
    bool  HUD_SpectatorList   = true;
    bool  HUD_Ping            = true;
    bool  HUD_FPS             = true;
    bool  HUD_Position        = true;
    bool  HUD_Speedometer     = true;
    bool  HUD_Input           = true;
    bool  HUD_Tick            = true;
    bool  HUD_PlayerInfo      = true;
    bool  HUD_VelocityDisplay = true;
    bool  HUD_FreezeIndicator = true;
    bool  HUD_Notifications   = true;

    // ── Misc ──────────────────────────────────────────────────────────────────
    bool  Misc_Spinbot        = false;
    float Misc_SpinSpeed      = 720.0f;
    bool  Misc_NameSpoof      = false;
    std::string Misc_SpoofedName = "nameless tee";
    bool  Misc_AntiPing       = false;
    bool  Misc_AutoShoot      = false;

    // ── Visual / Cosmetic ─────────────────────────────────────────────────────
    bool  Visual_FakeAim      = false;
    bool  Visual_RobotAim     = false;
    bool  Visual_RandomAim    = false;
    bool  Visual_FakeAngles   = false;
    bool  Visual_DrawAimLines = false;
    bool  Visual_DrawBulletLines = false;
    bool  Visual_RainbowTee   = false;
    bool  Visual_RainbowHook  = false;
    bool  Visual_SmoothCamera = false;
    bool  Visual_EmoteSpam    = false;
    bool  Visual_ModDetector  = true;
    bool  Visual_Watermark    = true;
    bool  Visual_DiscordRPC   = false;
    float Visual_RainbowSpeed = 2.0f;
    float Visual_CameraSmooth = 0.15f;
    int   Visual_EmoteID      = 0;
    int   Visual_EmoteInterval= 50;

    // ── Movement ──────────────────────────────────────────────────────────────
    bool  Mov_MoonWalk        = false;
    bool  Mov_AutoRehook      = false;
    bool  Mov_FallPrediction  = false;
    bool  Mov_AutoEdge        = false;
    bool  Mov_UnfreezeBot     = false;
    bool  Mov_DummyFly        = false;
    bool  Mov_JetRide         = false;
    bool  Mov_FlowfieldEditor = false;

    // ── TAS / Replay ──────────────────────────────────────────────────────────
    bool  TAS_ReplayManager   = false;
    bool  TAS_AutoRewind      = false;
    bool  TAS_AutoForward     = false;
    bool  TAS_AutoPause       = true;
    bool  TAS_PathPrediction  = true;
    bool  TAS_PathFeatures    = true;
    bool  TAS_Validation      = false;
    bool  TAS_PauseOnFreeze   = true;
    bool  TAS_PauseOnDeath    = true;
    int   TAS_RewindCheckpoint= 50;
    int   TAS_ForwardSpeed    = 4;

    // ── Extra (new features) ──────────────────────────────────────────────────
    bool  Extra_Bhop          = false;
    bool  Extra_BhopAutoOnly  = true;    // only while space held
    bool  Extra_AutoStrafe    = false;
    float Extra_StrafStrength = 0.6f;
    bool  Extra_SpeedHack     = false;
    float Extra_SpeedMult     = 1.5f;
    bool  Extra_ZoomHack      = false;
    float Extra_ZoomLevel     = 1.0f;
    bool  Extra_GhostTrail    = false;
    int   Extra_TrailLength   = 40;
    float Extra_TrailR        = 0.4f;
    float Extra_TrailG        = 0.8f;
    float Extra_TrailB        = 1.0f;
    bool  Extra_KillSay       = false;
    bool  Extra_KillSayOnKill = true;
    bool  Extra_KillSayOnDeath= false;
    std::string Extra_KillMsg  = "ez";
    std::string Extra_DeathMsg = "gg";
    bool  Extra_ServerInfoHUD = false;
    bool  Extra_PlayerTracker = false;
    std::string Extra_TrackerTarget;
    bool  Extra_TrackerFollow = false;
    bool  Extra_AutoLogin     = false;
    std::string Extra_LoginPw;
    bool  Extra_ChatLogger    = false;
    std::string Extra_ChatLogPath = "ddnet_cheat/chat.log";

    // Unfreeze
    bool  Extra_SelfUnfreeze     = false;
    int   Extra_UnfreezeMode     = 3;    // 0=Hammer 1=Hook 2=Move 3=Laser 4=Gun
    bool  Extra_UnfreezeTimer    = true; // показывать таймер заморозки
    bool  Extra_UnfreezeAutoAngle= true; // автоматически вычислять угол отскока лазера
};

// ─── Config manager ───────────────────────────────────────────────────────────

class CCheatConfigManager
{
public:
    CCheatConfigManager();

    void         LoadAll(const std::string &dir);
    void         SaveProfile(const CProfile &p, const std::string &dir);
    CProfile    *ActiveProfile()            { return &m_Profiles[m_Active]; }
    const CProfile *ActiveProfile()   const { return &m_Profiles[m_Active]; }
    void         SetActive(int idx)         { m_Active = idx; }
    int          ActiveIndex()        const { return m_Active; }
    int          Count()              const { return (int)m_Profiles.size(); }
    CProfile    &Get(int i)                 { return m_Profiles[i]; }
    void         AddProfile(const CProfile &p) { m_Profiles.push_back(p); }
    void         RemoveProfile(int idx);
    std::string  Dir()                const { return m_Dir; }

private:
    std::vector<CProfile> m_Profiles;
    int                   m_Active = 0;
    std::string           m_Dir;

    static CProfile ParseFile(const std::string &path);
    static void     WriteFile(const CProfile &p, const std::string &path);
};

// ─── Cheat controller (singleton) ─────────────────────────────────────────────

class CCheat
{
public:
    static CCheat &Get()
    {
        static CCheat s_Instance;
        return s_Instance;
    }

    void Init(CGameClient *pClient);
    void OnTick();
    void OnRender();
    void OnInput(IInput *pInput);

    CCheatConfigManager &Config()       { return m_Config; }
    CGameClient    *Client()       { return m_pClient; }

    CCheatModule *Module(const std::string &name);
    void BindKey(int key, std::function<void()> cb);

    bool MenuOpen() const { return m_MenuOpen; }
    void ApplyProfile();

private:
    CCheat() = default;
    CCheat(const CCheat &) = delete;

    CGameClient                 *m_pClient  = nullptr;
    CCheatConfigManager               m_Config;
    std::vector<std::unique_ptr<CCheatModule>> m_Modules;
    std::vector<std::pair<int, std::function<void()>>> m_KeyBinds;
    bool                         m_MenuOpen = false;
    int                          m_MenuKey  = 'M';

    void RegisterModules();
};
