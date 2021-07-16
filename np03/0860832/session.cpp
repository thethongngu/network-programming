//
// Created by thethongngu on 11/21/20.
//

#include "session.h"

Session::Session(int sess_id, const std::string &host, const std::string &port, const std::string &file,
                 boost::asio::io_context &io_service) : resolver(io_service), socket(io_service) {
    this->sess_id = sess_id;
    this->host = host;
    this->port = port;
    this->file = file;

    fs.open("test_case/" + this->file, std::fstream::in);
}

void Session::execute() {
    resolver.async_resolve(
            {host, port},
            [this](boost::system::error_code ec, const boost::asio::ip::tcp::resolver::iterator &it) {
                if (!ec) {
                    socket.async_connect(*it, [this](boost::system::error_code ec) {
                        if (!ec) {
                            do_read();
                        }
                    });
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

                    std::cout << html_res;
                    std::cout.flush();

                    if (res.find('%') != std::string::npos) {
                        std::string new_comm;
                        std::getline(fs, new_comm);
                        auto html_comm = StringUtils::format_html(new_comm + '\n', sess_id, 1);
                        socket.write_some(boost::asio::buffer(new_comm + '\n'));
                        std::cout << html_comm;
                        std::cout.flush();
                    }
                    do_read();
                }
            }
    );
}
