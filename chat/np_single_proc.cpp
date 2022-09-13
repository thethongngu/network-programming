//
// Created by thethongngu on 11/5/20.
//

#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <string>
#include <arpa/inet.h>
#include <vector>
#include <map>
#include <csignal>
#include <wait.h>

#include "user.h"
#include "string_utils.h"

#define debug(a) std::cerr << #a << " = " << a << std::endl

fd_set active_fd;
int max_fd;
//int dev_null;
std::map<int, User> users;
std::map<int, int> socket_to_id;

// ----------------------- GENERATE MESSAGE ------------------------

std::string get_welcome_message() {
    return "****************************************\n"
           "** Welcome to the information server. **\n"
           "****************************************\n";
}

std::string get_login_message(int user_id) {
    return "*** User '" + users[user_id].name + "' entered from " + users[user_id].ip + ":" + users[user_id].port +
           ". ***\n";
}

std::string get_logout_message(int user_id) {
    return "*** User '" + users[user_id].name + "' left. ***\n";
}

std::string get_send_pipe_message(int from_id, int to_id, const std::string &command) {
    auto pipe_mess = "*** " + users[from_id].name + " (#" + std::to_string(users[from_id].id) + ") just piped '";
    pipe_mess.append(command).append("' to " + users[to_id].name + " (#" + std::to_string(users[to_id].id) + ") ***\n");
    return pipe_mess;
}

std::string get_receive_pipe_message(int from_id, int to_id, const std::string &command) {
    auto pipe_mess = "*** " + users[to_id].name + " (#" + std::to_string(users[to_id].id) + ") just received from ";
    pipe_mess.append(users[from_id].name + " (#" + std::to_string(users[from_id].id) + ") by '" + command + "' ***\n");
    return pipe_mess;
}

std::string get_pipe_not_exist_message(int from_id, int to_id) {
    return "*** Error: the pipe #" + std::to_string(users[from_id].id) + "->#" + std::to_string(users[to_id].id) +
           " does not exist yet. ***\n";
}

std::string get_pipe_already_exist_message(int from_id, int to_id) {
    return "*** Error: the pipe #" + std::to_string(users[from_id].id) + "->#" + std::to_string(users[to_id].id) +
           " already exists. ***\n";
}

std::string get_user_not_exist_message(int user_id) {
    return "*** Error: user #" + std::to_string(user_id) + " does not exist yet. ***\n";
}

// -------------------------- MANAGE USER ----------------------------

int get_smallest_available_id() {
    int res = 1;
    while (users.find(res) != users.end()) res++;
    return res;
}

int get_user_id(int socket_fd) {
    return socket_to_id[socket_fd];
}

int add_user(int client_fd, const std::string &user_name, const std::string &user_ip, const std::string &user_port) {
    int user_id = get_smallest_available_id();
    User new_user(user_id, client_fd, user_name, user_ip, user_port);
    users[user_id] = new_user;
    socket_to_id[client_fd] = user_id;

    return user_id;
}

void remove_user(int user_id) {
    FD_CLR(users[user_id].socket_fd, &active_fd);
    close(users[user_id].socket_fd);
    socket_to_id.erase(users[user_id].socket_fd);
    users.erase(user_id);
}

bool is_user_exist(int user_id) {
    return users.find(user_id) != users.end();
}

bool is_pipe_exist(int from_id, int to_id) {
    return users[to_id].is_written[from_id];
}

int read_user(int user_id, char *buf, size_t len) {
    int bytes = 0;
    if ((bytes = recv(users[user_id].socket_fd, buf, len, 0)) <= 0) {
        perror("Cannot read from user ");
    }
    return bytes;
}

void write_user(int user_id, const std::string &content) {
    if (send(users[user_id].socket_fd, content.c_str(), content.size(), 0) < 0) {
        perror("Cannot send message to socket_fd\n");
    }
}

void broadcast(const std::string &content) {
    for (auto const &u: users) {
        write_user(u.first, content);
    }
}

// -------------------------- 4 NEW SHELL COMMANDS ----------------------------

void who(int user_id) {
    std::string res = "<ID>\t<nickname>\t<IP:port>\t<indicate me>\n";
    for (auto const &u: users) {
        res.append(std::to_string(u.first) + "\t");
        res.append(u.second.name + "\t");
        res.append(u.second.ip + ":" + u.second.port);
        if (user_id == u.second.id) res.append("\t<-me");
        res.append("\n");
    }

    write_user(user_id, res);
}

void tell(int user_id, int recv_id, const std::string &content) {
    if (is_user_exist(recv_id)) {
        write_user(recv_id, "*** " + users[user_id].name + " told you ***: " + content + "\n");
    } else {
        write_user(user_id, "*** Error: user #" + std::to_string(recv_id) + " does not exist yet. ***\n");
    }
}

