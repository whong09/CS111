// UCLA CS 111 Lab 1 command execution

#include "alloc.h"
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
#include <string.h>

typedef struct dependency_node *dependency_node_t;
typedef struct file_node *file_node_t;
typedef struct pid_node *pid_node_t;

dependency_node_t dependency_root;
int id = 1;

struct file_node {
  char *file_name;
  file_node_t next;
  file_node_t prev;
};

struct pid_node {
  pid_t pid;
  pid_node_t next;
};

struct dependency_node {
  char *file_name;
  pid_node_t waiting_list;
  dependency_node_t next;
};

void exec_command(command_t c);
file_node_t extract_dependencies(command_t c);

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
print_dependency ()
{
  dependency_node_t d_node = dependency_root;
  while(d_node != NULL)
  {
    printf("%s:", d_node->file_name);
    pid_node_t p_node = d_node->waiting_list;
    while(p_node != NULL)
    {
      printf(" %d", p_node->pid);
      p_node = p_node->next;
    }
    printf("\n");
    d_node = d_node->next;
  }
}

bool
is_runnable(pid_t pid)
{
  dependency_node_t d_node = dependency_root;
  while(d_node != NULL)
  {
    pid_node_t p_node = d_node->waiting_list;
    if(p_node != NULL && p_node->pid == pid)
      continue;
    while(p_node != NULL)
    {
      if(p_node->pid == pid)
        return false;
      p_node = p_node->next;
    }
    d_node = d_node->next;
  }
}

file_node_t
extract_simple_dependencies(command_t c)
{
  file_node_t files = NULL;
  if(c->input != NULL)
  {
    files = checked_malloc(sizeof(struct file_node));
    files->prev = NULL; files->next = NULL;
    files->file_name = c->input;
  }
  if(c->output != NULL)
  {
    file_node_t output = NULL;
    if(files == NULL)
    {
      files = checked_malloc(sizeof(struct file_node));
      files->prev = NULL; files->next = NULL;
      output = files;
    }
    else
    {
      files->next = checked_malloc(sizeof(struct file_node));
      output = files->next; 
      output->prev = files; output->next = NULL;
    }
    output->file_name = c->output;
  }
  return files;
}

file_node_t
extract_complex_dependencies(command_t c)
{
  file_node_t left = extract_dependencies(c->u.command[0]);
  file_node_t right = extract_dependencies(c->u.command[1]);
  if(left == NULL)
    return right;
  else if(right == NULL)
    return left;
  file_node_t l_node = left;
  file_node_t l_tail = l_node;
  while(l_node != NULL)
  {
    file_node_t r_node = right;
    while(r_node != NULL)
    {
      if(strcmp(r_node->file_name, l_node->file_name)== 0)
      {
        file_node_t temp = r_node;
        if(r_node->prev != NULL && r_node->next != NULL)
        {
          r_node->prev->next = r_node->next;
          r_node->next->prev = r_node->prev;
          r_node = r_node->next;
        }
        else if(r_node->prev == NULL && r_node->next != NULL)
        {
          r_node->next->prev = NULL;
          right = r_node = r_node->next;
        }
        else if(r_node->prev != NULL && r_node->next == NULL)
        {
          r_node->prev->next = NULL;
          r_node = NULL;
        }
        else
        {
          right = NULL;
          r_node = NULL;
        }
        free(temp);
      }
      else
        r_node = r_node->next;
    }
    l_tail = l_node;
    l_node = l_node->next;
  }
  if(right != NULL)
  {
    l_tail->next = right;
    right->prev = l_tail;
  }
  return left;
}

file_node_t
extract_dependencies(command_t c)
{
  switch(c->type)
  {
    case SIMPLE_COMMAND:
      return extract_simple_dependencies(c);  
    case SUBSHELL_COMMAND:
      return extract_dependencies(c->u.subshell_command);
    default:
      return extract_complex_dependencies(c);
  }
}

