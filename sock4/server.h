//
// Created by thethongngu on 12/10/20.
//

#ifndef NP_PROJECT_04_SERVER_H
#define NP_PROJECT_04_SERVER_H

#include <boost/asio.hpp>
#include <wait.h>
#include "sock4_request.h"
#include "sock4_response.h"

class Server {
public:
    Server(boost::asio::io_service &io, uint32_t port);

    void wait_accept();

    void wait_signal();

    void process_client_request();

    void handle_connect(const Sock4Request &req);

    void handle_bind(const Sock4Request &req);

    static bool is_firewall_block(const Sock4Request &req, const std::string &user_ip);

    void output_console(Sock4Response::status_type status, const Sock4Request &req);

    void do_relay_data();

    void do_read_server();

    void do_read_client();

    void do_write_server(std::size_t len);

    void do_write_client(std::size_t len);


private:
    boost::asio::io_service &io;
    boost::asio::ip::tcp::resolver resolver;
    boost::asio::ip::tcp::acceptor acceptor, binder;
    boost::asio::signal_set signal;
    boost::asio::ip::tcp::socket client_socket, server_socket;

    unsigned char server_buffer[65536], client_buffer[65536];

};


#endif //NP_PROJECT_04_SERVER_H