void yell(int user_id, const std::string &content) {
    broadcast("*** " + users[user_id].name + " yelled ***: " + content + "\n");
}

void name(int user_id, const std::string &new_name) {
    for (auto const &u: users) {
        if (u.second.name == new_name) {
            write_user(user_id, "*** User '" + new_name + "' already exists. ***\n");
            return;
        }
    }

    users[user_id].name = new_name;
    auto message = "*** User from " + users[user_id].ip + ":" + users[user_id].port +
                   " is named '" + users[user_id].name + "'. ***\n";
    broadcast(message);
}

// -------------------------- OLD SHELL COMMANDS + USER PIPE ----------------------------

std::string parse_numbered_pipe(const std::string &input_str, int &numbered_pipe, bool &use_stderr) {
    auto token = StringUtils::split_string(input_str, ' ');
    std::string new_input_str = input_str;
    numbered_pipe = 0;
    use_stderr = false;

    if (token.back()[0] == '|' || token.back()[0] == '!') {
        new_input_str = input_str.substr(0, input_str.size() - token.back().size());
        numbered_pipe = std::stoi(token.back().substr(1));
        if (token.back()[0] == '!') use_stderr = true; else use_stderr = false;
    }

    return std::move(new_input_str);
}

std::string parse_user_pipe(const std::string &input_str, int &user_in, int &user_out) {
    auto token = StringUtils::split_string(input_str, ' ');
    std::string new_input_str;

    user_in = -1;
    user_out = -1;
    for (auto str: token) {
        str = StringUtils::trim(str);
        if (str.size() < 2 || (str[0] != '>' && str[0] != '<') ||
            !StringUtils::is_positive_number(str.substr(1, str.size() - 1))) {
            new_input_str.append(str + " ");
        } else {
            if (str[0] == '>') user_out = std::stoi(str.substr(1, str.size() - 1));
            else user_in = std::stoi(str.substr(1, str.size() - 1));
        }
    }

    if (!new_input_str.empty()) new_input_str.pop_back();
    return std::move(new_input_str);
}

// -------------------------- MAIN CONTROLS ----------------------------------

int listen_on_port(const std::string &port) {
    struct addrinfo hints{};
    struct addrinfo *server;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status;
    if ((status = getaddrinfo(nullptr, port.c_str(), &hints, &server)) != 0) {
        fprintf(stderr, "Cannot getaddrinfo: %s\n", gai_strerror(status));
    }

    int server_fd;
    if ((server_fd = socket(server->ai_family, server->ai_socktype, server->ai_protocol)) < 0) {
        perror("Cannot create socket\n");
    }
    max_fd = server_fd;

    int yes = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) < 0) {
        perror("Error setsockopt");
    }

    if (bind(server_fd, server->ai_addr, server->ai_addrlen) < 0) {
        perror("Cannot bind socket\n");
    }
    freeaddrinfo(server);

    if (listen(server_fd, 35) < 0) {
        perror("Cannot listen on this socket\n");
    }
    return server_fd;
}

void reset_env(int user_id) {
    clearenv();
    for (auto const &s: users[user_id].env) {
        setenv(s.first.c_str(), s.second.c_str(), 1);
    }
}

void handle_login(int server_fd) {
    struct sockaddr_storage client{};
    socklen_t client_size = sizeof client;
    int client_fd = accept(server_fd, (struct sockaddr *) &client, &client_size);
    if (client_fd < 0) {
        perror("Cannot accept new connection");
        exit(EXIT_FAILURE);
    }
    FD_SET(client_fd, &active_fd);
    max_fd = std::max(max_fd, client_fd);

    char user_ip[INET6_ADDRSTRLEN];
    char user_port[INET6_ADDRSTRLEN];
    if (getnameinfo(
            (struct sockaddr *) &client,
            client_size, user_ip, INET6_ADDRSTRLEN, user_port,
            INET6_ADDRSTRLEN, NI_NUMERICHOST | NI_NUMERICSERV) < 0) {
        perror("Cannot get new_user ip\n");
    }

    int user_id = add_user(client_fd, "(no name)", user_ip, user_port);
    write_user(user_id, get_welcome_message());
    broadcast(get_login_message(user_id));
    write_user(user_id, "% ");
}

void handle_logout(int user_id) {
    broadcast(get_logout_message(user_id));
    remove_user(user_id);
}