void
add_dependencies(command_t c, pid_t pid)
{
  file_node_t f_root = extract_dependencies(c);
  dependency_node_t d_node = dependency_root;
  dependency_node_t d_prev = d_node;
  while(d_node != NULL)
  {
    file_node_t f_node = f_root;
    while(f_node != NULL)
    {
      if(strcmp(d_node->file_name, f_node->file_name) == 0)
      {
        if(f_node == f_root)
          f_root = f_node->next;
        pid_node_t p_node = d_node->waiting_list;
        pid_node_t p_prev = p_node;
        if(p_node == NULL)
        {
          p_node = checked_malloc(sizeof(struct pid_node));
          goto make_node;
        }
        while(p_node != NULL)
        {
          p_prev = p_node;
          p_node = p_node->next;
        }
        p_node = checked_malloc(sizeof(struct pid_node));
        make_node:;
        p_node->pid = pid;
        p_node->next = NULL;
        if(d_node->waiting_list != NULL)
          p_prev->next = p_node;
        else
          d_node->waiting_list = p_node;
        if(f_node->prev != NULL)
          f_node->prev->next = f_node->next;
        if(f_node->next != NULL)
          f_node->next->prev = f_node->prev;
        if(f_node->next == NULL && f_node->prev == NULL)
          break;
        file_node_t temp = f_node;
        f_node = f_node->next;
        free(temp);
      }
      else
        f_node = f_node->next;
    }
    d_prev = d_node;
    d_node = d_node->next;
  }
  if(f_root != NULL)
  {
    file_node_t f_node = f_root;
    dependency_node_t d_tail = checked_malloc(sizeof(struct dependency_node));
    dependency_node_t tail_node = d_tail;
    dependency_node_t tail_prev = tail_node;
    while(f_node != NULL)
    {
      tail_node->file_name = f_node->file_name;
      tail_node->waiting_list = checked_malloc(sizeof(struct pid_node));
      tail_node->waiting_list->pid = pid;
      tail_node->waiting_list->next = NULL;
      tail_node->next = checked_malloc(sizeof(struct dependency_node));
      tail_prev = tail_node;
      tail_node = tail_node->next;
      f_node = f_node->next;
    }
    free(tail_node);
    tail_prev->next = NULL;
    if(d_prev != NULL)
      d_prev->next = d_tail;
    else
      dependency_root = d_tail;
  }
}

void
remove_dependencies(pid_t pid)
{
  dependency_node_t d_node = dependency_root;
  while(d_node != NULL)
  {
    pid_node_t p_node = d_node->waiting_list;
    pid_node_t p_prev = d_node->waiting_list;
    while(p_node != NULL)
    {
      if(p_node->pid != pid)
      {
        p_prev = p_node;
        p_node = p_node->next;
      }
      else if(p_node == d_node->waiting_list)
      {
        pid_node_t temp = p_node;
        p_node = p_node->next;
        d_node->waiting_list = p_node;
        p_prev = p_node;
        free(temp);
      }
      else if(p_node != d_node->waiting_list)
      {
        pid_node_t temp = p_node;
        p_node = p_node->next;
        p_prev->next = p_node;
        free(temp);
      }
    }
    d_node = d_node->next;
  }
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
  } else if(pid > 0) {
    pid_t pid2;
    if((pid2=fork()) == 0) {
      dup2(fd[1],1);
      close(fd[0]);
      exec_command(command->u.command[0]);
      close(fd[1]);
      exit(command->u.command[0]->status);
    } else if (pid2 > 0) {
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
    } else { error(1, errno, "forking error"); }    
  } else { error(1, errno, "forking error"); }
}
 
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
/*
  if(!time_travel)
    exec_command(c);
  else
  {
    pid_t pid = fork();
    if(pid > 0) {
      return;
    } else if(pid == 0) {
      add_dependencies(c, getpid());
      while(!is_runnable)
        yield();
      exec_command(c);
      remove_dependencies(getpid());
      exit(0);
    } else { error(1, errno, "forking error"); }
  }
*/
  add_dependencies(c, id++);
  add_dependencies(c, id++);
  add_dependencies(c, id++);
  remove_dependencies(id-1);
  print_dependency();
}
