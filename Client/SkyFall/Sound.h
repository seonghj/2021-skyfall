#pragma once
#ifndef _CSOUND_H_
#define _CSOUND_H_

#include "../fmod/FMOD/inc/fmod.h"
#define SOUND_MAX 1.0f
#define SOUND_MIN 0.0f
#define SOUND_DEFAULT 0.25f
#define SOUND_WEIGHT 0.1f

#define SE_NUM 5
#define BGM_NUM 4

enum SE {
    Walk = 0,
    Monster_Hit,
    Arrow_Hit,
    Knife_Attack,
    Spear_Attack,
};

enum BGM {
    FinalBoss = 0,
    Poisoned_Rose,
    The_Battle_of_1066,
    Gothic_Vigilante,
};
class CSound {
private:
    static FMOD_SYSTEM* g_sound_system;

    FMOD_SOUND* m_sound;
    FMOD_CHANNEL* m_channel;

    float m_beforevolume;
    float m_volume;
    FMOD_BOOL m_bool;
public:
    CSound(const char* path, bool loop);
    ~CSound();

    static int Init();
    static int Release();

    int play();
    int pause();
    int resume();
    int stop();
    int volumeUp();
    int volumeDown();
    void turnoff();
    void turnon();

    int Update();
    bool playing = false;
};

#endif