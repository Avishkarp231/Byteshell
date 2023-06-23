// ACM OPEN PROJECT DONE BY AVISHKAR - 21116069.

//importing required header files.
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <memory>
using namespace std;

/*
  Function Declarations for builtin shell commands:
 */
int lsh_cd(vector<string>& args);
int lsh_help(vector<string>& args);
int lsh_exit(vector<string>& args);
int display_history(vector<string>& args);
int lsh_ls(vector<string>& args);
int lsh_pwd(vector<string>& args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
vector<string> builtin_str = {
    "cd",
    "help",
    "exit",
    "history",
    "ls",
    "pwd"};

int (*builtin_func[])(vector<string>&) = {
    &lsh_cd,
    &lsh_help,
    &lsh_exit,
    &display_history,
    &lsh_ls,
    &lsh_pwd};

struct Node
{
    string str;
    shared_ptr<Node> next;
};
shared_ptr<Node> head = nullptr;
shared_ptr<Node> cur = nullptr;

void add_to_hist(vector<string>& args);

string strAppend(const string& str1, const string& str2)
{
    return str1 + str2;
}

int display_history(vector<string>& args)
{
    shared_ptr<Node> ptr = head;
    int i = 1;
    while (ptr != nullptr)
    {
        cout << " " << i++ << " " << ptr->str << endl;
        ptr = ptr->next;
    }
    return 1;
}

int lsh_num_builtins()
{
    return builtin_str.size();
}

/*
  Builtin function implementations.
*/

/**
   @brief Builtin command: change directory.
   @param args List of args.  args[0] is "cd".  args[1] is the directory.
   @return Always returns 1, to continue executing.
 */
int lsh_cd(vector<string>& args)
{
    if (args.size() < 2)
    {
        cerr << "lsh: expected argument to \"cd\"" << endl;
    }
    else
    {
        if (chdir(args[1].c_str()) != 0)
        {
            perror("lsh");
        }
    }
    return 1;
}

/**
   @brief Builtin command: print help.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
int lsh_help(vector<string>& args)
{
    cout << "Type program names and arguments, and hit enter." << endl;
    cout << "The following are built in:" << endl;

    for (const auto& command : builtin_str)
    {
        cout << "  " << command << endl;
    }

    cout << "Use the man command for information on other programs." << endl;
    return 1;
}

/**
   @brief Builtin command: exit.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int lsh_exit(vector<string>& args)
{
    return 0;
}



/**
   @brief Builtin command: print current working directory.
   @param args List of args. Not examined.
   @return Always returns 1, to continue executing.
 */
int lsh_pwd(vector<string>& args)
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != nullptr)
    {
        cout << cwd << endl;
    }
    else
    {
        perror("lsh");
    }
    return 1;
}

/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */
int lsh_launch(vector<string>& args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        // Child process
        vector<char*> c_args(args.size() + 1);
        for (size_t i = 0; i < args.size(); ++i)
        {
            c_args[i] = const_cast<char*>(args[i].c_str());
        }
        c_args[args.size()] = nullptr;

        if (execvp(c_args[0], c_args.data()) == -1)
        {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        // Error forking
        perror("lsh");
    }
    else
    {
        // Parent process
        do
        {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int lsh_execute(vector<string>& args)
{
    if (args.empty())
    {
        // An empty command was entered.
        return 1;
    }

    for (size_t i = 0; i < lsh_num_builtins(); ++i)
    {
        if (args[0] == builtin_str[i])
        {
            return (*builtin_func[i])(args);
        }
    }

    return lsh_launch(args);
}

/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
string lsh_read_line()
{
    string line;
    if (!getline(cin, line))
    {
        if (cin.eof())
        {
            exit(EXIT_SUCCESS); // We received an EOF
        }
        else
        {
            perror("lsh: getline\n");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */
vector<string> lsh_split_line(const string& line)
{
    vector<string> tokens;
    size_t pos = 0;
    size_t found = 0;
    while ((found = line.find_first_of(LSH_TOK_DELIM, pos)) != string::npos)
    {
        if (found != pos)
        {
            tokens.push_back(line.substr(pos, found - pos));
        }
        pos = found + 1;
    }
    if (pos < line.size())
    {
        tokens.push_back(line.substr(pos));
    }
    return tokens;
}

void add_to_hist(vector<string>& args)
{
    shared_ptr<Node> ptr = make_shared<Node>();

    if (head == nullptr)
    {
        head = make_shared<Node>();
        head->str = "";

        string str1 = " ";

        if (args.size() > 0)
            head->str = strAppend(args[0], str1);

        if (args.size() > 1)
            head->str = strAppend(head->str, args[1]);

        head->next = nullptr;
        cur = head;
    }
    else
    {
        ptr = make_shared<Node>();
        string str1 = " ";

        if (args.size() > 0)
            ptr->str = strAppend(args[0], str1);

        if (args.size() > 1)
            ptr->str = strAppend(ptr->str, args[1]);

        cur->next = ptr;
        ptr->next = nullptr;
        cur = ptr;
    }
}

/**
   @brief Loop getting input and executing it.
 */
void lsh_loop()
{
    int status;
    vector<string> args;
    string line;

    do
    {
        cout << "> ";
        line = lsh_read_line();
        args = lsh_split_line(line);
        add_to_hist(args);
        status = lsh_execute(args);
    } while (status);
}
/**
   @brief Builtin command: list files in directory.
   @param args List of args. Not examined.
   @return Always returns 1, to continue executing.
 */
int lsh_ls(vector<string>& args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        // Child process
        vector<char*> c_args(args.size() + 1);
        for (size_t i = 0; i < args.size(); ++i)
        {
            c_args[i] = const_cast<char*>(args[i].c_str());
        }
        c_args[args.size()] = nullptr;

        if (execvp(c_args[0], c_args.data()) == -1)
        {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        // Error forking
        perror("lsh");
    }
    else
    {
        // Parent process
        do
        {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int main()
{
    // Run command loop.
    lsh_loop();

    return EXIT_SUCCESS;
}

//done.