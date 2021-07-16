//
// Created by thethongngu on 11/21/20.
//

#include "window_session.h"

Session::Session(int sess_id, const std::string &host, const std::string &port, const std::string &file,
                 boost::asio::io_context &io_service, boost::asio::ip::tcp::socket &client_socket) : resolver(io_service), socket(io_service), 
                 fs("test_case/" + file, std::ios::in), client_socket(client_socket) {
    this->sess_id = sess_id;
    this->host = host;
    this->port = port;
    this->file = file;
}

void Session::execute() {
    // std::cerr << "start execute..." << std::endl;
    resolver.async_resolve(
            {host, port},
            [this](boost::system::error_code ec, const boost::asio::ip::tcp::resolver::iterator &it) {
                if (!ec) {
                    socket.async_connect(*it, [this](boost::system::error_code ec) {
                        if (!ec) {
                            // std::cerr << "start reading..." << std::endl;
                            do_read();
                        }
                    });
                } else {
                    debug(ec);
                }
            }
    );
}

void Session::do_read() {
    socket.async_receive(
            boost::asio::buffer(data, DATA_LENGTH),
            [this](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    auto res = std::string(data, data + length);
                    auto html_res = StringUtils::format_html(res, sess_id, 0);
                    do_write(html_res);

                    if (res.find('%') != std::string::npos) {
                        std::string new_comm;
                        std::getline(fs, new_comm);
                        auto html_comm = StringUtils::format_html(new_comm + '\n', sess_id, 1);
                        
                        
                        // int a;
                        // std::cin >> a;
                        socket.write_some(boost::asio::buffer(new_comm + '\n'));
                        do_write(html_comm);
                    }
                    do_read();
                } else {
                    perror("Error read");
                }
            }
    );
}

void Session::do_write(std::string content) {
    boost::asio::write(client_socket, boost::asio::buffer(content.c_str(), content.size()));
}
