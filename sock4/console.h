//
// Created by thethongngu on 12/13/20.
//

#ifndef NP_PROJECT_04_CONSOLE_H
#define NP_PROJECT_04_CONSOLE_H

#include <boost/asio.hpp>
#include <string>
#include <map>
#include "sock4_session.h"

class Console {
public:
    boost::asio::io_service &io;
    std::vector<Sock4Session> sock4_session;

    Console(boost::asio::io_context &io_context, const std::map<std::string, std::string> &query_dict);

    void parse_session(const std::map<std::string, std::string> &query_dict, int sess_id);

    void response_initial_html();
};


#endif //NP_PROJECT_04_CONSOLE_H
