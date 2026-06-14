#define SDL_MAIN_USE_CALLBACKS 1;
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <opus.h>

#include <string>
#include <vector>
// #include <chrono>
#include <format>
#include <iostream>
// #include <cstdint>
#include <thread>

#include "app_state.hpp"
#include "message.hpp"
// #include "ThreadSafeQueue.hpp"
#include "ui/views.hpp"
#include "view_states.hpp"


// APP STATE
AppState g_AppState;
// SDL_Window *window = nullptr;
// SDL_Renderer *renderer = nullptr;
//
// // --- 3. Stan aplikacji ---
// EViewState currentView = EViewState::AUTH_VIEW; // Startujemy od AuthView
//
// // Stan dla każdego widoku
// std::string email, password;
// std::string serverAddress, nickname;
// std::string serverName, region;
// std::string chatInput;
// std::string username = "User_C++";
// std::string selectedMic = "Default Microphone Input";
// std::string verificationLevel = "Brak (Dowolny użytkownik może wejść)";
// std::string confirmPassword;
//
// // Dane testowe (później zastąpione realnymi danymi)
// std::vector<Molecules::Friend> friends = {
//     {"JanKowalski", true},
//     {"Programista99", false}
// };
// std::vector<Views::Channel> channels = {
//     {"ogólny", "#", true},
//     {"pomoc-kod", "#", true},
//     {"Poczekalnia", "🔊", false},
//     {"Pokój gier", "🔊", false}
// };
// std::vector<Molecules::Message> messages = {
//     {"Admin", "Cześć! Witamy na klonie Discorda napisanym w C++.", "Dzisiaj o 12:00"},
//     {"User_C++", "Wygląda super, czat tekstowy i enumy działają stabilnie!", "Dzisiaj o 12:05"}
// };
//
// Theme theme = Theme::createGoldGalaxyTheme();

