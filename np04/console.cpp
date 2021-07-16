//
// Created by thethongngu on 12/13/20.
//

#include "console.h"

Console::Console(boost::asio::io_context &io_context, const std::map<std::string, std::string> &query_dict)
        : io(io_context) {

    std::cout << "Content-type: text/html\r\n\r\n";
    std::cout.flush();
    for (int session_id = 0; session_id < 5; session_id++) {
        parse_session(query_dict, session_id);
    }
    response_initial_html();
}

void Console::parse_session(const std::map<std::string, std::string> &query_dict, int sess_id) {
    std::string host, port, file;

    auto socks_ip = query_dict.find("sh")->second;
    auto socks_port = query_dict.find("sp")->second;

    for (auto const &q: query_dict) {
        auto id = q.first[1] - '0';
        if (sess_id == id) {
            if (q.first[0] == 'h') host = q.second;
            if (q.first[0] == 'p') port = q.second;
            if (q.first[0] == 'f') file = q.second;
        }
    }

    if (!host.empty() && !port.empty()) {
        sock4_session.emplace_back(sess_id, host, port, file, socks_ip, socks_port, io);
    }
}

void Console::response_initial_html() {
    std::string col_name, col_value;
    for (auto const &s: sock4_session) {
        col_name += StringUtils::get_column_name_html(s.host, s.port);
        col_value += StringUtils::get_column_value_html(s.sess_id);
    }
    std::cout << StringUtils::get_html_page(col_name, col_value);
    std::cout.flush();
}

int main(int argc, char *argv[]) {
    auto query = getenv("QUERY_STRING");
    auto query_dict = StringUtils::parse_http_query(query);

    boost::asio::io_context io;
    Console console(io, query_dict);
    for (auto &s: console.sock4_session) {
        s.execute();
    }
    io.run();
}