bool handle_command(int user_id, const std::string &input_str) {
    if (input_str == "exit") {
        handle_logout(user_id);
        int status;
        int wpid;
        while ((wpid = wait(&status)) > 0);
        return true;
    }

    if (input_str.substr(0, 8) == "printenv") {
        auto args = StringUtils::split_string(input_str, ' ');
        if (args.size() < 2) return true;
        if (users[user_id].env.find(args[1]) != users[user_id].env.end()) {
            write_user(user_id, users[user_id].env[args[1]] + "\n");
        }
        return true;
    }

    if (input_str.substr(0, 6) == "setenv") {
        auto args = StringUtils::split_string(input_str, ' ');
        if (args.size() < 2) return true;
        setenv(args[1].c_str(), args[2].c_str(), 1);
        users[user_id].env[args[1]] = args[2];
        return true;
    }

    if (input_str == "who") {
        who(user_id);
        return true;
    }

    if (input_str.substr(0, 4) == "tell") {
        auto args = StringUtils::split_string(input_str, ' ');
        std::string mess;
        for (int i = 2; i < args.size(); i++) mess.append(args[i]).append(" ");
        mess.pop_back();
        tell(user_id, stoi(args[1]), mess);
        return true;
    }

    if (input_str.substr(0, 4) == "yell") {
        std::string message = "*** " + users[user_id].name + " yelled ***: ";
        yell(user_id, input_str.substr(4, input_str.size() - 4));
        return true;
    }

    if (input_str.substr(0, 4) == "name") {
        auto args = StringUtils::split_string(input_str, ' ');
        name(user_id, args[1]);
        return true;
    }

    if (input_str.empty()) {
        return true;
    }

    return false;
}

void handle_shell(int user_id, const std::string &input_str) {
    int user_in, user_out, numbered_pipe;
    bool use_stderr;
    std::string command_str;
    command_str = parse_numbered_pipe(input_str, numbered_pipe, use_stderr);
    command_str = parse_user_pipe(command_str, user_in, user_out);

    int err_null = -1, out_null = -1;
    if (user_in != -1) {
        if (!is_user_exist(user_in)) {
            write_user(user_id, get_user_not_exist_message(user_in));
            users[user_id].redirect_input_from_null();
        } else {
            if (!is_pipe_exist(user_in, user_id)) {
                write_user(user_id, get_pipe_not_exist_message(user_in, user_id));
                users[user_id].redirect_input_from_null();
            } else {
                users[user_id].redirect_input_from_user(user_in);
                broadcast(get_receive_pipe_message(user_in, user_id, input_str));
            }
        }
    }

    if (user_out != -1) {
        if (!is_user_exist(user_out)) {
            write_user(user_id, get_user_not_exist_message(user_out));
            out_null = users[user_id].redirect_output_to_null();
            err_null = users[user_id].redirect_error_to_null();
        } else {
            if (is_pipe_exist(user_id, user_out)) {
                write_user(user_id, get_pipe_already_exist_message(user_id, user_out));
                users[user_id].redirect_output_to_null();
                err_null = users[user_id].redirect_error_to_null();
            } else {
                users[user_id].redirect_output_to_user(users[user_out]);
                broadcast(get_send_pipe_message(user_id, user_out, input_str));
            }
        }
    }

    // change pipe direction for numbered pipe
    if (numbered_pipe != 0) users[user_id].redirect_output_to_command(numbered_pipe, use_stderr);

    // if not pipe to other user (note: user_out == -1 different with !is_user_exist(user_out))
    if (user_out == -1) {
        if (!use_stderr) users[user_id].redirect_stderr_to_me();
        if (numbered_pipe == 0) users[user_id].redirect_stdout_to_me();
    }

    // execvp
    reset_env(user_id);
    users[user_id].shell.exec_long_command(
            command_str, users[user_id].socket_fd, users[user_id].socket_fd, true
    );

    if (err_null != -1) {
        close(err_null);
        close(out_null);
    }
}

int main(int argc, char *argv[]) {

    signal(SIGCHLD, SIG_IGN);

    int server_fd = listen_on_port(argv[1]);

    fd_set read_fd;
    FD_ZERO(&active_fd);
    FD_ZERO(&read_fd);
    FD_SET(server_fd, &active_fd);

    while (true) {
        read_fd = active_fd;
        if (select(max_fd + 1, &read_fd, nullptr, nullptr, nullptr) < 0) {
            perror("Cannot select socket descriptor to read");
            exit(EXIT_FAILURE);
        }

        for (int socket_fd = 0; socket_fd < max_fd + 1; socket_fd++) {
            if (FD_ISSET(socket_fd, &read_fd) == 0) continue;

            if (socket_fd == server_fd) {  // new client
                handle_login(server_fd);

            } else {  // read from current user
                char buf[15000];
                memset(buf, 0, sizeof buf);
                int user_id = get_user_id(socket_fd);
                int bytes = read_user(user_id, buf, sizeof buf);
                if (bytes == 0) {
                    handle_logout(user_id);
                    continue;
                }

                std::string input_str(buf);
                input_str = StringUtils::trim(input_str);
                bool is_handle = handle_command(user_id, input_str);
                if (!is_handle) handle_shell(user_id, input_str);
                if (is_user_exist(user_id)) write_user(user_id, "% ");
            }
        }
    }
}