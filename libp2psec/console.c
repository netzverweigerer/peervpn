

#ifndef F_CONSOLE_C
#define F_CONSOLE_C


#include "mapstr.c"
#include "util.c"


#define consoleMAXARGS 10


struct s_console_args {
  void *arg[consoleMAXARGS];
  int len[consoleMAXARGS];
  int count;
};
struct s_console_command {
  void (*function)(struct s_console_args *);
  struct s_console_args fixed_args;
};
struct s_console {
  struct s_map commanddb;
  char *inbuf;
  char *outbuf;
  char prompt[32];
  int prompt_length;
  int prompt_enabled;
  int buffer_size;
  int inbuf_count;
  int outbuf_start;
  int outbuf_count;
};


#define consoleArgs0() consoleArgsN(0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)
#define consoleArgs1(arg0) consoleArgsN(1, arg0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)
#define consoleArgs2(arg0, arg1) consoleArgsN(2, arg0, arg1, NULL, NULL, NULL, NULL, NULL, NULL, NULL)
#define consoleArgs3(arg0, arg1, arg2) consoleArgsN(3, arg0, arg1, arg2, NULL, NULL, NULL, NULL, NULL, NULL)
#define consoleArgs4(arg0, arg1, arg2, arg3) consoleArgsN(4, arg0, arg1, arg2, arg3, NULL, NULL, NULL, NULL, NULL)
#define consoleArgs5(arg0, arg1, arg2, arg3, arg4) consoleArgsN(5, arg0, arg1, arg2, arg3, arg4, NULL, NULL, NULL, NULL)
#define consoleArgs6(arg0, arg1, arg2, arg3, arg4, arg5) consoleArgsN(6, arg0, arg1, arg2, arg3, arg4, arg5, NULL, NULL, NULL)
#define consoleArgs7(arg0, arg1, arg2, arg3, arg4, arg5, arg6) consoleArgsN(7, arg0, arg1, arg2, arg3, arg4, arg5, arg6, NULL, NULL)
#define consoleArgs8(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7) consoleArgsN(8, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, NULL)
#define consoleArgs9(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) consoleArgsN(9, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
static struct s_console_args consoleArgsN(int argc, void *arg0, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6, void *arg7, void *arg8) {
  struct s_console_args ret = { .count = argc, .arg[0] = arg0, .arg[1] = arg1, .arg[2] = arg2, .arg[3] = arg3, .arg[4] = arg4, .arg[5] = arg5, .arg[6] = arg6, .arg[7] = arg7, .arg[8] = arg8 };
  return ret;
}


#define consoleRegisterCommand(console, name, function, args) consoleRegisterCommandN(console, name, strlen(name), function, args)
static int consoleRegisterCommandN(struct s_console *console, const char *name, const int namelen, void (*function)(struct s_console_args *), struct s_console_args args) {
  const struct s_console_command cmd = { .function = function, .fixed_args = args };
  char filtered_name[namelen];
  utilStringFilter(filtered_name, name, namelen);
  return mapStrNAdd(&console->commanddb, filtered_name, namelen, &cmd);
}


#define consoleUnregisterCommand(console, name) consoleUnregisterCommandN(console, name, strlen(name))
static int consoleUnregisterCommandN(struct s_console *console, const char *name, const int namelen) {
  char filtered_name[namelen];
  utilStringFilter(filtered_name, name, namelen);
  return mapStrNRemove(&console->commanddb, filtered_name, namelen);
}


#define consoleGetCommand(console, name) consoleGetCommandN(console, name, strlen(name))
static struct s_console_command *consoleGetCommandN(struct s_console *console, const char *name, const int namelen) {
  char filtered_name[namelen];
  utilStringFilter(filtered_name, name, namelen);
  return mapStrNGet(&console->commanddb, filtered_name, namelen);
}


static int consoleOut(struct s_console *console, const char *data, const int datalen) {
  int buffer_size = console->buffer_size;
  int count = console->outbuf_count;
  int spos = ((console->outbuf_start + console->outbuf_count) % buffer_size);
  int new_count = (count + datalen);
  int i, j;
  if(datalen > 0) {
    if(new_count < buffer_size) {
      j = 0;
      i = (buffer_size - spos);
      if(datalen > i) {
        memcpy(&console->outbuf[spos], data, i);
        spos = ((spos + i) % buffer_size);
        j = i;
      }
      i = (datalen - j);
      memcpy(&console->outbuf[spos], &data[j], i);
      console->outbuf_count = new_count;
      return 1;
    }
    else {
      return 0;
    }
  }
  else {
    return 1;
  }
}


static int consolePrompt(struct s_console *console) {
  if(console->prompt_enabled) {
    return consoleOut(console, console->prompt, console->prompt_length);
  }
  else {
    return consoleOut(console, "", 0);
  }
}


static int consoleNL(struct s_console *console) {
  return consoleOut(console, "\r\n", 2);
}


static int consoleGetPromptStatus(struct s_console *console) {
  return console->prompt_enabled;
}


static void consoleSetPromptStatus(struct s_console *console, const int status) {
  console->prompt_enabled = status;
}


#define consoleSetPrompt(console, prompt) consoleSetPromptN(console, prompt, strlen(prompt))
static int consoleSetPromptN(struct s_console *console, const char *prompt, const int prompt_length) {
  if(prompt != NULL && prompt_length > 0) {
    if(prompt_length < 32) {
      memcpy(console->prompt, prompt, prompt_length);
      console->prompt_length = prompt_length;
      return 1;
    }
    else {
      return 0;
    }
  }
  else {
    console->prompt_length = 0;
    return 1;
  }
}


#define consoleMsg(console, msg) consoleMsgN(console, msg, strlen(msg))
static int consoleMsgN(struct s_console *console, const char *msg, const int msglen) {
  return consoleOut(console, msg, msglen);
}


