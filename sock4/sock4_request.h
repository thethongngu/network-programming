//
// Created by thethongngu on 12/12/20.
//

#ifndef NP_PROJECT_04_SOCK4_REQUEST_H
#define NP_PROJECT_04_SOCK4_REQUEST_H

#include <boost/asio.hpp>
#include <array>


class Sock4Request {
public:
    Sock4Request();
    Sock4Request(unsigned char cmd_id, unsigned char high_byte_port, unsigned char low_byte_port,
                 boost::asio::ip::address_v4::bytes_type addr, std::string user_id);

    std::array<boost::asio::mutable_buffer, 7> get_req_buffer();

    std::string get_ip_address() const;

    std::string get_port() const;

    std::string get_cmd_name() const;

    bool is_connect_op() const;

    unsigned char version, cmd_id, high_byte_port, low_byte_port, null_byte;
    boost::asio::ip::address_v4::bytes_type ip;
    std::string user_id;
};


#endif //NP_PROJECT_04_SOCK4_REQUEST_H
