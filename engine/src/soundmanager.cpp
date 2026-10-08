#include "soundmanager.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <stdio.h>


CSoundManager* g_SoundManager = new CSoundManager();

CSoundManager::~CSoundManager( void )
{
    if (m_stream) SDL_DestroyAudioStream(m_stream);
}

bool CSoundManager::Init(const SDL_AudioSpec& spec)
{
    AudioSpec = spec;

    m_stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
        &AudioSpec,
        nullptr,
        nullptr
    );

    if (!m_stream) {
        SDL_Log("Failed to open audio stream: %s", SDL_GetError());
        return false;
    }

    if (!SDL_ResumeAudioStreamDevice(m_stream)) {
        SDL_Log("Failed to resume audio stream: %s", SDL_GetError());
        return false;
    }

    return true;
}


void CSoundManager::PlaySoundData(
    const uint8_t* data,
    uint32_t length,
    float volume)
{
    if (!m_stream || !data || length == 0)
        return;

    volume = SDL_clamp(volume, 0.0f, 1.0f);

    const auto* samples = reinterpret_cast<const int16_t*>(data);
    const size_t count = length / sizeof(int16_t);

    std::vector<int16_t> scaled(count);

    for (size_t i = 0; i < count; ++i)
        scaled[i] = static_cast<int16_t>(samples[i] * volume);

    if (!SDL_PutAudioStreamData(
            m_stream,
            scaled.data(),
            static_cast<int>(count * sizeof(int16_t)))) {
        SDL_Log("Failed to submit audio: %s", SDL_GetError());
    }
}