#ifndef EFFECTSEDITOR_H
#define EFFECTSEDITOR_H

#include <zyn.common/globals.h>
#include "applicationsession.h"

enum class ParamIndices
{
    None = 0,

    // Mixer
    Mixer_Volume = 1,
    Mixer_Panning = 2,

    // System FX
    SystemFX_1_Volume = 1001,
    SystemFX_2_Volume = 1002,
    SystemFX_3_Volume = 1003,
    SystemFX_4_Volume = 1004,

    // Insert FX

    // Track FX
    TrackFX_1_Reverb_Volume = 3101,
    TrackFX_1_Reverb_Pan = 3102,
    TrackFX_1_Reverb_Time = 3102,
    TrackFX_1_Reverb_InitialDelay = 3103,
    TrackFX_1_Reverb_InitialDelayFeedback = 3104,
    TrackFX_1_Reverb_LowPass = 3105,
    TrackFX_1_Reverb_HighPass = 3106,
    TrackFX_1_Reverb_Damp = 3107,

    TrackFX_1_Echo_Volume = 3111,
    TrackFX_1_Echo_Pan = 3112,
    TrackFX_1_Echo_Delay = 3112,
    TrackFX_1_Echo_LeftRightDelay = 3113,
    TrackFX_1_Echo_CrossingDelay = 3114,
    TrackFX_1_Echo_Feedback = 3115,
    TrackFX_1_Echo_Damp = 3116,

    TrackFX_1_Chorus_Volume = 3121,
    TrackFX_1_Chorus_Pan = 3122,
    TrackFX_1_Chorus_LFOFrequency  = 3122,
    TrackFX_1_Chorus_LFORandomness = 3123,
    TrackFX_1_Chorus_LeftRightPhaseDifference = 3124,
    TrackFX_1_Chorus_LFODepth = 3125,
    TrackFX_1_Chorus_Delay = 3126,
    TrackFX_1_Chorus_Feedback = 3127,
    TrackFX_1_Chorus_Subtract = 3129,

    TrackFX_1_Phaser_Volume = 3131,
    TrackFX_1_Phaser_Pan = 3132,
    TrackFX_1_Phaser_LFOFrequency  = 3132,
    TrackFX_1_Phaser_LFORandomness = 3133,
    TrackFX_1_Phaser_LeftRightPhaseDifference = 3134,
    TrackFX_1_Phaser_LFODepth = 3135,
    TrackFX_1_Phaser_Stages = 3136,
    TrackFX_1_Phaser_Feedback = 3137,
    TrackFX_1_Phaser_Subtract = 3139,

    TrackFX_1_AlienWah_Volume = 3141,
    TrackFX_1_AlienWah_Pan = 3142,
    TrackFX_1_AlienWah_LFOFrequency  = 3142,
    TrackFX_1_AlienWah_LFORandomness = 3143,
    TrackFX_1_AlienWah_LeftRightPhaseDifference = 3144,
    TrackFX_1_AlienWah_LFODepth = 3145,
    TrackFX_1_AlienWah_Delay = 3146,
    TrackFX_1_AlienWah_Feedback = 3147,
    TrackFX_1_AlienWah_Subtract = 3148,
    TrackFX_1_AlienWah_Phase = 3149,

    TrackFX_1_Distorsion_Volume = 3151,
    TrackFX_1_Distorsion_Pan = 3152,
    TrackFX_1_Distorsion_Drive  = 3152,
    TrackFX_1_Distorsion_Level = 3153,
    TrackFX_1_Distorsion_LowPass = 3154,
    TrackFX_1_Distorsion_HighPass = 3155,
    TrackFX_1_Distorsion_DryWetMix = 3156,

    TrackFX_1_EQ_Gain = 3161,

    TrackFX_2_Reverb_Volume = 3201,
    TrackFX_2_Reverb_Pan = 3202,
    TrackFX_2_Reverb_Time = 3202,
    TrackFX_2_Reverb_InitialDelay = 3203,
    TrackFX_2_Reverb_InitialDelayFeedback = 3204,
    TrackFX_2_Reverb_LowPass = 3205,
    TrackFX_2_Reverb_HighPass = 3206,
    TrackFX_2_Reverb_Damp = 3207,

    TrackFX_3_Reverb_Volume = 3301,
    TrackFX_3_Reverb_Pan = 3302,
    TrackFX_3_Reverb_Time = 3302,
    TrackFX_3_Reverb_InitialDelay = 3303,
    TrackFX_3_Reverb_InitialDelayFeedback = 3304,
    TrackFX_3_Reverb_LowPass = 3305,
    TrackFX_3_Reverb_HighPass = 3306,
    TrackFX_3_Reverb_Damp = 3307,
};

class EffectsEditor
{
    ApplicationSession *_session;
public:
    EffectsEditor();

    void SetUp(ApplicationSession *session);
    void Render2d();
};

#endif // EFFECTSEDITOR_H
