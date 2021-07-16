#include <deque>
#include <vector>
#include <string>
#include <unistd.h>

class NPShell {

public:
    struct CommandStream {
        int read_from = STDIN_FILENO;
        int write_for_read = -1;
        int write_output_to = STDOUT_FILENO;
        int write_error_to = STDERR_FILENO;

        CommandStream() {
            read_from = STDIN_FILENO;
            write_for_read = -1;
            write_output_to = STDOUT_FILENO;
            write_error_to = STDERR_FILENO;
        }

        CommandStream(const CommandStream &c) {
            this->read_from = c.read_from;
            this->write_for_read = c.write_for_read;
            this->write_output_to = c.write_output_to;
            this->write_error_to = c.write_error_to;
        }

        CommandStream &operator=(const CommandStream &c) = default;
    };

    std::deque<CommandStream> pipe_table;
    static const int MAX_PIPE = 100;
    int process_fd[MAX_PIPE + 5][2]{};
    std::vector<int> process_list;

    NPShell();

    NPShell(const NPShell &s);

    NPShell &operator=(const NPShell &u) = default;

    static void npshell_printenv(const std::string &input_str);

    static void npshell_setenv(const std::string &input_str);

    static std::string parse_numbered_pipe(const std::string &input_str, int &numbered_pipe, bool &use_stderr);

    static void open_pipe(int (&fd)[MAX_PIPE + 5][2], int num);

    static void close_pipe(int (&fd)[MAX_PIPE + 5][2], int num);

    static void redirect(int to, int from);

    void update_numbered_pipe_direction(const int &numbered_pipe, const bool &use_stderr);

    pid_t exec_single_command(const std::string &command, int is_first, int read_from, int write_output_to,
                              int write_error_to, int num_opened_pipe, int client_fd);

    void exec_numbered_pipe_command(std::string &input_str);

    void run_interactive_shell();

    pid_t exec_long_command(const std::string &input_str, int wait_fd, int middle_stderr, bool is_remote_call);
};
