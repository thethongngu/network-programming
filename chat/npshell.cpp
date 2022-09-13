#include <sys/types.h>
#include <algorithm>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/socket.h>

#include "npshell.h"
#include "string_utils.h"

#define debug(a) std::cerr << #a << " = " << a << std::endl

const int NPShell::MAX_PIPE;

NPShell::NPShell() : pipe_table(1005) {}

void NPShell::npshell_printenv(const std::string &input_str) {
    auto args = StringUtils::split_string(input_str, ' ');

    if (args.size() < 2) return;
    auto output = getenv(args[1].c_str());

    if (output != nullptr) {
        std::cout << output << std::endl;
    }
}

void NPShell::npshell_setenv(const std::string &input_str) {
    auto args = StringUtils::split_string(input_str, ' ');

    if (args.size() < 2) return;
    setenv(args[1].c_str(), args[2].c_str(), 1);  // assume number of arguments of setenv will be correct
}

std::string NPShell::parse_numbered_pipe(const std::string &input_str, int &numbered_pipe, bool &use_stderr) {
    auto token = StringUtils::split_string(input_str, ' ');
    std::string new_input_str = input_str;
    numbered_pipe = 0;

    if (token.back()[0] == '|' || token.back()[0] == '!') {
        new_input_str = input_str.substr(0, input_str.size() - token.back().size());
        numbered_pipe = std::stoi(token.back().substr(1));
        if (token.back()[0] == '!') use_stderr = true; else use_stderr = false;
    }

    return std::move(new_input_str);
}

void NPShell::open_pipe(int (&fd)[MAX_PIPE + 5][2], int num) {
    for (int i = 0; i < num; i++) {
        if (pipe(fd[i]) == -1) {
            std::cerr << "Error create pipe: " << errno << std::endl;
        }
    }
}

void NPShell::close_pipe(int (&fd)[MAX_PIPE + 5][2], int num) {
    for (int i = 0; i < num; i++) {
        close(fd[i][0]);
        close(fd[i][1]);
    }
}

void NPShell::redirect(int to, int from) {
    if (to != from) {
        if (dup2(to, from) == -1) {
            std::cerr << "Error duplicate file descriptor " << errno << std::endl;
        }
    }
}

void NPShell::update_numbered_pipe_direction(const int &numbered_pipe, const bool &use_stderr) {
    if (numbered_pipe == 0) return;

    if (pipe_table[numbered_pipe].read_from != STDIN_FILENO) {  // already piped by other numbered pipe before
        pipe_table[0].write_output_to = pipe_table[numbered_pipe].write_for_read;
        if (use_stderr) pipe_table[0].write_error_to = pipe_table[numbered_pipe].write_for_read;

    } else {  // have not piped yet
        int fd[2];
        pipe(fd);

        pipe_table[numbered_pipe].read_from = fd[0];
        pipe_table[numbered_pipe].write_for_read = fd[1];

        pipe_table[0].write_output_to = pipe_table[numbered_pipe].write_for_read;  // output of [0] is input of [numbered_pipe]
        if (use_stderr) pipe_table[0].write_error_to = pipe_table[numbered_pipe].write_for_read;  // error of [0] is input of [numbered_pipe]
    }
}

