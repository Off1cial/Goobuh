#pragma once

#include "math/vector.h"
#include <SDL3/SDL_audio.h>

#include "public/engine/assethandle.hpp"
#define AUDIO_SAMPLE_RATE 48000
#define AUDIO_CHANNELS    2
#define AUDIO_FORMAT_STR "Signed 16-bit PCM"

class ISoundManager
{
public:
    virtual bool Init( const SDL_AudioSpec& spec ) = 0;

    virtual void PlaySound( AssetHandle sound, float volume ) = 0;
    virtual void PlaySound3D( AssetHandle sound, float volume, vec3_t world_pos ) = 0;
};