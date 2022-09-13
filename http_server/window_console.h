//
// Created by thethongngu on 11/18/20.
//

#ifndef NP_PROJECT_03_CONSOLE_H
#define NP_PROJECT_03_CONSOLE_H

#include <string>
#include <map>
#include "window_session.h"

class Console {
public:
    boost::asio::io_service &io;
    std::vector<Session> sessions;

    Console(boost::asio::io_context &io_context);

    void create_sessions(const std::map<std::string, std::string> &query_dict, boost::asio::ip::tcp::socket &client_socket);

    Session parse_session(const std::map<std::string, std::string>& query_dict, int sess_id, boost::asio::ip::tcp::socket &client_socket);

    std::string response_initial_html();
};


#endif //NP_PROJECT_03_CONSOLE_H
