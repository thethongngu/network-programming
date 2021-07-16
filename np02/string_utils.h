//
// Created by thethongngu on 11/3/20.
//

#ifndef NP_PROJECT_02_STRING_UTILS_H
#define NP_PROJECT_02_STRING_UTILS_H


#include <string>
#include <vector>
#include <sstream>

class StringUtils {
public:
    inline static std::string trim(const std::string &s) {
        int left;
        for (left = 0; left < s.length(); left++) {
            if (s[left] != ' ' && s[left] != '\n' && s[left] != '\r') break;
        }

        int right;
        for (right = s.length() - 1; right >= 0; right--) {
            if (s[right] != ' ' && s[right] != '\n' && s[right] != '\r') break;
        }

        return s.substr(left, right + 1 - left);
    }


    inline static std::vector<std::string> split_string(const std::string &input_str, const char &de) {
        std::istringstream string_stream(input_str);
        std::string s;
        std::vector<std::string> res;
        while (std::getline(string_stream, s, de)) {
            if (!s.empty()) res.push_back(trim(s));
        }

        return std::move(res);
    }

    inline static bool is_positive_number(const std::string& str) {
        for(auto c: str) {
            if (!isdigit(c)) return false;
        }
        return true;
    }
};


#endif //NP_PROJECT_02_STRING_UTILS_H
