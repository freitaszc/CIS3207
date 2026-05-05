#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

char** parse_input(char* line) {
  size_t bufsize = 8;
  size_t position = 0;
  char** args = malloc(bufsize * sizeof(char*));
  char* token;

  if (!args) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, " \t\n");
  while (token != NULL) {
    args[position++] = token;

    if (position >= bufsize) {
      bufsize *= 2;
      args = realloc(args, bufsize * sizeof(char*));
      if (!args) {
        perror("realloc");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, " \t\n");
  }

  args[position] = NULL;
  return args;
}

int extract_redirect(char** args, char** out_file, int* append) {
  *out_file = NULL;
  *append = 0;

  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], ">") == 0) {
      if (args[i + 1] == NULL) return -1;
      *out_file = args[i + 1];

      int j = i;
      while (args[j + 2] != NULL) {
        args[j] = args[j + 2];
        j++;
      }
      args[j] = NULL;

      *append = 0;
      return 1;
    }

    if (strcmp(args[i], ">>") == 0) {
      if (args[i + 1] == NULL) return -1;
      *out_file = args[i + 1];

      int j = i;
      while (args[j + 2] != NULL) {
        args[j] = args[j + 2];
        j++;
      }
      args[j] = NULL;

      *append = 1;
      return 1;
    }
  }

  return 0;
}

int split_pipes(char** args, char*** stages, int max_stages) {
  int n = 0;
  stages[n++] = args;

  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], "|") == 0) {
      if (args[i + 1] == NULL) return -1;  // nothing after pipe
      args[i] = NULL;                      // end this stage
      if (n >= max_stages) return -1;      // too many pipes
      stages[n++] = &args[i + 1];          // start next stage
    }
  }
  return n;  // number of stages
}

void exec_pipe(char** left, char** right, char* out_file_right, int append_right) {
  int pipefd[2];
  if (pipe(pipefd) < 0) {
    perror("pipe");
    return;
  }

  pid_t p1 = fork();
  if (p1 < 0) {
    perror("fork");
    close(pipefd[0]);
    close(pipefd[1]);
    return;
  }

  if (p1 == 0) {
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[0]);
    close(pipefd[1]);
    execvp(left[0], left);
    perror("execvp left");
    _exit(127);
  }

  pid_t p2 = fork();
  if (p2 < 0) {
    perror("fork");
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(p1, NULL, 0);
    return;
  }

  if (p2 == 0) {
    dup2(pipefd[0], STDIN_FILENO);
    close(pipefd[1]);
    close(pipefd[0]);

    if (out_file_right) {
      int flags = O_WRONLY | O_CREAT | (append_right ? O_APPEND : O_TRUNC);
      int fd = open(out_file_right, flags, 0644);
      if (fd < 0) {
        perror("open");
        _exit(127);
      }
      if (dup2(fd, STDOUT_FILENO) < 0) {
        perror("dup2");
        _exit(127);
      }
      close(fd);
    }

    execvp(right[0], right);
    perror("execvp right");
    _exit(127);
  }

  close(pipefd[0]);
  close(pipefd[1]);
  waitpid(p1, NULL, 0);
  waitpid(p2, NULL, 0);
}

