//
// Created by thethongngu on 11/5/20.
//

#ifndef NP_PROJECT_02_USER_H
#define NP_PROJECT_02_USER_H


#include <string>
#include <iostream>
#include <map>
#include <fcntl.h>

#include "npshell.h"

class User {

public:
    int id;
    int socket_fd;
    std::string name;
    std::string ip;
    std::string port;
    std::map<std::string, std::string> env;
    NPShell shell;

    const static int MAX_USER = 35;

    // user_pipe for reading message from other users
    // other users write to: this_user.user_pipe[their_id][1]
    // this user read from: user_pipe[their_id][0]
    int user_pipe[MAX_USER][2]{};
    bool is_written[MAX_USER]{};


    User();

    User(const User &u);

    User &operator=(const User &u);

    User(int user_id, int socket_fd, const std::string &name, const std::string &ip, const std::string &port);

    void redirect_input_from_user(int user_id);

    void redirect_output_to_command(int numbered_pipe, bool use_stderr);

    void redirect_output_to_user(User &u);

    void redirect_output_to_fd(int fd);

    void redirect_input_from_fd(int fd);

    void redirect_error_to_fd(int fd);

    void redirect_input_from_null();

    int redirect_output_to_null();

    int redirect_error_to_null();

    void redirect_stdout_to_me();

    void redirect_stderr_to_me();

    ~User();
};


#endif //NP_PROJECT_02_USER_H
