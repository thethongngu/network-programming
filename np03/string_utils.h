//
// Created by thethongngu on 11/19/20.
//

#ifndef NP_PROJECT_03_STRING_UTILS_H
#define NP_PROJECT_03_STRING_UTILS_H

#endif //NP_PROJECT_03_STRING_UTILS_H

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <map>
#include <string>
#include <iostream>
#include <vector>

#define debug(a) std::cerr << #a << " = " << a << std::endl

class StringUtils {
public:
    static void remove_char(std::string &str, const char &chr);

    static std::map<std::string, std::string> parse_http_req(std::string req);

    static std::map<std::string, std::string> parse_http_query(std::string query);

    static std::string get_column_name_html(const std::string &host, const std::string &port);

    static std::string get_column_value_html(int sess_id);

    static std::string get_html_page(std::string col_name, std::string col_value);

    static std::string format_html(std::string res, int id, int color);

    static std::string get_panel_html();
};