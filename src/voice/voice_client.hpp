#ifndef ECHOHUB_VOICE_CLIENT_HPP
#define ECHOHUB_VOICE_CLIENT_HPP

#include <SDL3/SDL.h>
#include <opus.h>
#include <vector>
#include <string>
#include <cstdint>

/**
 * @brief Client for voice capture and playback using Opus codec and SDL3 audio.
 * 
 * Handles microphone input, Opus encoding, and speaker output for VoIP functionality.
 * Designed to work in a separate thread for non-blocking audio processing.
 */
class VoiceClient {
public:
    // --- Constants for audio configuration ---
    static constexpr int SAMPLE_RATE = 48000;      // Opus requirement for optimal VoIP
    static constexpr int CHANNELS = 1;             // Mono
    static constexpr int FRAME_SIZE = 960;         // 20ms of audio at 48kHz (48000 * 0.02)
    static constexpr int BYTES_PER_FRAME = FRAME_SIZE * sizeof(float);

    VoiceClient();
    ~VoiceClient();

    /**
     * @brief Initialize audio devices and Opus codecs.
     * @return true if initialization succeeded, false otherwise.
     */
    bool initialize();

    /**
     * @brief Shutdown audio devices and Opus codecs.
     */
    void shutdown();

    /**
     * @brief Record audio from microphone and encode it using Opus.
     * @param outEncodedPacket Vector to store the encoded audio packet.
     * @return true if a full frame was recorded and encoded, false otherwise.
     */
    bool recordAndEncode(std::vector<uint8_t>& outEncodedPacket);

    /**
     * @brief Decode an Opus packet and play it through the speaker.
     * @param inEncodedPacket The encoded audio packet to decode and play.
     */
    void decodeAndPlay(const std::vector<uint8_t>& inEncodedPacket);

    /**
     * @brief Check if the voice client is initialized and ready.
     */
    bool isInitialized() const { return micStream != nullptr && speakerStream != nullptr && encoder != nullptr && decoder != nullptr; }

private:
    // --- SDL3 Audio Resources ---
    SDL_AudioStream* micStream = nullptr;
    SDL_AudioStream* speakerStream = nullptr;

    // --- Opus Codec Resources ---
    OpusEncoder* encoder = nullptr;
    OpusDecoder* decoder = nullptr;

    // --- Working Buffers ---
    std::vector<float> pcmBuffer;
};

#endif // ECHOHUB_VOICE_CLIENT_HPP
