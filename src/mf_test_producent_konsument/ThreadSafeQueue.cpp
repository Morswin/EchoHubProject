#include <iostream>
#include <thread>
#include <chrono>
#include "ThreadSafeQueue.hpp"
#include <asio.hpp>

// Funkcja Wątku A (Producent)
void producerThread(ThreadSafeQueue<int>& q) {
    for (int i = 1; i <= 10; ++i) {
        std::cout << "[PRODUCENT] Wygenerowano paczke nr: " << i << std::endl;
        q.push(i); // Wrzucamy dane bezpiecznie do kolejki
        
        // Symulacja czasu potrzebnego na zebranie danych z mikrofonu (100ms)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Funkcja Wątku B (Konsument)
void consumerThread(ThreadSafeQueue<int>& q) {
    for (int i = 1; i <= 10; ++i) {
        // pop() zablokuje ten wątek (uśpi go), jeśli nie ma jeszcze danych
        int data = q.pop(); 
        
        std::cout << "[KONSUMENT] Odebrano i wyslano paczke nr: " << data << "\n-------------------" << std::endl;
        
        // Zauważ, że konsument nie ma żadnego sleep_for! 
        // Czeka tylko dzięki cond_var_.wait() wewnątrz funkcji pop().
    }
}

int main() {
    std::cout << "Start testu kolejki wielowatkowej..." << std::endl;

    // Tworzymy naszą bezpieczną kolejkę na liczby całkowite
    ThreadSafeQueue<int> myQueue;

    // Uruchamiamy dwa niezależne wątki, przekazując im referencję do naszej kolejki
    // Używamy std::ref, aby oba wątki widziały dokładnie tę samą kolejkę w pamięci
    std::thread t1(producerThread, std::ref(myQueue));
    std::thread t2(consumerThread, std::ref(myQueue));

    // Wątek główny (main) czeka, aż t1 i t2 zakończą swoją pracę
    t1.join();
    t2.join();

    std::cout << "Test zakonczony sukcesem. Wątki polaczone." << std::endl;
    return 0;
}