//
// Created by thethongngu on 11/21/20.
//

#ifndef NP_PROJECT_03_SESSION_H
#define NP_PROJECT_03_SESSION_H


#include <string>
#include <fstream>
#include <boost/asio.hpp>
#include <memory>
#include <iostream>

#include "string_utils.h"

class Session {
public:
    static const int DATA_LENGTH = 2048;
    int sess_id;
    std::string host;
    std::string port;
    std::string file;
    std::fstream fs;
    char data[DATA_LENGTH];

    boost::asio::ip::tcp::resolver resolver;
    boost::asio::ip::tcp::socket socket;
    boost::asio::ip::tcp::socket& client_socket;

    Session() = delete;

    Session(int sess_id, const std::string &host, const std::string &port, const std::string &file,
            boost::asio::io_context &io_service, boost::asio::ip::tcp::socket &client_socket);

    void execute();

    void do_read();

    void do_write(std::string content);
};


#endif //NP_PROJECT_03_SESSION_H
