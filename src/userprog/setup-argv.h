#ifndef SETUP_ARGV_H
#define SETUP_ARGV_H
#define STACK_DEBUG(...) printf(__VA_ARGS__)

struct main_args
{
  void (*ret)(void);
  int argc;
  char** argv;
};

int count_args(const char* buf, const char* delimeters);
bool exists_in(char c, const char* d);
void* setup_main_stack(const char* command_line, void* stack_top);
#endif
