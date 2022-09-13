//
// Created by thethongngu on 11/18/20.
//

#include <csignal>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <boost/asio.hpp>
#include "window_console.h"

class HTTPServer {
public:
    boost::asio::io_service &io;
    boost::asio::ip::tcp::acceptor acceptor;
    boost::asio::ip::tcp::socket socket;
    static const int DATA_LENGTH = 2048;
    char data[DATA_LENGTH];
    Console &console;

    HTTPServer(boost::asio::io_context &io_context, Console& console, unsigned short port) :
            io(io_context), acceptor(io, {boost::asio::ip::tcp::v4(), port}), socket(io), console(console) {

        acceptor.listen();
        do_accept();
    }

    void do_accept() {
        acceptor.async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket s) {
            if (!ec) {
                this->socket = std::move(s);
                do_read();
            }
            do_accept();
        });
    }

    void do_read() {
        socket.async_read_some(
                boost::asio::buffer(data, DATA_LENGTH),
                [this](boost::system::error_code ec, std::size_t length) {
                    if (!ec) {
                        auto header = StringUtils::parse_http_req(std::string(data, data + length));

                        auto exec_cgi = header["CGI_EXEC"];
                        auto res_header = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
                        do_write(res_header);

                        if (exec_cgi == "panel.cgi") {
                            auto panel_html = StringUtils::get_panel_html();
                            do_write(panel_html);

                        } else if (exec_cgi == "console.cgi") {
                            auto query = header["QUERY_STRING"];
                            auto query_dict = StringUtils::parse_http_query(query);
                            console.create_sessions(query_dict, socket);
                            do_write(console.response_initial_html());
                            for (auto &s: console.sessions) {
                                s.execute();
                            }
                        }
                    }
                }
        );
    }

    void do_write(std::string content) {
        boost::asio::write(socket, boost::asio::buffer(content, content.size()));
    }
};

int main(int argc, char *argv[]) {
    std::cerr << "Listening..." << std::endl;

    boost::asio::io_context io;
    Console console(io);
    HTTPServer server(io, console, atoi(argv[1]));
    io.run();

    return 0;
}