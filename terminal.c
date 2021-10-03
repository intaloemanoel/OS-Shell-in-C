#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>

#define READLINE_BUFSIZE 1024

int cmdCD(char **args);
int cmdHELP(char **args);
int cmdEXIT(char **args);

void clear() {
  printf("\033[H\033[J");
}

void green() {
    printf("\033[0;32m");
}

void resetColor() {
  printf("\033[0m");
}

void printDir()
{
  char cwd[1024];
  getcwd(cwd, sizeof(cwd));
  printf("💾 The actual Directory is %s \n", cwd);
}

char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &cmdCD,
  &cmdHELP,
  &cmdEXIT
};

int numBuiltins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int cmdCD(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "⚠️ Expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("cmd");
    }
  }

  printDir();

  return 1;
}

int cmdHELP(char **args)
{
  int i;
  printf("⚠️  Welcome to the AIT SHELL help area!\n");
  printf("Type program names/arguments and then hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < numBuiltins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  return 1;
}

int cmdEXIT(char **args)
{
  return 0;
}

char *readLine(void)
{
  char *line = NULL;
  ssize_t bufsize = 0;

  if (getline(&line, &bufsize, stdin) == -1){
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);  // We recieved an EOF
    } else  {
      perror("readline");
      exit(EXIT_FAILURE);
    }
  }

  return line;
}

char **splitLine(char *line)
{
  int bufsize = 64, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "ERROR: Found an issue with allocation.\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, " \t\r\n\a");
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += 64;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "ERROR: Found an issue with allocation.\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, " \t\r\n\a");
  }
  tokens[position] = NULL;
  return tokens;
}

int launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("lsh");
  } else {
    // Parent
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < numBuiltins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return launch(args);
}

void mainLoop(){
    char *line;
    char **args;
    int status;
    char* username = getenv("USER");

    do {
        green();
        printf("@%s", username);
        resetColor();
        printf("> ");
        line = readLine();
        args = splitLine(line);
        status = execute(args);

        free(line);
        free(args);
    } while (status);
}

int main()
{
    clear();

    mainLoop();

    return 0;
}