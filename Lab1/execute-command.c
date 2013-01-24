// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int
command_status (command_t c)
{
  return c->status;
}

void execute_simple_command(command_t *c)
{
	//add file I/O
	command_t command = *c;
	pid_t pid = fork();
	if(pid > 0) {
		int status;
		while(waitpid(pid, &status, 0) < 0)
			continue;
		if(WIFEXITED(status))
			command->status = WEXITSTATUS(status);
	}
	else if(pid == 0) {
		execvp(command->u.word[0], command->u.word);
	} else {
		error (1, 0, "forking error");
	}
}

//add execute_pipe_command

void
execute_command (command_t c, bool time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  error (1, 0, "command execution not yet implemented");
}