static void consoleProcessLine(struct s_console *console) {
  int line_length = console->inbuf_count;
  const struct s_console_command *cmd;
  struct s_console_args exec_args = { .count = 0 };
  struct s_console_args line_args= { .count = 0 };
  int last;
  int i, j;
  console->inbuf_count = 0;
  if(line_length > 0) {
    if(line_length <= (console->buffer_size)) {
      line_args.count = 0;
      line_args.len[0] = 0;
      last = 0;
      i = 0;
      while((i < line_length) && (line_args.count < consoleMAXARGS)) {
        if(console->inbuf[i] == '\0') {
          line_args.len[line_args.count] = (i - last);
          line_args.arg[line_args.count] = &console->inbuf[last];
          line_args.count++;
          last = i+1;
        }
        i++;
      }
      
      if(line_args.len[0] > 0) {
        cmd = consoleGetCommandN(console, line_args.arg[0], line_args.len[0]);
        if(cmd == NULL) {
          consoleMsg(console, "error: unknown command");
          consoleNL(console);
        }
        else {
          j = 1;
          i = 0;
          while(i < cmd->fixed_args.count) {
            if(cmd->fixed_args.arg[i] == NULL) {
              if(j < line_args.count) {
                exec_args.arg[i] = line_args.arg[j];
                exec_args.len[i] = line_args.len[j];
                j++;
              }
              else {
                exec_args.arg[i] = NULL;
              }
            }
            else {
              exec_args.arg[i] = cmd->fixed_args.arg[i];
              exec_args.len[i] = cmd->fixed_args.len[i];
            }
            i++;
          }
          exec_args.count = cmd->fixed_args.count;
          
          cmd->function(&exec_args);
        }
      }
    }
    else {
      consoleMsg(console, "error: too much input");
      consoleNL(console);
    }
    consolePrompt(console);
  }
}


static int consoleWrite(struct s_console *console, const char *input, const int length) {
  int i = 0;
  int zero;
  int escape;
  int ignore;
  int c;
  while(i < length) {
    zero = 0; escape = 0; ignore = 0;
    if(console->inbuf_count > 0) {
      switch(console->inbuf[((console->inbuf_count)-1)]) {
        case '\0': zero = 1; break;
        case '\\': escape = 1; break;
        case '#': ignore = 1; break;
      }
    }
    c = input[i];
    if(c == '\0') {
      console->inbuf_count = 0;
    }
    else if(c == '\n') {
      if(ignore) {
        console->inbuf[--console->inbuf_count] = '\0';
        if(console->inbuf_count > 0) if(console->inbuf[((console->inbuf_count)-1)] == '\0') zero = 1;
      }
      if(!zero) console->inbuf[console->inbuf_count++] = '\0';
      consoleProcessLine(console);
    }
    else if(console->inbuf_count <= console->buffer_size) {
      if(c == '#') {
        if(!ignore) console->inbuf[console->inbuf_count++] = '#';
      }
      else if(!ignore) {
        if(c == ' ') {
          if(escape) {  
            console->inbuf[(console->inbuf_count - 1)] = ' ';
          }
          else {
            if(!zero) console->inbuf[console->inbuf_count++] = '\0';
          }
        }
        else if(c == '\t') {
          if(!zero) console->inbuf[console->inbuf_count++] = '\0';
        }
        else if(c < 32 || c > 126) {
        }
        else {
          console->inbuf[console->inbuf_count++] = c;
        }
      }
    }
    i++;
  }
  return i;
}


static int consoleRead(struct s_console *console, char *output, const int length) {
  int buffer_size = console->buffer_size;
  int len = console->outbuf_count;
  int spos = console->outbuf_start;
  int i, j;
  if(length < len) len = length;
  if(len > 0) {
    j = 0;
    i = (buffer_size - spos);
    if(len > i) {
      memcpy(output, &console->outbuf[spos], i);
      spos = ((spos + i) % buffer_size);
      j = i;
    }
    i = (len - j);
    memcpy(&output[j], &console->outbuf[spos], i);
    spos = ((spos + i) % buffer_size);
    console->outbuf_start = spos;
    console->outbuf_count = console->outbuf_count - len;
    return len;
  }
  else {
    return 0;
  }
}


static void consoleInit(struct s_console *console) {
  mapInit(&console->commanddb);
  console->inbuf_count = 0;
  console->outbuf_start = 0;
  console->outbuf_count = 0;
  console->prompt_length = 0;
  console->prompt_enabled = 0;
}


static int consoleCreate(struct s_console *console, const int db_size, const int key_size, const int buffer_size) {
  if(!((db_size > 0) && (key_size > 0) && (buffer_size > 0))) return 0;
  
  void *inmem = NULL;
  void *outmem = NULL;
  if(!mapCreate(&console->commanddb, db_size, key_size, sizeof(struct s_console_command))) { return 0; }
  if((inmem = malloc(buffer_size + 4)) == NULL) { mapDestroy(&console->commanddb); return 0; }
  if((outmem = malloc(buffer_size)) == NULL) { free(inmem); mapDestroy(&console->commanddb); return 0; }
  console->inbuf = inmem;
  console->outbuf = outmem;
  console->buffer_size = buffer_size;
  consoleInit(console);
  
  return 1;
}


static int consoleDestroy(struct s_console *console) {
  if(!((console != NULL) && (console->inbuf != NULL) && (console->outbuf != NULL))) return 0;
  free(console->outbuf);
  free(console->inbuf);
  mapDestroy(&console->commanddb);
  console->outbuf = NULL;
  console->inbuf = NULL;
  return 1;
}


#endif // F_CONSOLE_C
