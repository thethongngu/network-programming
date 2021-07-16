//
// Created by thethongngu on 11/18/20.
//

#include <wait.h>
#include "http_server.h"
#include "string_utils.h"

#define debug(a) std::cerr << #a << " = " << a << std::endl


HTTPServer::HTTPServer(boost::asio::io_context &io_context, unsigned short port) :
        io(io_context), signal(io), acceptor(io, {boost::asio::ip::tcp::v4(), port}), socket(io) {

//    signal.add(SIGINT);
//    signal.add(SIGTERM);

    acceptor.listen();
    do_wait_child();
    do_accept();
}

void HTTPServer::do_wait_child() {
    signal.async_wait([this](boost::system::error_code ec, int sig) {
        if (acceptor.is_open()) {  // parent process
            while (waitpid(-1, nullptr, WNOHANG) > 0);
            do_wait_child();
        }
    });
}

void HTTPServer::do_accept() {
    acceptor.async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket s) {
        if (!ec) {
            socket = std::move(s);
            io.notify_fork(boost::asio::io_service::fork_prepare);
            if (!fork()) {  // child
                io.notify_fork(boost::asio::io_service::fork_child);
                acceptor.close();
                do_read();
            } else {
                io.notify_fork(boost::asio::io_service::fork_parent);
                socket.close();
                do_accept();
            }
        }
    });
}

void HTTPServer::do_read() {

    socket.async_read_some(
            boost::asio::buffer(data, DATA_LENGTH),
            [this](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    auto header = StringUtils::parse_http_req(std::string(data, data + length));
                    for (auto const &e: header) {
                        setenv(e.first.c_str(), e.second.c_str(), 1);
                    }
                    setenv("SERVER_ADDR", socket.local_endpoint().address().to_string().c_str(), 1);
                    setenv("SERVER_PORT", std::to_string(socket.local_endpoint().port()).c_str(), 1);
                    setenv("REMOTE_ADDR", socket.remote_endpoint().address().to_string().c_str(), 1);
                    setenv("REMOTE_PORT", std::to_string(socket.remote_endpoint().port()).c_str(), 1);

                    dup2(socket.native_handle(), 0);
                    dup2(socket.native_handle(), 1);
//                    dup2(socket.native_handle(), 2);

                    auto exec_cgi = header["CGI_EXEC"];
                    char *args[] = {nullptr};

                    std::cout << "HTTP/1.1" << " 200 OK\r\n";
                    if (execv(exec_cgi.c_str(), args) == -1) {
                        auto error = "Cannot run execv(" + exec_cgi + ")";
                        perror(error.c_str());
                    }
                }
            }
    );
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);

    std::cout << "Listening..." << std::endl;

    boost::asio::io_context io;
    HTTPServer server(io, atoi(argv[1]));
    io.run();

    return 0;
}

