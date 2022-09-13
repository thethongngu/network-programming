//
// Created by thethongngu on 11/18/20.
//

#ifndef NP_PROJECT_03_CONSOLE_H
#define NP_PROJECT_03_CONSOLE_H

#include <boost/asio.hpp>
#include <string>
#include <map>
#include "session.h"

class Console {
public:
    boost::asio::io_service &io;
    std::vector<Session> sessions;

    Console(boost::asio::io_context &io_context, const std::map<std::string, std::string>& query_dict);

    Session parse_session(const std::map<std::string, std::string>& query_dict, int sess_id);

    void response_initial_html();
};


#endif //NP_PROJECT_03_CONSOLE_H
