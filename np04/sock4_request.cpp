//
// Created by thethongngu on 12/12/20.
//

#include "sock4_request.h"

std::array<boost::asio::mutable_buffer, 7> Sock4Request::get_req_buffer() {
    return {{
                    boost::asio::buffer(&version, 1),
                    boost::asio::buffer(&cmd_id, 1),
                    boost::asio::buffer(&high_byte_port, 1),
                    boost::asio::buffer(&low_byte_port, 1),
                    boost::asio::buffer(ip),
                    boost::asio::buffer(user_id),
                    boost::asio::buffer(&null_byte, 1)
            }};
}

std::string Sock4Request::get_ip_address() const {
    return boost::asio::ip::address_v4(ip).to_string();
}

std::string Sock4Request::get_port() const {
    uint32_t port = ((high_byte_port << 8) & 0xff00) | low_byte_port;
    return std::to_string(port);
}

bool Sock4Request::is_connect_op() const {
    return (cmd_id == 1);
}

std::string Sock4Request::get_cmd_name() const {
    return (cmd_id == 1) ? "CONNECT" : "BIND";
}

Sock4Request::Sock4Request(unsigned char cmd_id, unsigned char high_byte_port, unsigned char low_byte_port,
                           boost::asio::ip::address_v4::bytes_type addr, std::string user_id) :
        cmd_id(cmd_id), high_byte_port(high_byte_port), low_byte_port(low_byte_port), ip(addr) {
    version = 4;
    null_byte = 0;
    this->user_id = user_id;
}

Sock4Request::Sock4Request() {
    version = 4;
    null_byte = 0;
    user_id = "";
}
