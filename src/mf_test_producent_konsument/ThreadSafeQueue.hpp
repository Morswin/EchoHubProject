#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <utility> // dla std::move

template <typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_var_;

public:
    // Domyślny konstruktor
    ThreadSafeQueue() = default;

    // Zabraniany kopiowania tej kolejki (kopiowanie muteksów jest nielegalne)
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    /**
     * Wrzuca element do kolejki (używane przez Producenta / Wątek A)
     */
    void push(T item) {
        // 1. Zamykamy dostęp do kolejki na kłódkę
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 2. Dodajemy element (używamy std::move dla optymalizacji wydajności)
        queue_.push(std::move(item));
        
        // 3. Budzimy jeden wątek, który czeka na dane (jeśli jakiś czeka)
        cond_var_.notify_one();
        
        // Kłódka zdejmuje się sama na końcu funkcji (dzięki std::lock_guard)
    }

    /**
     * Wyciąga element z kolejki (używane przez Konsumenta / Wątek B).
     * Jeśli kolejka jest pusta, wątek zasypia (nie zużywa CPU) i czeka na dane.
     */
    T pop() {
        // 1. Zakładamy specjalną kłódkę, która umie się "odpinać" na czas snu
        std::unique_lock<std::mutex> lock(mutex_);
        
        // 2. Jeśli kolejka jest pusta, oddajemy kłódkę i IDZIEMY SPAĆ.
        // Budzimy się dopiero, gdy ktoś wywoła notify_one() w funkcji push()
        cond_var_.wait(lock, [this]() { return !queue_.empty(); });
        
        // 3. Po przebudzeniu (i ponownym zablokowaniu kłódki), wyciągamy dane
        T item = std::move(queue_.front());
        queue_.pop();
        
        return item;
    }

    /**
     * Sprawdza, czy kolejka jest pusta.
     */
    bool empty() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
};