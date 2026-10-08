#pragma once

#include "isoundmanager.hpp"

#include "assetmanager.hpp"

typedef struct SoundVoice SoundVoice;

class CSoundManager : public ISoundManager
{
public:

    CSoundManager( void ) = default;
    ~CSoundManager( void );

    bool Init( const SDL_AudioSpec& spec ) override;
    void PlaySound( AssetHandle sound, float volume ) override {}
    void PlaySound3D( AssetHandle sound, float volume, vec3_t world_pos ) override {}
    void PlaySoundData(const uint8_t* data, uint32_t length, float volume );

    SDL_AudioSpec AudioSpec = {
    .format = SDL_AUDIO_F32,
    .channels = AUDIO_CHANNELS,
    .freq = AUDIO_SAMPLE_RATE
    };

    SDL_AudioStream* GetStream( void ) const { return m_stream; }

private:
    SDL_AudioStream* m_stream = nullptr;

  
};

extern CSoundManager* g_SoundManager;


struct SoundVoice
{
    SoundData* sound;

    vec3_t origin;

    float volume;
    float pitch;

    bool looping;
    bool active;
};