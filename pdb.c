#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pdb.h"

#define OK(err)\
  if (err) { printf("notok@%s:%d\n", __FILE__, __LINE__); goto notok; }

enum cmd {
  CONTINUE_CMD,
  FILE_CMD,
  HELP_CMD,
  RUN_CMD,
  START_CMD,
  STEP_CMD,
  QUIT_CMD,
  NUM_CMDS,
  INVALID_CMD
};

enum state {
  EOF_STATE,
  LOADED_STATE,
  PRE_RUNNING_STATE, /* this must be one less than RUNNING_STATE */
  RUNNING_STATE,
  PRE_STARTED_STATE, /* this must be one less than STARTED_STATE */
  STARTED_STATE,
  START_STATE,
  QUIT_STATE,
  NUM_STATES
};

struct {
  enum cmd cmd;
  char const * const short_cmd;
  char const * const long_cmd;
  unsigned long long_cmd_len;
} cmds[NUM_CMDS] = {
  { CONTINUE_CMD, "c", "continue", 9 },
  { FILE_CMD, NULL, "file", 5 },
  { HELP_CMD, "h", "help", 5 },
  { RUN_CMD, "r", "run", 4 },
  { START_CMD, NULL, "start", 6 },
  { STEP_CMD, "s", "step", 5 },
  { QUIT_CMD, "q", "quit", 5 }
};

static enum cmd get_cmd(char **buf, size_t *buf_len) {
  int i;
  unsigned long n;
  ssize_t len;
  char const *s, *l;

  printf("(pdb) ");
  len = getline(buf, buf_len, stdin);
  OK(-1 == len);

  (*buf)[len - 1] = '\0'; /* get rid of trailing new line */

  /* Decode command string */
  for (i=0; i < NUM_CMDS; i++) {
    s = cmds[i].short_cmd;
    l = cmds[i].long_cmd;
    n = cmds[i].long_cmd_len;

    if ((s && !strncmp(*buf, s, 2)) || (l && !strncmp(*buf, l, n))) {
      return cmds[i].cmd;
    }
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
        ret = vm->init();
        OK(ret);
        ret = vm->load(prog->mem, prog->len);
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

      default:
        /* badstate(...); */
        OK(-1);
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
