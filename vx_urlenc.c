// SPDX-License-Identifier: Apache-2.0
/***********************************************************
 **********************************************************
 
 * Fast URL Encoder - Closely conforms to RFC 3986
 *
 * Takes piped in uri's similar to jq but is quicker and conforms 
 * more to RFC 3986 [diverges w/most browsers on #'s]. 
 *
 * Handles multibyte unicode characters 
 *  
 * Copyright (C) 2024 Victrixsoft
 *
 * Issues: If you find any issues please open a ticket on github!
 *         https://github.com/victrixsoft/vx_urlenc/issues 
 * 
 * Author: Adam Danischewski <my first nm(dot)my last nm@gmail.com>
 * 
 **********************************************************
 ***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#define access _access
#else
#include <unistd.h>
#endif

#define ASCII_SIZE 128

char *url_encode(const char *);
void init_lookup_table(void);
int fileno(FILE *);
void __attribute__((constructor)) init(void);

char lookup_table[ASCII_SIZE] = {0};

void init_lookup_table(void) {
  for (int i = 'a'; i <= 'z'; i++)
    lookup_table[i] = 1;
  for (int i = 'A'; i <= 'Z'; i++)
    lookup_table[i] = 1;
  for (int i = '0'; i <= '9'; i++)
    lookup_table[i] = 1;
  const char *safe = "-_.:/!~*@$&[]+=,;?";
  while (*safe)
    lookup_table[(unsigned char)*safe++] = 1;
}

void __attribute__((constructor)) init() { init_lookup_table(); }

char *url_encode(const char *str) {
  if (str == NULL)
    return NULL;

  char *encoded =
      malloc(strlen(str) * 4 * 3 + 1); // Worst case: every char 4 byte mb
  if (encoded == NULL)
    return NULL;

  char *penc = encoded;
  size_t output_idx = 0;
  while (*str && *str != '\n') { // Stop at newline
    if (lookup_table[(unsigned char)*str]) {
      *penc++ = *str;
      output_idx++;
    } else {
      sprintf(penc, "%%%02X", (unsigned char)*str);
      penc += 3;
      output_idx += 3;
    }
    str++;
  }
  *penc = '\0';
  char *resized_enc = realloc(encoded, output_idx + 1);
  if (resized_enc == NULL) {
    return encoded;
  } else {
    return resized_enc;
  }
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  if (isatty(fileno(stdin))) {
    perror("It's not a pipe\n");
    return 1;
  }

  char strbuf[65536];
  while (fgets(strbuf, sizeof strbuf, stdin) != NULL) {
    char *encoded = url_encode(strbuf);
    if (encoded) {
      printf("%s\n", encoded);
      free(encoded);
    } else {
      fprintf(stderr, "Error encoding line\n");
    }
  }

  return 0;
}
