//
// Created by thethongngu on 11/18/20.
//

#ifndef NP_PROJECT_03_HTTP_SERVER_H
#define NP_PROJECT_03_HTTP_SERVER_H

#include <boost/asio.hpp>


class HTTPServer {

public:
    static const int DATA_LENGTH = 2048;

private:
    boost::asio::io_service &io;
    boost::asio::signal_set signal;
    boost::asio::ip::tcp::acceptor acceptor;
    boost::asio::ip::tcp::socket socket;
    char data[DATA_LENGTH];

public:
    HTTPServer(boost::asio::io_context &io_context, unsigned short port);

    void do_wait_child();

    void do_accept();

    void do_read();
};


#endif //NP_PROJECT_03_HTTP_SERVER_H