pid_t NPShell::exec_single_command(const std::string &command, int is_first, int read_from, int write_output_to,
                                   int write_error_to, int num_opened_pipe, int client_fd) {

    pid_t child_pid;
    while ((child_pid = fork()) < 0) usleep(1000);  // in case of number of process is off limit
    if (child_pid != 0) return child_pid;  // parent process return child pid here

    // only child process run from here
    redirect(read_from, STDIN_FILENO);
    redirect(write_output_to, STDOUT_FILENO);
    redirect(write_error_to, STDERR_FILENO);
    if (is_first && pipe_table[0].write_for_read != -1) close(pipe_table[0].write_for_read);

    close_pipe(process_fd, num_opened_pipe);   // we have duplicated pipe to standard, we can close these pipes now

    // handle redirect to file
    auto token = StringUtils::split_string(command, '>');
    if (token.size() > 1) {
        auto file = token[1];
        int write_to_file = open(file.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
        redirect(write_to_file, STDOUT_FILENO);
        close(write_to_file);
    }

    // parse input for execvp
    auto args = StringUtils::split_string(token[0], ' ');
    char *args_c[args.size() + 1];
    for (int i = 0; i < args.size(); i++) args_c[i] = const_cast<char *>(args[i].c_str());
    args_c[args.size()] = nullptr;

    // exec command
    execvp(args[0].c_str(), args_c);
    std::string unknown_error = "Unknown command: [" + std::string(args[0]) + "].\n";
    if (client_fd == -1) {
        std::cout << unknown_error;
        std::cout.flush();
    } else {  // dirty hack for send unknown command through network
        send(client_fd, unknown_error.c_str(), unknown_error.size(), 0);
    }
    exit(EXIT_FAILURE);
}

void NPShell::exec_numbered_pipe_command(std::string &input_str) {

    int numbered_pipe = 0;
    bool use_stderr = false;

    input_str = parse_numbered_pipe(input_str, numbered_pipe, use_stderr);
    update_numbered_pipe_direction(numbered_pipe, use_stderr);
    exec_long_command(input_str, STDOUT_FILENO, STDERR_FILENO, false);

}

pid_t NPShell::exec_long_command(const std::string &input_str, int wait_fd, int middle_stderr, bool is_remote_call) {
    if (is_remote_call) pipe_table.emplace_back();
    auto commands = StringUtils::split_string(input_str, '|');

    pid_t wait_id = -1;
    int bridge_pipe[2];
    for (int i = 0; i < commands.size(); i += MAX_PIPE) {

        int start = i;
        int end = i + std::min(MAX_PIPE, (int) commands.size() - i);

        open_pipe(process_fd, end - start);
        bridge_pipe[1] = process_fd[MAX_PIPE - 1][1];

        int first_input = (start == 0) ? pipe_table[0].read_from : bridge_pipe[0];
        int last_output = (end == commands.size()) ? pipe_table[0].write_output_to : bridge_pipe[1];
        int last_error = (end == commands.size()) ? pipe_table[0].write_error_to : middle_stderr;

        for (int pos = start; pos < end; pos++) {
            auto comm = commands[pos];

            int inp = (pos == start) ? first_input : process_fd[pos - start - 1][0];
            int out = (pos == end - 1) ? last_output : process_fd[pos - start][1];
            int err = (pos == end - 1) ? last_error : middle_stderr;
            bool is_first = (start == 0);

            pid_t child_pid;
            if (is_remote_call) {  // send through network
                child_pid = exec_single_command(comm, is_first, inp, out, err, end - start, wait_fd);
            } else {
                child_pid = exec_single_command(comm, is_first, inp, out, err, end - start, -1);
            }

            process_list.push_back(child_pid);
        }

        if (start == 0) close(pipe_table[0].write_for_read);
        if (end != commands.size()) bridge_pipe[0] = dup(process_fd[MAX_PIPE - 1][0]);
        close_pipe(process_fd, end - start);  // close duplicated pipe in parent, children will close on its own

        int id = -1;
        for (auto process_id: process_list) {
            id++;
            if (pipe_table[0].write_output_to == wait_fd) {
                waitpid(process_id, nullptr, 0);
            }
        }
        if (pipe_table[0].write_output_to != wait_fd) wait_id = process_list.front();
        process_list.clear();
    }

    if (is_remote_call) pipe_table.pop_front();
    return wait_id;
}

void NPShell::run_interactive_shell() {

    setenv("PATH", "bin:.", 1);

    while (true) {
        std::cout << "% ";
        std::string input_str;
        std::getline(std::cin, input_str);
        if (std::cin.eof()) return;
        input_str = StringUtils::trim(input_str);

        pipe_table.emplace_back();
        if (input_str == "exit") {
            return;
        } else if (input_str.substr(0, 8) == "printenv") {
            npshell_printenv(input_str);
        } else if (input_str.substr(0, 6) == "setenv") {
            npshell_setenv(input_str);
        } else if (input_str.empty()) {
            continue;
        } else {
            exec_numbered_pipe_command(input_str);
        }
        pipe_table.pop_front();
    }
}

NPShell::NPShell(const NPShell &s) {
    for(auto const& c: s.pipe_table) this->pipe_table.push_back(c);
    for(int i = 0; i < NPShell::MAX_PIPE + 5; i++) {
        this->process_fd[i][0] = s.process_fd[i][0];
        this->process_fd[i][1] = s.process_fd[i][1];
    }
    for(int i : s.process_list) this->process_list.push_back(i);
}