// static char message_buffer[1024] = "";
// APP STATE END

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    // Initializing window and renderer with SDL3
    if (!SDL_Init(SDL_INIT_VIDEO)) return SDL_APP_FAILURE;
    if (!SDL_CreateWindowAndRenderer("EchoHub", 800, 600, SDL_WINDOW_RESIZABLE, &g_AppState.window, &g_AppState.renderer)) {
        return SDL_APP_FAILURE;
    }
    // Initiating ImGui
    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForSDLRenderer(g_AppState.getWindow(), g_AppState.getRenderer());
    ImGui_ImplSDLRenderer3_Init(g_AppState.getRenderer());

    // 1. Załaduj fonty (MUSI BYĆ PRZED PIERWSZYM ImGui::NewFrame!)
    ImGuiIO& io = ImGui::GetIO();

    // Roboto (tekst)
    g_AppState.getTheme().fontRegular = io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Black.ttf", 16.0f);
    g_AppState.getTheme().fontBold = io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Bold.ttf", 16.0f);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    // Start rendering imgui frame
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // --- 5. System przełączania widoków ---
        switch (g_AppState.getView()) {
            case EViewState::AUTH_VIEW:
                Views::AuthView(g_AppState.getUsername(), g_AppState.getTheme(),
                    [&]() { g_AppState.setView(EViewState::LANDING_VIEW); },  // Zaloguj
                    [&]() { g_AppState.setView(EViewState::REGISTER_VIEW); }  // Zarejestruj
                );
                break;

            case EViewState::LANDING_VIEW:
                Views::LandingView(g_AppState.getTheme(),
                    [&]() { g_AppState.setView(EViewState::FRIENDS_LIST_VIEW); },
                    [&]() { g_AppState.setView(EViewState::CONNECT_TO_NEW_SERVER_VIEW); },
                    [&]() { g_AppState.setView(EViewState::CREATE_NEW_SERVER_VIEW); }
                );
                break;

            case EViewState::FRIENDS_LIST_VIEW:
                Views::FriendsListView(g_AppState.getFriends(), g_AppState.getTheme(),
                    [&]() { g_AppState.setView(EViewState::CREATE_NEW_SERVER_VIEW); },
                    [&](const std::string& serverId) {
                        g_AppState.setView(EViewState::SERVER_VIEW);
                    },
                    [&]() { g_AppState.setView(EViewState::FRIENDS_LIST_VIEW); }
                );
                break;

            case EViewState::CONNECT_TO_NEW_SERVER_VIEW:
                Views::ConnectToNewServerView(g_AppState.getServerAddress(), g_AppState.getNickname(), g_AppState.getTheme(),
                    [&]() { g_AppState.setView(EViewState::CONNECTING_LOADING_VIEW); },
                    [&]() { g_AppState.setView(EViewState::LANDING_VIEW); }
                );
                break;

            case EViewState::CREATE_NEW_SERVER_VIEW:
                Views::CreateNewServerView(g_AppState.getServerName(), g_AppState.getRegion(), g_AppState.getTheme(),
                    [&]() { g_AppState.setView(EViewState::CONNECTING_LOADING_VIEW); },
                    [&]() { g_AppState.setView(EViewState::LANDING_VIEW); }
                );
                break;

            case EViewState::SERVER_VIEW:
                Views::ServerView("Mój Serwer C++", g_AppState.getChannels(), g_AppState.getMessages(), g_AppState.getChatInput(), g_AppState.getTheme(),
                    [&](const std::string& msg) {
                        g_AppState.getMessages().push_back({g_AppState.getUsername(), msg, "Teraz"});
                        g_AppState.getChatInput().clear();
                    },
                    [&]() { g_AppState.setView(EViewState::CREATE_NEW_SERVER_VIEW); },
                    [&](const std::string& serverId) { g_AppState.setView(EViewState::SERVER_VIEW); },
                    [&]() { g_AppState.setView(EViewState::FRIENDS_LIST_VIEW); }
                );
                break;

            case EViewState::USER_SETTINGS_VIEW:
                Views::UserSettingsView(g_AppState.getUsername(), g_AppState.getSelectedMic(), g_AppState.getTheme(),
                    [&]() { g_AppState.setView(EViewState::LANDING_VIEW); }
                );
                break;

            case EViewState::SERVER_SETTINGS_VIEW:
                Views::ServerSettingsView(g_AppState.getServerName(), g_AppState.getVerificationLevel(), g_AppState.getTheme(),
                    [&]() { g_AppState.setView(EViewState::SERVER_VIEW); }
                );
                break;

            case EViewState::CONNECTING_LOADING_VIEW:
                Views::ConnectingLoadingView(g_AppState.getTheme());
                // Symuluj połączenie (później zastąp realną logiką)
                static int counter = 0;
                if (counter++ > 60) { // Po 60 klatkach (ok. 1s)
                    g_AppState.setView(EViewState::SERVER_VIEW);
                    counter = 0;
                }
                break;

            case EViewState::ERROR_DISCONNECTED_VIEW:
                Views::ErrorDisconnectedView(g_AppState.getTheme(),
                    [&]() { g_AppState.setView(EViewState::CONNECTING_LOADING_VIEW); },
                    [&]() { g_AppState.setView(EViewState::LANDING_VIEW); }
                );
                break;

            case EViewState::REGISTER_VIEW:
                Views::RegisterView(g_AppState.getUsername(), g_AppState.getTheme(),
                    [&]() { g_AppState.setView(EViewState::LANDING_VIEW); },  // Powrót po rejestracji
                    [&]() { g_AppState.setView(EViewState::AUTH_VIEW); }       // Anuluj
                );
                break;
        }

    // Rendering background
    SDL_SetRenderDrawColor(g_AppState.getRenderer(), 33, 33, 33, 255);
    SDL_RenderClear(g_AppState.getRenderer());

    // Render the ImGui
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), g_AppState.getRenderer());

    // Display the frame
    SDL_RenderPresent(g_AppState.getRenderer());

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    // Passing events to ImGui so the UI can be interacted with
    ImGui_ImplSDL3_ProcessEvent(event);

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  // Exits the app
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(g_AppState.renderer);
    SDL_DestroyWindow(g_AppState.window);
    SDL_Quit();
}

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



// 