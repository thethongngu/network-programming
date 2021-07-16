//
// Created by thethongngu on 12/14/20.
//

#include "sock4_session.h"

Sock4Session::Sock4Session(int sess_id, const std::string &host, const std::string &port, const std::string &file,
                           const std::string &sock_host, const std::string &sock_port,
                           boost::asio::io_context &io_service) : resolver(io_service), socket(io_service) {
    this->sess_id = sess_id;
    this->host = host;
    this->port = port;
    this->file = file;
    this->sock_host = sock_host;
    this->sock_port = sock_port;

    fs.open("test_case/" + this->file, std::fstream::in);
}

void Sock4Session::execute() {

    resolver.async_resolve(
            {sock_host, sock_port},
            [this](boost::system::error_code ec, const boost::asio::ip::tcp::resolver::iterator &it) {
                if (!ec) {
                    socket.async_connect(*it, [this](boost::system::error_code ec) {
                        if (!ec) {  // connected to proxy
                            auto server_endpoint = *resolver.resolve({host, port});
                            Sock4Request req(
                                    1,
                                    (server_endpoint.endpoint().port() >> 8) & 0xff,
                                    (server_endpoint.endpoint().port() & 0xff),
                                    server_endpoint.endpoint().address().to_v4().to_bytes(),
                                    ""
                            );
                            boost::asio::write(socket, req.get_req_buffer());

                            Sock4Response reply(Sock4Response::status_type::ACCEPT);
                            boost::asio::read(socket, reply.get_res_buffer());

                            if (reply.code != 90) return;
                            do_read();
                        }
                    });
                }
            }
    );
}

void Sock4Session::do_read() {
    socket.async_receive(
            boost::asio::buffer(data),
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
                        std::cout << html_comm;
                        std::cout.flush();

                        socket.write_some(boost::asio::buffer(new_comm + '\n'));
                    }
                    do_read();
                }
            }
    );
}