int main(void) {
  char *line = NULL;
  size_t len = 0;

  while (1) {
    printf("gabshell> ");
    fflush(stdout);

    ssize_t nread = getline(&line, &len, stdin);
    if (nread == -1) {
      printf("\n");
      break;
    }

    char **args = parse_input(line);

    if (args[0] == NULL) {
      free(args);
      continue;
    }

    // Built-in: exit
    if (strcmp(args[0], "exit") == 0) {
      free(args);
      break;
    }

    // Built-in: cd
    if (strcmp(args[0], "cd") == 0) {
      if (args[1] == NULL) {
        fprintf(stderr, "cd: missing argument\n");
      } else if (chdir(args[1]) != 0) {
        perror("cd");
      }
      free(args);
      continue;
    }

    // Count pipes and validate syntax
    int pipe_count = 0;
    for (int i = 0; args[i] != NULL; i++) {
      if (strcmp(args[i], "|") == 0) {
        pipe_count++;

        if (args[i + 1] == NULL) {               // ends with |
          fprintf(stderr, "pipe error: invalid syntax\n");
          free(args);
          goto next_loop;
        }
        if (strcmp(args[i + 1], "|") == 0) {     // | |
          fprintf(stderr, "pipe error: invalid syntax\n");
          free(args);
          goto next_loop;
        }
      }
    }

    int N = pipe_count + 1; // number of pipeline stages

    // If we have at least one '|', build stages dynamically
    if (N > 1) {
      char ***stages = malloc((size_t)N * sizeof(char **));
      if (!stages) {
        perror("malloc");
        free(args);
        continue;
      }

      // Split args in-place into N argv lists
      int n = 0;
      stages[n++] = args;
      for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
          args[i] = NULL;                 // terminate previous stage argv
          stages[n++] = &args[i + 1];     // next stage starts after '|'
        }
      }

      // Only allow output redirection on the LAST stage
      char *out_file = NULL;
      int append = 0;
      int rr = extract_redirect(stages[N - 1], &out_file, &append);
      if (rr == -1) {
        fprintf(stderr, "redirect error: missing output file\n");
        free(stages);
        free(args);
        continue;
      }

      int prev_read = -1;
      pid_t *pids = malloc((size_t)N * sizeof(pid_t));
      if (!pids) {
        perror("malloc");
        free(stages);
        free(args);
        continue;
      }

      for (int k = 0; k < N; k++) {
        int pipefd[2] = {-1, -1};

        if (k < N - 1) {
          if (pipe(pipefd) < 0) {
            perror("pipe");
            if (prev_read != -1) close(prev_read);
            for (int t = 0; t < k; t++) waitpid(pids[t], NULL, 0);
            free(pids);
            free(stages);
            free(args);
            goto next_loop;
          }
        }

        pid_t pid = fork();
        if (pid < 0) {
          perror("fork");
          if (prev_read != -1) close(prev_read);
          if (pipefd[0] != -1) close(pipefd[0]);
          if (pipefd[1] != -1) close(pipefd[1]);
          for (int t = 0; t < k; t++) waitpid(pids[t], NULL, 0);
          free(pids);
          free(stages);
          free(args);
          goto next_loop;
        }

        if (pid == 0) {
          // Child k: connect stdin/stdout

          if (k > 0) {
            if (dup2(prev_read, STDIN_FILENO) < 0) { perror("dup2"); _exit(127); }
          }

          if (k < N - 1) {
            if (dup2(pipefd[1], STDOUT_FILENO) < 0) { perror("dup2"); _exit(127); }
          } else {
            // last stage: optional > or >>
            if (out_file) {
              int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
              int fd = open(out_file, flags, 0644);
              if (fd < 0) { perror("open"); _exit(127); }
              if (dup2(fd, STDOUT_FILENO) < 0) { perror("dup2"); _exit(127); }
              close(fd);
            }
          }

          // Close fds in child
          if (prev_read != -1) close(prev_read);
          if (pipefd[0] != -1) close(pipefd[0]);
          if (pipefd[1] != -1) close(pipefd[1]);

          execvp(stages[k][0], stages[k]);
          perror("execvp");
          _exit(127);
        }

        // Parent
        pids[k] = pid;

        if (prev_read != -1) close(prev_read);

        if (k < N - 1) {
          close(pipefd[1]);        // parent doesn't write
          prev_read = pipefd[0];   // next stage reads from here
        } else {
          prev_read = -1;
        }
      }

      // Wait for all stages
      for (int k = 0; k < N; k++) {
        waitpid(pids[k], NULL, 0);
      }

      free(pids);
      free(stages);
      free(args);
      continue;
    }

    char *out_file = NULL;
    int append = 0;

    int redir = extract_redirect(args, &out_file, &append);
    if (redir == -1) {
      fprintf(stderr, "redirect error: missing output file\n");
      free(args);
      continue;
    }

    pid_t pid = fork();
    if (pid < 0) {
      perror("fork");
      free(args);
      continue;
    }

    if (pid == 0) {
      if (out_file) {
        int fd = open(out_file, O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), 0644);
        if (fd < 0) { perror("open"); _exit(127); }
        if (dup2(fd, STDOUT_FILENO) < 0) { perror("dup2"); _exit(127); }
        close(fd);
      }

      execvp(args[0], args);
      perror("execvp");
      _exit(127);
    }

    waitpid(pid, NULL, 0);
    free(args);

  next_loop:
    ; // label needs a statement
  }

  free(line);
  return 0;
}