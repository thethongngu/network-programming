//
// Created by thethongngu on 12/12/20.
//

#ifndef NP_PROJECT_04_SOCK4_RESPONSE_H
#define NP_PROJECT_04_SOCK4_RESPONSE_H


#include "sock4_request.h"

class Sock4Response {
public:

    enum status_type {
        ACCEPT = 1, REJECT = 0
    };

    explicit Sock4Response(status_type status);
    Sock4Response(status_type status, unsigned char high_port, unsigned char low_port, boost::asio::ip::address_v4::bytes_type ip);

    std::string get_ip_address() const;
    std::string get_port() const;

    static std::string get_status_str(status_type status);

    std::array<boost::asio::mutable_buffer, 5> get_res_buffer();

    unsigned char version, code, high_byte_port, low_byte_port;
    boost::asio::ip::address_v4::bytes_type ip;
};


#endif //NP_PROJECT_04_SOCK4_RESPONSE_H
