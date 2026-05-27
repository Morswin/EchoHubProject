#pragma once
#include <SDL3/SDL.h>
#include <opus.h>
#include <vector>
#include <iostream>
#include <cstdint>


class VoiceClient {
private:
    // --- STAŁE PARAMETRY AUDIO ---
    static constexpr int SAMPLE_RATE = 48000;      // Wymóg Opus dla optymalnego VoIP
    static constexpr int CHANNELS = 1;             // Mono
    static constexpr int FRAME_SIZE = 960;         // 20ms dźwięku przy 48kHz (48000 * 0.02)
    
    // Ilość bajtów na jedną ramkę surowego PCM (960 próbek * 4 bajty na float)
    static constexpr int BYTES_PER_FRAME = FRAME_SIZE * sizeof(float);

    // --- ZASOBY SDL3 (Sprzęt Audio) ---
    SDL_AudioStream* micStream = nullptr;
    SDL_AudioStream* speakerStream = nullptr;

    // --- ZASOBY OPUS (Kodeki) ---
    OpusEncoder* encoder = nullptr;
    OpusDecoder* decoder = nullptr;

    // --- BUFORY ROBOCZE ---
    // Bufor na surowe próbki z mikrofonu (Float32)
    std::vector<float> pcmBuffer;
    
public:
    VoiceClient() {
        // 1. Inicjalizacja podsystemu Audio w SDL3
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
            std::cerr << "Błąd inicjalizacji SDL_Audio: " << SDL_GetError() << std::endl;
            return; // W profesjonalnym kodzie warto rzucić wyjątkiem
        }

        // 2. Definicja formatu (48kHz, Mono, 32-bit Float) - taki sam dla obu urządzeń
        SDL_AudioSpec audioSpec;
        audioSpec.freq = SAMPLE_RATE;
        audioSpec.format = SDL_AUDIO_F32;
        audioSpec.channels = CHANNELS;

        // 3. Otwarcie strumieni audio (Mikrofon i Głośniki)
        micStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_RECORDING, &audioSpec, nullptr, nullptr);
        speakerStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audioSpec, nullptr, nullptr);

        if (!micStream || !speakerStream) {
            std::cerr << "Błąd otwarcia strumieni SDL: " << SDL_GetError() << std::endl;
        }

        // Uruchomienie sprzętu (rozpoczęcie nasłuchu i gotowość do odtwarzania)
        SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(micStream));
        SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(speakerStream));

        // 4. Inicjalizacja Kodeków Opus
        int error;
        // Koder: optymalizowany pod ludzki głos (VOIP)
        encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_VOIP, &error);
        if (error != OPUS_OK) {
            std::cerr << "Błąd kodera Opus: " << opus_strerror(error) << std::endl;
        }

        // Dekoder: musi znać tylko częstotliwość i ilość kanałów
        decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &error);
        if (error != OPUS_OK) {
            std::cerr << "Błąd dekodera Opus: " << opus_strerror(error) << std::endl;
        }

        // Przygotowanie bufora na wymaganą wielkość ramki
        pcmBuffer.resize(FRAME_SIZE);
    }

    ~VoiceClient() {
        // Sprzątanie pamięci przy niszczeniu obiektu, aby uniknąć wycieków
        if (micStream) SDL_DestroyAudioStream(micStream);
        if (speakerStream) SDL_DestroyAudioStream(speakerStream);
        
        if (encoder) opus_encoder_destroy(encoder);
        if (decoder) opus_decoder_destroy(decoder);
        
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }

    /**
     * Zbieranie danych z mikrofonu.
     * Wywołuj tę funkcję w pętli głównej programu (np. co klatkę/tik).
     * @param outEncodedPacket Wektor, do którego klasa wrzuci skompresowane dane sieciowe.
     * @return Zwraca true, jeśli zebrano pełną ramkę i ją skompresowano, false jeśli mikrofon wciąż zbiera dane.
     */
    bool RecordAndEncode(std::vector<uint8_t>& outEncodedPacket) {
        // Sprawdzamy, czy w buforze mikrofonu jest już wystarczająco dużo danych na pełną ramkę (20ms)
        if (SDL_GetAudioStreamAvailable(micStream) >= BYTES_PER_FRAME) {
            
            // Pobieramy surowe dane (PCM) do naszego wektora floatów
            SDL_GetAudioStreamData(micStream, pcmBuffer.data(), BYTES_PER_FRAME);
            
            // Przygotowujemy bufor na skompresowane dane. 
            // 1500 bajtów to bezpieczny zapas, pakiety głosowe ważą zazwyczaj < 100 bajtów.
            outEncodedPacket.resize(1500); 

            // Kompresja Opus
            int compressedBytes = opus_encode_float(
                encoder, 
                pcmBuffer.data(), 
                FRAME_SIZE, 
                outEncodedPacket.data(), 
                outEncodedPacket.size()
            );

            if (compressedBytes > 0) {
                // Skracamy wektor do rzeczywistego rozmiaru skompresowanych danych
                outEncodedPacket.resize(compressedBytes);
                return true; 
            }
        }
        return false;
    }

    /**
     * Odtwarzanie danych odebranych z sieci.
     * Wywołaj tę funkcję od razu, gdy dostaniesz przez UDP pakiet głosowy od kogoś z serwera.
     * @param inEncodedPacket Paczka bajtów prosto z sieci (UDP).
     */
    void DecodeAndPlay(const std::vector<uint8_t>& inEncodedPacket) {
        if (inEncodedPacket.empty()) return;

        // Lokaly bufor na zdekodowany sygnał (zawsze odzyskujemy pełną ramkę 960 próbek)
        std::vector<float> decodedPcm(FRAME_SIZE);

        // Dekompresja Opus
        int decodedSamples = opus_decode_float(
            decoder, 
            inEncodedPacket.data(), 
            inEncodedPacket.size(), 
            decodedPcm.data(), 
            FRAME_SIZE, 
            0 // 0 oznacza, że nie używamy mechanizmu FEC (odzyskiwania utraconych pakietów)
        );

        if (decodedSamples > 0) {
            // Wypychamy odzyskany surowy dźwięk prosto do głośników.
            // SDL3 automatycznie zadba o to, żeby sygnał został odtworzony płynnie.
            SDL_PutAudioStreamData(speakerStream, decodedPcm.data(), decodedSamples * sizeof(float));
        }
    }
};