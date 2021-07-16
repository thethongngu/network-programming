#include <sys/types.h>
#include <algorithm>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sstream>
#include <deque>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "npshell.hpp"

#define debug(a) std::cerr << #a << " = " << a << std::endl

struct CommandStream {
    int read_from = STDIN_FILENO;
    int write_for_read = -1;
    int write_output_to = STDOUT_FILENO;
    int write_error_to = STDERR_FILENO;
};

std::deque<CommandStream> pipe_table(1005);
const int MAX_PIPE = 100;
int fd[MAX_PIPE + 5][2];
std::vector<int> process_list;

// --------------------- FUNCTIONS ---------------------------------

std::string trim(const std::string& s) {
    int left;
    for(left = 0; left < s.length(); left++) {
        if (s[left] != ' ') break;
    }
    
    int right;
    for(right = s.length() - 1; right >= 0; right--) {
        if (s[right] != ' ') break;
    }

    return s.substr(left, right + 1 - left);
}

std::vector<std::string> split_string(const std::string& input_str, const char& de) {
    std::istringstream string_stream(input_str);
    std::string s;
    std::vector<std::string> res;
    while (std::getline(string_stream, s, de)) {
        if (!s.empty()) res.push_back(trim(s));
    }

    return std::move(res);
}

void printenv(const std::string& input_str) {
    auto args = split_string(input_str, ' ');

    if (args.size() < 2) return;
    auto output = getenv(args[1].c_str());

    if (output != nullptr) {
        std::cout << output << std::endl;
    }
}

void setenv(const std::string& input_str) {
    auto args = split_string(input_str, ' ');

    if (args.size() < 2) return;
    setenv(args[1].c_str(), args[2].c_str(), 1);  // assume number of arguments of setenv will be correct
}

std::string parse_numbered_pipe(const std::string& input_str, int& numbered_pipe, bool& use_stderr) {
    auto token = split_string(input_str, ' ');
    std::string new_input_str = input_str;
    numbered_pipe = 0;

    if (token.back()[0] == '|' || token.back()[0] == '!') {
        new_input_str = input_str.substr(0, input_str.size() - token.back().size());
        numbered_pipe = std::stoi(token.back().substr(1));
        if (token.back()[0] == '!') use_stderr = true; else use_stderr = false;
    }

    return std::move(new_input_str);
}

void update_pipe_direction(const int& numbered_pipe, const bool& use_stderr) {
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

void open_pipe(int (&fd)[MAX_PIPE + 5][2], int num) {
    for(int i = 0; i < num; i++) {
        if (pipe(fd[i]) == -1) {
            std::cerr << "Error create pipe: " << errno << std::endl;
        }
    }
}

void close_pipe(int (&fd)[MAX_PIPE + 5][2], int num) {
    for(int i = 0; i < num; i++) {
        close(fd[i][0]);
        close(fd[i][1]);
    }
}

void redirect(int to, int from) {
    if (to != from) {
        if (dup2(to, from) == -1) {
            std::cerr << "Error duplicate file descriptor " << errno << std::endl;
        }
    }
}

pid_t exec_single_command(const std::string& command, int is_first, int read_from, int write_output_to, int write_error_to, int num_opened_pipe) {

    pid_t child_pid;
    while ((child_pid = fork()) < 0) usleep(1000);  // in case of number of process is off limit
    if (child_pid != 0) return child_pid;  // parent process return child pid here

    // only child process run from here
    redirect(read_from, STDIN_FILENO);
    redirect(write_output_to, STDOUT_FILENO);
    redirect(write_error_to, STDERR_FILENO);
    if (is_first && pipe_table[0].write_for_read != -1) close(pipe_table[0].write_for_read);
    
    close_pipe(fd, num_opened_pipe);   // we have duplicated pipe to standard, we can close these pipes now

    // handle redirect to file
    auto token = split_string(command, '>');
    if (token.size() > 1) {
        auto file = token[1];
        int write_to_file = open(file.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
        redirect(write_to_file, STDOUT_FILENO);
        close(write_to_file);
    } 

    // parse input for execvp
    auto args = split_string(token[0], ' ');
    char* args_c[args.size() + 1];
    for(int i = 0; i < args.size(); i++) args_c[i] = const_cast<char*>(args[i].c_str());
    args_c[args.size()] = nullptr;
    
    // exec command
    if (execvp(args[0].c_str(), args_c) == -1) {
        if (errno == 2) {
            std::cout << "Unknown command: [" << args[0] << "]." << std::endl;
        } else {
            std::cerr << "Error execvp process: " << errno << std::endl;
        }
        exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
}

void exec_user_input(std::string& input_str) {
    
    int numbered_pipe = 0;
    bool use_stderr = false;

    input_str = parse_numbered_pipe(input_str, numbered_pipe, use_stderr);
    update_pipe_direction(numbered_pipe, use_stderr);
    auto commands = split_string(input_str, '|');

    int bridge_pipe[2];
    for(int i = 0; i < commands.size(); i += MAX_PIPE) {

        int start = i;
        int end = i + std::min(MAX_PIPE, (int)commands.size() - i);

        open_pipe(fd, end - start);
        bridge_pipe[1] = fd[MAX_PIPE - 1][1];

        int first_input = (start == 0) ? pipe_table[0].read_from : bridge_pipe[0];
        int last_output = (end == commands.size()) ? pipe_table[0].write_output_to : bridge_pipe[1];
        int last_error = (end == commands.size()) ? pipe_table[0].write_error_to : STDERR_FILENO;

        for(int pos = start; pos < end; pos++) {
            auto comm = commands[pos];

            int inp = (pos == start) ? first_input : fd[pos - start - 1][0];
            int out = (pos == end - 1) ? last_output : fd[pos - start][1];
            int err = (pos == end - 1) ? last_error : STDERR_FILENO;
            bool is_first = (start == 0);

            pid_t child_pid;
            child_pid = exec_single_command(comm, is_first, inp, out, err, end - start);
            process_list.push_back(child_pid);
        }

        if (start == 0) close(pipe_table[0].write_for_read);
        if (end != commands.size()) bridge_pipe[0] = dup(fd[MAX_PIPE - 1][0]);
        close_pipe(fd, end - start);  // close duplicated pipe in parent, children will close on its own

        for (auto process_id: process_list) {
            waitpid(process_id, nullptr, 0);
        }
        process_list.clear();
    }
}

// ------------------------------ MAIN ---------------------------------------

int main() {

    setenv("PATH", "bin:.", 1);

    while (true) {
        std::cout << "% ";

        std::string input_str;
        std::getline(std::cin, input_str);
        if (std::cin.eof()) return 0;
        
        pipe_table.emplace_back();

        if (input_str == "exit") {
            return 0;

        } else if (input_str.substr(0, 8) == "printenv") {
            printenv(input_str);

        } else if (input_str.substr(0, 6) == "setenv") {
            setenv(input_str);

        } else if (input_str.empty()) {
            continue;

        } else {
            exec_user_input(input_str);

        }

        pipe_table.pop_front();
    }
}