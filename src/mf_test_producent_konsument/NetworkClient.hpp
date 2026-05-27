#pragma once

#include <asio.hpp>
#include <iostream>
#include <string>
#include <vector>

class NetworkClient {
private:
    asio::io_context& io_context_;
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint server_endpoint_;
    
    // Bufor na dane przychodzące (1024 bajty spokojnie wystarczą na pakiety głosowe)
    std::vector<char> recv_buffer_;
    
    // Zmienna, do której Asio wpisze, od kogo właśnie odebraliśmy pakiet
    asio::ip::udp::endpoint sender_endpoint_;

public:
    // Konstruktor: przyjmuje referencję do io_context, adres IP serwera i port
    NetworkClient(asio::io_context& io_context, const std::string& host, const std::string& port)
        : io_context_(io_context), 
          socket_(io_context, asio::ip::udp::v4()), // Tworzymy gniazdo IPv4 UDP
          recv_buffer_(1024) 
    {
        // Resolver tłumaczy adres (np. "127.0.0.1" lub "localhost") na format zrozumiały dla Asio
        asio::ip::udp::resolver resolver(io_context_);
        server_endpoint_ = *resolver.resolve(asio::ip::udp::v4(), host, port).begin();

        // Od razu po utworzeniu klienta, każemy mu nasłuchiwać na odpowiedzi z serwera
        startReceive();
    }

    // Funkcja do wysyłania tekstowego "Pinga" (później zmienimy to na wysyłanie bajtów Opusa)
    void send(const std::string& message) {
        // async_send_to wysyła pakiet w tle. 
        socket_.async_send_to(
            asio::buffer(message), server_endpoint_,
            [](std::error_code ec, std::size_t /*bytes_sent*/) {
                if (ec) {
                    std::cerr << "[KLIENT] Błąd wysyłania: " << ec.message() << "\n";
                }
            });
    }

private:
    // Pętla nasłuchująca (działa asynchronicznie)
    void startReceive() {
        socket_.async_receive_from(
            asio::buffer(recv_buffer_), sender_endpoint_,
            [this](std::error_code ec, std::size_t bytes_recvd) {
                if (!ec && bytes_recvd > 0) {
                    // Zamieniamy odebrane bajty na stringa i wypisujemy
                    std::string received_msg(recv_buffer_.data(), bytes_recvd);
                    std::cout << "[KLIENT OTRZYMAŁ]: " << received_msg << "\n";
                } else if (ec) {
                    std::cerr << "[KLIENT] Błąd odbierania: " << ec.message() << "\n";
                }

                // BARDZO WAŻNE: Kiedy skończymy przetwarzać ten pakiet, 
                // musimy ponownie wywołać nasłuchiwanie na kolejny pakiet!
                startReceive();
            });
    }
};