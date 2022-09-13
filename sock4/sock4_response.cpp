//
// Created by thethongngu on 12/12/20.
//

#include "sock4_response.h"

Sock4Response::Sock4Response(Sock4Response::status_type status) : ip() {
    version = 0;
    code = (status == status_type::ACCEPT) ? 90 : 91;
    high_byte_port = 0;
    low_byte_port = 0;
}

Sock4Response::Sock4Response(Sock4Response::status_type status, unsigned char high_port, unsigned char low_port,
                             boost::asio::ip::address_v4::bytes_type ip) {
    version = 0;
    code = (status == status_type::ACCEPT) ? 90 : 91;
    high_byte_port = high_port;
    low_byte_port = low_port;
    this->ip = ip;
}

std::array<boost::asio::mutable_buffer, 5> Sock4Response::get_res_buffer() {
    return {{
                    boost::asio::buffer(&version, 1),
                    boost::asio::buffer(&code, 1),
                    boost::asio::buffer(&high_byte_port, 1),
                    boost::asio::buffer(&low_byte_port, 1),
                    boost::asio::buffer(ip)
            }};
}

std::string Sock4Response::get_ip_address() const {
    return boost::asio::ip::address_v4(ip).to_string();
}

std::string Sock4Response::get_port() const {
    uint32_t port = ((high_byte_port << 8) & 0xff00) | low_byte_port;
    return std::to_string(port);
}

std::string Sock4Response::get_status_str(status_type status) {
    return (status == Sock4Response::ACCEPT) ? "ACCEPT" : "REJECT";
}


