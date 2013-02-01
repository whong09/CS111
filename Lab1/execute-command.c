// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

void exec_command(command_t c);

struct file_node;
typedef struct file_node *file_node_t;


struct file_node
{
  char *file_name;
  file_node_t next;
  file_node_t prev;
};

struct file_list
{
  file_node_t root;
};
typedef struct file_list *file_list_t;

int
command_status (command_t c)
{
  return c->status;
}

void
execute_simple_command(command_t command)
{
  pid_t pid = fork();
  if(pid > 0) {
    int status;
    if(waitpid(pid, &status, 0) == -1)
      error(1, errno, "Child process exit error");
    command->status = status;
  }
  else if(pid == 0) {
    if(command->input != NULL)
    {
      int new_stdin = open(command->input, O_RDONLY);
      if(new_stdin == -1)
        error(1, errno, "Couldn't open file as input");
      dup2(new_stdin, STDIN_FILENO);
      close(new_stdin);
     }
    if(command->output != NULL)
    {
      int new_stdout = open(command->output, O_WRONLY | O_CREAT, 0664);
      if(new_stdout == -1)
        error(1, errno, "Couldn't open file as output");
      dup2(new_stdout, STDOUT_FILENO);
      close(new_stdout);
    }
    execvp(command->u.word[0], command->u.word);
  } 
  else {
    error (1, 0, "forking error");
  }
}

void
execute_pipe_command(command_t command)
{
   int fd[2];
   pipe(fd);
   pid_t pid;
   if((pid=fork()) == 0)
   {
      dup2(fd[0],0);
      close(fd[1]);
      exec_command(command->u.command[1]);
      close(fd[0]);
      exit(command->u.command[0]->status);
   }
   else if(pid > 0)
   {
      pid_t pid2;
      if((pid2=fork()) == 0) {
        dup2(fd[1],1);
        close(fd[0]);
        exec_command(command->u.command[0]);
        close(fd[1]);
        exit(command->u.command[0]->status);
      }
      else if (pid2 > 0)
      {
        close(fd[0]);
        close(fd[1]);
        int status;
        pid_t wait_pid = waitpid(-1,&status,0);
        if(wait_pid == pid)
	{
           command->status = status;
           waitpid(pid2,&status,0);
           return;
        }
        else if(wait_pid == pid2)
	{
           waitpid(pid,&status,0);
           command->status = status;
           return;
        }
      }
      else 
         error(1, errno, "forking error");
   }
   else 
     error(1, errno, "forking error");
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
 
void execute_and_command(command_t c)
{
	exec_command(c->u.command[0]);
	
	if (c->u.command[0]->status == 0) { 
          exec_command(c->u.command[1]);
          c->status = c->u.command[1]->status;
	} else { 
	  c->status = c->u.command[0]->status;// first one returns false, don't execute second one   	
	}
}

void execute_or_command(command_t c)
{
	exec_command(c->u.command[0]);
	if (c->u.command[0]->status == 0) { // first one returns true, don't execute the second command
		c->status = c->u.command[0]->status;
	} else {
		exec_command(c->u.command[1]);
		c->status = c->u.command[1]->status;
	}
}

void execute_sequence_command(command_t c)
{
	exec_command(c->u.command[0]);
	exec_command(c->u.command[1]);
	c->status = c->u.command[1]->status;
}

void execute_subshell_command(command_t c)
{
	exec_command(c->u.subshell_command);
	c->status = c->u.subshell_command->status;
}

void
exec_command(command_t c)
{
	switch(c->type)
	{
		case AND_COMMAND:
			execute_and_command(c);
			break;
		case OR_COMMAND:
			execute_or_command(c);
			break;
		case PIPE_COMMAND:
			execute_pipe_command(c);
			break;
		case SEQUENCE_COMMAND:
			execute_sequence_command(c);
			break;	
		case SIMPLE_COMMAND:
		execute_simple_command(c);
			break;
		case SUBSHELL_COMMAND:
		execute_subshell_command(c);
			break;
		default:
			error(1, 0, "Invalid command type");
	}
}

void
add_to_list(char* name, file_list_t file_list)
{
  if(file_list->root == NULL)
  {
    file_list->root = (file_node_t) checked_malloc(sizeof(struct file_node));
    file_list->root->file_name = name;
    file_list->root->next = NULL;
    file_list->root->prev = NULL;
  } 
  else
  {
    file_node_t curr = file_list->root;
    file_node_t tmp = curr;
    while(curr != NULL)
    {
      if(strcmp(curr->file_name,name)==0)
         return;
      else
      {
         tmp = curr;
         curr = curr->next;
      }
    }
    curr = checked_malloc(sizeof(struct file_node));
    curr->file_name = name;
    curr->next = NULL;
    curr->prev = tmp;
    tmp->next = curr;
  }
}

void
extract_dependencies(command_t c,file_list_t file_list)
{
  switch(c->type)
  {
    case AND_COMMAND:
    case OR_COMMAND:
    case PIPE_COMMAND:
    case SEQUENCE_COMMAND:
    extract_dependencies(c->u.command[0], file_list);
    extract_dependencies(c->u.command[1], file_list);
    break;
    case SIMPLE_COMMAND:
    if(c->input != NULL)
    {
      add_to_list(c->input,file_list); 
    }
    if(c->output != NULL)
    {
      add_to_list(c->output,file_list);
    }
    break;
    case SUBSHELL_COMMAND:
    if(c->input != NULL)
    {
      add_to_list(c->input,file_list);
    }
    if(c->output != NULL)
    {
      add_to_list(c->output,file_list);
    }
    extract_dependencies(c->u.subshell_command, file_list);
    break;
    default:
      error(1, 0, "Invalid command type");
  }
}

void
execute_command (command_t c, bool time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
   add auxiliary functions and otherwise modify the source code.
   You can also use external functions defined in the GNU C Library.  */
	if(time_travel == 0)
	{
		exec_command(c);
	}
	else
	{
           file_list_t file_list = (file_list_t) checked_malloc(sizeof(struct file_list));
           file_list->root = NULL;
           extract_dependencies(c,file_list);   
           file_node_t list = file_list->root;
/*
           while(list != NULL)
		{
printf("%s\n",list->file_name);
list = list->next;
                 }
*/
 		//error (1, 0, "command execution not yet implemented");
	}
	//error (1, 0, "command execution not yet implemented");
}
