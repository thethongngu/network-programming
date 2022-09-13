//
// Created by thethongngu on 12/14/20.
//

#ifndef NP_PROJECT_04_SOCK4_SESSION_H
#define NP_PROJECT_04_SOCK4_SESSION_H

#include <boost/asio.hpp>
#include <string>
#include <fstream>

#include "string_utils.h"
#include "sock4_response.h"

class Sock4Session {
public:

    static const int DATA_LENGTH = 2048;
    int sess_id;
    std::string host;
    std::string port;
    std::string file;
    std::string sock_host;
    std::string sock_port;
    std::fstream fs;
    char data[DATA_LENGTH];

    boost::asio::ip::tcp::resolver resolver;
    boost::asio::ip::tcp::socket socket;

    Sock4Session() = delete;

    Sock4Session(int sess_id, const std::string &host, const std::string &port, const std::string &file,
                 const std::string &sock_host, const std::string &sock_port, boost::asio::io_context &io_service);

    void execute();

    void do_read();
};


#endif //NP_PROJECT_04_SOCK4_SESSION_H
