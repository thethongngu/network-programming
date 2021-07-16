//
// Created by thethongngu on 11/5/20.
//

#include "user.h"


User::User(int user_id, int socket_fd, const std::string &name, const std::string &ip, const std::string &port) {
    this->id = user_id;
    this->socket_fd = socket_fd;
    this->name = name;
    this->ip = ip;
    this->port = port;

    this->shell = NPShell();
    this->shell.npshell_setenv("setenv PATH bin:.");

    this->env.clear();
    this->env["PATH"] = "bin:.";

    for (int i = 0; i < MAX_USER; i++) {
        this->user_pipe[i][0] = this->user_pipe[i][1] = 0;
        this->is_written[i] = false;
    }
}

User &User::operator=(const User &u) {
    this->id = u.id;
    this->socket_fd = u.socket_fd;
    this->name = u.name;
    this->ip = u.ip;
    this->port = u.port;
    this->env = u.env;
    this->shell = u.shell;

    for (int i = 0; i < MAX_USER; i++) {
        this->user_pipe[i][0] = u.user_pipe[i][0];
        this->user_pipe[i][1] = u.user_pipe[i][1];
        this->is_written[i] = u.is_written[i];
    }

    return *this;
}

User::User(const User &u) {
    this->id = u.id;
    this->socket_fd = u.socket_fd;
    this->name = u.name;
    this->ip = u.ip;
    this->port = u.port;
    this->env = u.env;
    this->shell = u.shell;

    for (int i = 0; i < MAX_USER; i++) {
        this->user_pipe[i][0] = u.user_pipe[i][0];
        this->user_pipe[i][1] = u.user_pipe[i][1];
        this->is_written[i] = u.is_written[i];
    }
}

User::~User() = default;

User::User() {
    this->id = -1;
    this->socket_fd = -1;
    this->name = "";
    this->ip = "";
    this->port = "";
    this->env.clear();

    this->shell = NPShell();
    this->shell.npshell_setenv("setenv PATH bin:.");

    for (int i = 0; i < MAX_USER; i++) {
        this->user_pipe[i][0] = this->user_pipe[i][1] = 0;
        this->is_written[i] = false;
    }
}

void User::redirect_input_from_user(int user_id) {
    shell.pipe_table[0].read_from = user_pipe[user_id][0];
    shell.pipe_table[0].write_for_read = user_pipe[user_id][1];
    is_written[user_id] = false;
}

void User::redirect_output_to_user(User &u) {
    if (pipe(u.user_pipe[this->id]) == -1) {
        perror("Error cannot open pipe");
    }
    shell.pipe_table[0].write_output_to = u.user_pipe[this->id][1];
    u.is_written[this->id] = true;
}

void User::redirect_output_to_fd(int fd) {
    shell.pipe_table[0].write_output_to = fd;
}

void User::redirect_error_to_fd(int fd) {
    shell.pipe_table[0].write_error_to = fd;
}

void User::redirect_input_from_fd(int fd) {
    shell.pipe_table[0].read_from = fd;
}

void User::redirect_input_from_null() {
    int dev_null = open("/dev/null", O_RDWR);
    redirect_input_from_fd(dev_null);
}

int User::redirect_output_to_null() {
    int dev_null = open("/dev/null", O_RDWR);
    redirect_output_to_fd(dev_null);
    return dev_null;
}

int User::redirect_error_to_null() {
    int dev_null = open("/dev/null", O_RDWR);
    redirect_error_to_fd(dev_null);
    return dev_null;
}

void User::redirect_output_to_command(int numbered_pipe, bool use_stderr) {
    shell.update_numbered_pipe_direction(numbered_pipe, use_stderr);
}

void User::redirect_stdout_to_me() {
    shell.pipe_table[0].write_output_to = socket_fd;
}

void User::redirect_stderr_to_me() {
    shell.pipe_table[0].write_error_to = socket_fd;
}
