#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <assert.h>
#include "compiler.h"

void print_usage();
void tokenize(FILE *file, tokens *tok);
tokens *new_tokens();
char *new_token_name();
void output_to_c(tokens *toks, FILE *output_file);
void sexp_to_c(tokens *toks, FILE *output_file);
void assignment_to_c(tokens *toks, FILE *output_file);
void check_tokens_not_NULL(tokens *toks, const char *location);
void check_parse_point(tokens *toks, const char *location);
void increment_parse_point(tokens *toks, const char *location);

void print_token(token *tok);
void print_tokens(tokens *toks);


int main(int argc, char *argv[]) {
  if (argc < 1) {
    fprintf(stderr, "lispv: no file given");
    return 1;
  }

  
  FILE *file = fopen(argv[1], "r");
  if (file == NULL) {
    perror("Failed to open file");
    return 1;
  }
  tokens *toks = new_tokens();
  
  tokenize(file, toks);

  // create output filename, inputname.c
  int input_len = strlen(argv[1]);
  char *output_name = malloc(sizeof(char) * (input_len + 4));
  if (output_name == NULL) {
    perror(NULL);
    exit(EXIT_FAILURE);
  }
  output_name[0] = '.';
  output_name[1] = '/';
  strncpy(&output_name[2], argv[1], input_len);
  output_name[input_len+2] = '.';
  output_name[input_len+3] = 'c';
  output_name[input_len+4] = '\0';
  printf("%s\n", output_name);
  FILE *output_file = fopen(output_name, "w");
  if (output_file == NULL) {
    perror("Couldn't create output file");
    exit(EXIT_FAILURE);
  }
  print_tokens(toks);
  output_to_c(toks, output_file);

  fclose(file);
  fclose(output_file);
  return 0;
}

void print_usage() {
  fprintf(stdin, "usage: lispv file\n");
}

tokens *new_tokens() {
  tokens *new = malloc(sizeof(tokens));
  if (new == NULL) {
    perror("Failed to malloc new tokens");
    exit(EXIT_FAILURE);
  }
  new->num_tokens = 0;
  new->parse_point = -1;
  memset(new->toks, 0, sizeof(token) * MAX_TOKENS);
  return new;
}

void new_token(int type, char *name, tokens *toks) {
  if (toks == NULL) {
    fprintf(stderr, "Called new_token on empty tokens struct");
    exit(EXIT_FAILURE);
  }
  if (toks->num_tokens == MAX_TOKENS) {
    fprintf(stderr, "Can't add new token: max number of tokens reached!");
    exit(EXIT_FAILURE);
  }
  token *new_tok = &toks->toks[toks->num_tokens];
  new_tok->type = type;
  new_tok->name = name;
  ++toks->num_tokens;
}

void tokenize(FILE *file, tokens *tok) {
  char c;
  while ((c = fgetc(file)) != EOF) {
    if (c == '(') {
      new_token(LEFT_PAREN, TOKEN_TYPE_STRINGS[LEFT_PAREN], tok);
    } else if (c == ')') {
      new_token(RIGHT_PAREN, TOKEN_TYPE_STRINGS[RIGHT_PAREN], tok);
    } else if (c == '=') {
      new_token(ASSIGN, TOKEN_TYPE_STRINGS[ASSIGN], tok);
    } else if (isdigit(c)) {
      char *new_name = new_token_name();
      new_name[0] = c;
      // only support single digit ints for now
      new_token(INT, new_name, tok);
    } else if (isalpha(c)) {
      char *new_name = new_token_name();
      new_name[0] = c;
      int i = 1;
      while (i < MAX_TOKEN_NAME_LENGTH-1) {
	c = fgetc(file);
	if (isalpha(c)) {
	  new_name[i++] = c;
	} else {
	  break;
	}
      }
      new_token(SYMBOL, new_name, tok);
    }
  }
}

char *new_token_name() {
  char *new = malloc(sizeof(char) * MAX_TOKEN_NAME_LENGTH);
  if (new == NULL) {
    perror("Failed to malloc token name");
    exit(EXIT_FAILURE);
  }
  memset(new, 0, MAX_TOKEN_NAME_LENGTH);
  return new;
}

void print_token(token *tok) {
  if (tok == NULL) {
    fprintf(stderr, "Tried to print NULL token!\n");
    return;
  }
  printf("<\"%s\", %s>", tok->name, TOKEN_TYPE_STRINGS[tok->type]);
  return;
}

void print_tokens(tokens *toks) {
  if (toks == NULL) {
    fprintf(stderr, "Tried to print NULL tokens struct!\n");
    return;
  }
  for (int i = 0; i < toks->num_tokens; ++i) {
    print_token(&toks->toks[i]);
    printf("   ");
  }
  printf("\n");
}

void output_to_c(tokens *toks, FILE *output_file) {
  check_tokens_not_NULL(toks, __func__);
  toks->parse_point = 0;
  printf("parse_point: %d", toks->parse_point);
  check_parse_point(toks, __func__);
  int *parse_point = &toks->parse_point;
  if (toks->toks[*parse_point].type == LEFT_PAREN) {
    sexp_to_c(toks, output_file);
  }
}

void sexp_to_c(tokens *toks, FILE *output_file) {
  printf("in sexp_to_c\n");
  check_tokens_not_NULL(toks, __func__);
  check_parse_point(toks, __func__);
  int *parse_point = &toks->parse_point;
  assert(toks->toks[*parse_point].type == LEFT_PAREN);

  increment_parse_point(toks, __func__);
  if (toks->toks[*parse_point].type == ASSIGN) {
    assignment_to_c(toks, output_file);
    return;
  } else {
    printf("type: %d\n", toks->toks[*parse_point].type);
  }
}

void assignment_to_c(tokens *toks, FILE *output_file) {
  printf("in a_to_c\n");
  check_tokens_not_NULL(toks, __func__);
  check_parse_point(toks, __func__);
  int *parse_point = &toks->parse_point;
  assert(toks->toks[*parse_point].type == ASSIGN);
  increment_parse_point(toks, __func__);
  if (toks->toks[*parse_point].type != SYMBOL) {
    fprintf(stderr, "variable type must be a symbol!\n");
    exit(EXIT_FAILURE);
  }
  increment_parse_point(toks, __func__);
  if (toks->toks[*parse_point].type != SYMBOL) {
    fprintf(stderr, "variable name must be a symbol!\n");
    exit(EXIT_FAILURE);
  }
  increment_parse_point(toks, __func__);
  if (toks->toks[toks->parse_point].type != INT) {
    fprintf(stderr, "variable must be assigned to a digit!\n");
  }
  
  fprintf(output_file, "%s %s = %s;",	  \
	  toks->toks[*parse_point-2].name, \
	  toks->toks[*parse_point-1].name, \
	  toks->toks[*parse_point].name);
}

void check_tokens_not_NULL(tokens *toks, const char *location) {
  if (toks == NULL) {
    fprintf(stderr, "Called %s on null tokens struct\n", location);
    exit(EXIT_FAILURE);
  } 
}

// Make sure the parse point is at a valid token index
void check_parse_point(tokens *toks, const char *location) {  
  if (toks->parse_point < 0 || toks->parse_point >= toks->num_tokens) {
    fprintf(stderr, "Tried to call %s at invalid parse point:"
	    "%d, num_tokens: %d\n", location, toks->parse_point, toks->num_tokens);
    exit(EXIT_FAILURE);
  }  
}

void increment_parse_point(tokens *toks, const char *location) {
  ++toks->parse_point;
  check_parse_point(toks, location);
}
