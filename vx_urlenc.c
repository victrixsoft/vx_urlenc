// SPDX-License-Identifier: Apache-2.0
/***********************************************************
 **********************************************************
 *             vx_urlenc - Version 1.1 
 *
 * Author: Adam Danischewski <my first nm(dot)my last nm@gmail.com>
 * Created: 08/21/2024 
 * Modified: 09/03/2024 

 * Fast URL Encoder - Closely conforms to RFC 3986
 *
 * Takes piped input - formats uri's similar to jq -rR '@uri' yet 
 * quicker and conforms more to RFC 3986 [diverges w/most browsers
 * on #'s]. 
 *
 * Optimized (w/help thnks Claude), handles multibyte unicode characters 
 *  
 * Copyright (C) 2024 Victrixsoft
 *
 * Issues: If you find any issues please open a ticket on github:
 *         https://github.com/victrixsoft/vx_urlenc/issues 
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

#define MAX_LINE_LENGTH 8192
#define INITIAL_BUFF_SIZE 512 // Start with a reasonable size
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
    if (str == NULL) return NULL;

    size_t capacity = INITIAL_BUFF_SIZE;  
    char *encoded = malloc(capacity);
    if (encoded == NULL) return NULL;

    char *penc = encoded;
    while (*str && *str != '\n') {
        if (penc - encoded + 4 > capacity) {  // Ensure space for worst case
            capacity *= 2;
            char *new_enc = realloc(encoded, capacity);
            if (new_enc == NULL) {
                free(encoded);
                return NULL;
            }
            penc = new_enc + (penc - encoded);
            encoded = new_enc;
        }

        if (lookup_table[(unsigned char)*str]) {
            *penc++ = *str;
        } else {
            sprintf(penc, "%%%02X", (unsigned char)*str);
            penc += 3;
        }
        str++;
    }
    *penc = '\0';
    return encoded;
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  if (isatty(fileno(stdin))) {
    fprintf(stderr, ">>> Error - It's not a pipe\n");
    return 1;
  }

  char strbuf[MAX_LINE_LENGTH];
  while (fgets(strbuf, sizeof strbuf, stdin) != NULL) {
    char *encoded = url_encode(strbuf);
    if (encoded) {
      printf("%s\n", encoded);
      free(encoded);
    } else {
      fprintf(stderr, ">>> Error encoding line: %s\n", strbuf);
    }
  }

  return 0;
}
