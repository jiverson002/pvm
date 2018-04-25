#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pdb.h"

#define OK(err) if (err) { printf("notok@%d\n", __LINE__); goto notok; }

enum cmd {
  CONTINUE_CMD,
  FILE_CMD,
  HELP_CMD,
  INVALID_CMD,
  RUN_CMD,
  START_CMD,
  STEP_CMD,
  QUIT_CMD
};

enum state {
  EOF_STATE,
  LOADED_STATE,
  PRE_RUNNING_STATE, /* this must be one less than RUNNING_STATE */
  RUNNING_STATE,
  PRE_STARTED_STATE, /* this must be one less than STARTED_STATE */
  STARTED_STATE,
  START_STATE,
  QUIT_STATE
};

static enum cmd get_cmd(char **buf, size_t *buf_len) {
  ssize_t len;

  printf("(pdb) ");
  len = getline(buf, buf_len, stdin);
  OK(-1 == len);

  (*buf)[len - 1] = '\0'; /* get rid of trailing new line */

  /* Decode command string */
  if (!strncmp(*buf, "c", 2) || !strncmp(*buf, "continue", 9)) {
    return CONTINUE_CMD;
  } else if (!strncmp(*buf, "file", 5)) {
    return FILE_CMD;
  } else if (!strncmp(*buf, "h", 2) || !strncmp(*buf, "help", 5)) {
    return HELP_CMD;
  } else if (!strncmp(*buf, "start", 6)) {
    return START_CMD;
  } else if (!strncmp(*buf, "s", 2) || !strncmp(*buf, "step", 5)) {
    return STEP_CMD;
  } else if (!strncmp(*buf, "r", 2) || !strncmp(*buf, "run", 4)) {
    return RUN_CMD;
  } else if (!strncmp(*buf, "q", 2) || !strncmp(*buf, "quit", 5)) {
    return QUIT_CMD;
  }

notok:
  return INVALID_CMD;
}

int pdb(struct vm *vm, struct os *os, struct prog *prog, int dbg) {
  int ret;
  size_t line_len;
  enum state state;
  char *line;

  line_len = 0;
  line = NULL;

  if (os->mem) {
    ret = vm->burn(os->mem, os->len, os->burn_addr);
    OK(ret);
  }

  if (prog->mem) {
    state = LOADED_STATE;
  }

  if (!dbg) {
    state = PRE_RUNNING_STATE;
  }

  for (;;) {
    switch (state) {
      case EOF_STATE:
        switch (get_cmd(&line, &line_len)) {
          case QUIT_CMD:
            state = QUIT_STATE;
            break;

          case RUN_CMD:
            state = PRE_RUNNING_STATE;
            break;

          case START_CMD:
            state = PRE_STARTED_STATE;
            break;

          default:
            /* badcmd(...); */
            OK(-1);
        }
        break;

      case LOADED_STATE:
        switch (get_cmd(&line, &line_len)) {
          case RUN_CMD:
            state = PRE_RUNNING_STATE;
            break;

          case START_CMD:
            state = PRE_STARTED_STATE;
            break;

          case QUIT_CMD:
            state = QUIT_STATE;
            break;

          default:
            /* badcmd(...); */
            OK(-1);
        }
        break;

      case PRE_RUNNING_STATE:
      case PRE_STARTED_STATE:
        ret = vm->load(prog->mem, prog->len);
        OK(ret);
        ret = vm->init();
        OK(ret);
        state++;
        break;

      case RUNNING_STATE:
        /* TODO check for breakpoint */
        ret = vm->step();
        if (!ret) {
          state = EOF_STATE;
        }
        break;

      case START_STATE:
        switch (get_cmd(&line, &line_len)) {
          case FILE_CMD:
            /* TODO load file */
            /* state = LOADED_STATE */
            break;

          case HELP_CMD:
            /* TODO print help info */
            break;

          case QUIT_CMD:
            state = QUIT_STATE;
            break;

          default:
            /* badcmd(...); */
            OK(-1);
        }
        break;

      case STARTED_STATE:
        switch (get_cmd(&line, &line_len)) {
          case CONTINUE_CMD:
            state = RUNNING_STATE;
            break;

          case HELP_CMD:
            /* TODO print help info */
            break;

          case QUIT_CMD:
            state = QUIT_STATE;
            break;

          case RUN_CMD:
            state = PRE_RUNNING_STATE;
            break;

          case START_CMD:
            state = PRE_STARTED_STATE;
            break;

          case STEP_CMD:
            ret = vm->step();
            if (!ret) {
              state = EOF_STATE;
            }
            break;

          default:
            /* badcmd(...); */
            OK(-1);
        }
        break;

      case QUIT_STATE:
        break;
    }

    if ((EOF_STATE == state && !dbg) || QUIT_STATE == state) {
      break;
    }
  }

  free(line);

  return 0;

notok:
  free(line);

  return -1;
}
