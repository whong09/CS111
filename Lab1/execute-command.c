// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

int
command_status (command_t c)
{
  return c->status;
}

void
execute_simple_command(command_t *c)
{
  command_t command = *c;
  int save_stdin;  int save_stdout;
  if(command->input != NULL)
  {
  save_stdin = dup(STDIN_FILENO);
  int new_stdin = open(command->input, O_RDONLY);
  if(new_stdin == -1)
    error(1, errno, "Couldn't open file as input");
  dup2(new_stdin, STDIN_FILENO);
  }
  if(command->output != NULL)
  {
  save_stdout = dup(STDOUT_FILENO);
  int new_stdout = open(command->output, O_WRONLY | O_CREAT, 0664);
  if(new_stdout == -1)
    error(1, errno, "Couldn't open file as output");
  dup2(new_stdout, STDOUT_FILENO);
  }
  pid_t pid = fork();
  if(pid > 0) {
  int status;
  while(waitpid(pid, &status, 0) < 0)
    continue;
  if(!WIFEXITED(status))
    error(1, errno, "Child process exit error");
  if(command->input != NULL)
  {
    if(dup2(save_stdin, STDIN_FILENO) == -1)
    {
    print_command(command);
    error(1, errno, "Couldn't reopen stdin");
    }
  }
  if(command->output != NULL)
  {
    if(dup2(save_stdout, STDOUT_FILENO) == -1)
    {
    print_command(command);
    error(1, errno, "Couldn't reopen stdout");
    }
  }
  command->status = WEXITSTATUS(status);
  }
  else if(pid == 0) {
  execvp(command->u.word[0], command->u.word);
  } else {
  error (1, 0, "forking error");
  }
}

void
execute_pipe_command(command_t *c)
{
  command_t command = *c;
  pid_t pid = fork();
  if(pid > 0) {
    int status;
    while(waitpid(pid, &status, 0) < 0)
      continue;
    if(!WIFEXITED(status))
      error(1, errno, "Child exit error");
    command->status = WEXITSTATUS(status);
    return;
  } else if (pid == 0) {
    int fd[2]; pipe(fd);
    pid = fork();
    if(pid > 0) {
      close(fd[0]);
      dup2(fd[1], STDOUT_FILENO);
      char **args = command->u.command[0]->u.word;
      execvp(args[0], args);
    } else if (pid == 0) {
      close(fd[1]);
      dup2(fd[0], STDIN_FILENO);
      char **args = command->u.command[1]->u.word;
      execvp(args[0], args);
    }
  } else {
    error(1, errno, "forking error");
  }
}

/*void
execute_pipe_command (command_t *c)
{
  command_t command = *c;
  int save_stdin = dup(STDIN_FILENO);
  int save_stdout = dup(STDOUT_FILENO);
  while(command->type == PIPE_COMMAND)
  {
  int fd[2];
  if(pipe(fd) == -1)
    error(1, errno, "Couldn't open a pipe");
  dup2(fd[1], STDOUT_FILENO);
  pid_t pid = fork();
  if(pid > 0) {
    dup2(fd[0], STDIN_FILENO);
    command = command->u.command[0];
  } else if (pid == 0) {
    char **simple_command = command->u.command[1]->u.word;
    execvp(simple_command[0], simple_command);
  } else {
    error (1, errno, "forking error");  
  }
  }
  dup2(save_stdout, STDOUT_FILENO);
  pid_t pid= fork();
  if(pid > 0) {
  int status;
  while(waitpid(pid, &status, 0) < 0)
    continue;
  if(!WIFEXITED(status))
    error(1, errno, "Child process exit error");
  (*c)->status = WEXITSTATUS(status);
  dup2(save_stdin, STDIN_FILENO);  
  } else if(pid == 0) {
  char **simple_command = command->u.command[0]->u.word;
  execvp(simple_command[0], simple_command);
  } else {
  error (1, errno, "forking error");
  }
}*/

void
execute_command (command_t c, bool time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
   add auxiliary functions and otherwise modify the source code.
   You can also use external functions defined in the GNU C Library.  */
  error (1, 0, "command execution not yet implemented");
}
