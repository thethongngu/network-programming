//
// Created by thethongngu on 12/10/20.
//

#include <cstdlib>
#include <iostream>
#include <fstream>
#include "server.h"

#define debug(a) std::cerr << #a << " = " << a << std::endl

boost::asio::io_service io_service;

Server::Server(boost::asio::io_service &io, uint32_t port) :
        acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
        signal(io), resolver(io), client_socket(io), server_socket(io), binder(io), io(io) {

    wait_signal();
    wait_accept();
}

void Server::wait_signal() {
    signal.async_wait([this](boost::system::error_code ec, int sig) {
        if (acceptor.is_open()) {
            while (waitpid(-1, nullptr, WNOHANG) > 0);
            wait_signal();
        }
    });
}

void Server::wait_accept() {
    acceptor.async_accept(client_socket, [this](boost::system::error_code ec) {
        if (!ec) {
            io.notify_fork(boost::asio::io_context::fork_prepare);
            if (!fork()) {  // child
                io.notify_fork(boost::asio::io_service::fork_child);
                acceptor.close();
                signal.cancel();
                try {
                    process_client_request();
                } catch (boost::wrapexcept<boost::system::system_error> &e) {
                    client_socket.close();
                    server_socket.close();
                    exit(0);
                }

            } else {
                io_service.notify_fork(boost::asio::io_context::fork_parent);
                client_socket.close();
                server_socket.close();
                wait_accept();
            }
        } else {
            std::cerr << "error code: " << ec;
            std::cerr.flush();
        }
    });
}

void Server::process_client_request() {

    Sock4Request req;
    boost::asio::read(client_socket, req.get_req_buffer());

//    debug(int(req.version));
//    debug(int(req.cmd_id));
//    debug(int(req.high_byte_port));
//    debug(int(req.low_byte_port));
//    debug(req.get_ip_address());
//    debug(int(req.null_byte));
//    debug(req.user_id);

    auto user_ip = client_socket.remote_endpoint().address().to_string();
    if (is_firewall_block(req, user_ip)) {
        Sock4Response res(Sock4Response::status_type::REJECT);
        boost::asio::write(client_socket, res.get_res_buffer());
        output_console(Sock4Response::status_type::REJECT, req);
        return;
    }

    output_console(Sock4Response::status_type::ACCEPT, req);
    if (req.is_connect_op()) handle_connect(req);
    else handle_bind(req);
}

void Server::handle_connect(const Sock4Request &req) {
    resolver.async_resolve(
            {req.get_ip_address(), req.get_port()},
            [this, req](const boost::system::error_code ec, const boost::asio::ip::tcp::resolver::iterator &it) {
                if (!ec) {
                    server_socket.async_connect(*it, [this, req](boost::system::error_code ec1) {
                        if (!ec1) {
                            Sock4Response res(Sock4Response::status_type::ACCEPT);
                            boost::asio::write(client_socket, res.get_res_buffer());
                            do_relay_data();
                        }
                    });
                }
            }
    );
}

void Server::handle_bind(const Sock4Request &req) {
    boost::asio::ip::tcp::endpoint this_host(boost::asio::ip::tcp::v4(), 0);
    binder.open(this_host.protocol());
    binder.bind(this_host);
    binder.listen();

    unsigned char high_port = binder.local_endpoint().port() >> 8;
    unsigned char low_port = binder.local_endpoint().port();

    Sock4Response res(Sock4Response::status_type::ACCEPT, high_port, low_port,
                      boost::asio::ip::make_address_v4("0.0.0.0").to_bytes());
    boost::asio::write(client_socket, res.get_res_buffer());
    binder.accept(server_socket);
    boost::asio::write(client_socket, res.get_res_buffer());

    do_relay_data();
}

void Server::output_console(Sock4Response::status_type status, const Sock4Request &req) {
    std::cout << "<S_IP>: " + client_socket.remote_endpoint().address().to_string() + "\n" +
                 "<S_PORT>: " + std::to_string(client_socket.remote_endpoint().port()) + "\n" +
                 "<D_IP>: " + req.get_ip_address() + "\n" +
                 "<D_PORT>: " + req.get_port() + "\n" +
                 "<Command>: " + req.get_cmd_name() + "\n" +
                 "<Reply>: " + Sock4Response::get_status_str(status) + "\n" << std::endl;
    std::cout.flush();
}


void Server::do_relay_data() {
    do_read_server();
    do_read_client();
}

void Server::do_read_server() {
    server_socket.async_receive(
            boost::asio::buffer(server_buffer),
            [this](boost::system::error_code ec, std::size_t server_len) {
                if (!ec) {
//                    std::cout << "\n\n Read server:" << std::endl;
//                    for (int i = 0; i < server_len; i++) std::cout << server_buffer[i];
//                    std::cout.flush();
                    do_write_client(server_len);
                } else {
                    server_socket.close();
//                    std::cerr << "read server\n";
//                    std::cerr << ec.message();
//                    std::cerr << "end\n";
//                    std::cerr.flush();
//                    throw std::system_error{ec};
                }
            }
    );
}

void Server::do_write_client(std::size_t len) {
    boost::asio::async_write(
            client_socket, boost::asio::buffer(server_buffer, len),
            [this](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
//                    std::cout << "\n\n Write client:" << std::endl;
//                    for (int i = 0; i < length; i++) std::cout << server_buffer[i];
//                    std::cout.flush();
                    do_read_server();
                } else {
                    client_socket.close();
                }
            }
    );
}

void Server::do_read_client() {
    client_socket.async_receive(
            boost::asio::buffer(client_buffer),
            [this](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
//                    std::cout << "\n\n Read client:" << std::endl;
//                    for (int i = 0; i < length; i++) std::cout << client_buffer[i];
//                    std::cout.flush();
                    do_write_server(length);
                } else {
                    client_socket.close();
                }
            }
    );
}

void Server::do_write_server(std::size_t len) {
    boost::asio::async_write(
            server_socket, boost::asio::buffer(client_buffer, len),
            [this](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
//                    std::cout << "\n\n Write server:" << std::endl;
//                    for (int i = 0; i < length; i++) std::cout << client_buffer[i];
//                    std::cout.flush();
                    do_read_client();
                } else {
                    server_socket.close();
                }
            }
    );
}

bool Server::is_firewall_block(const Sock4Request &req, const std::string &user_ip) {
    std::ifstream f("socks.conf");
    std::string line;
    std::vector<std::string> whitelist;

    while (getline(f, line)) {
        auto type = line[7];
        auto ip = line.substr(9);
        if ((req.cmd_id == 1 && type == 'c') || (req.cmd_id == 2 && type == 'b')) {
            whitelist.push_back(ip);
//            debug(ip);
        }
    }

    for (const auto &white_ip: whitelist) {
        auto prefix = white_ip.substr(0, white_ip.find('*'));
//        debug(prefix);
//        debug(user_ip.substr(0, prefix.size()));
        if (user_ip.substr(0, prefix.size()) == prefix) return false;
    }

    return true;
}


int main(int argc, char *argv[]) {
    Server server(io_service, atoi(argv[1]));
    io_service.run();
}

