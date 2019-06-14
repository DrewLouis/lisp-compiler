// Nobody will ever need a name longer than 15 chars
#define MAX_TOKENS 512
#define MAX_TOKEN_NAME_LENGTH 16

// Token types
#define LEFT_PAREN 0
#define RIGHT_PAREN 1
#define SYMBOL 2
#define ASSIGN 3
#define INT 4

// Builtin token names
char* TOKEN_TYPE_STRINGS[] =
{"(",
 ")",
 "SYMBOL",
 ":=",
 "INT"};

typedef struct token {
  int type;
  char *name;
} token;

typedef struct tokens {
  int num_tokens;
  int parse_point; // where in the parse list we're working on
  token toks[MAX_TOKENS];  
} tokens;

