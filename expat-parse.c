// cc expat-parse.c `pkg-config --libs --cflags expat` -Wall && ./a.out
// https://github.com/libexpat/libexpat/blob/master/expat/examples/outline.c

#include <stdio.h>

#include <expat.h>

#define BUFFSIZE 8192
char Buff[BUFFSIZE];

int depth;

static void XMLCALL start(void *data, const XML_Char *el,
                          const XML_Char **attr) {
  for (unsigned i = 0; i < depth; i++)
    printf("  ");

  printf("<%s", el);

  for (unsigned i = 0; attr[i]; i += 2) {
    printf(" %s='%s'", attr[i], attr[i + 1]);
  }

  printf(">\n");
  depth++;
}

static void XMLCALL end(void *data, const XML_Char *el) { depth--; }

int main() {
  XML_Parser p = XML_ParserCreate(NULL);
  if (!p) {
    fprintf(stderr, "Couldn't allocate memory for parser\n");
    exit(-1);
  }

  XML_SetElementHandler(p, start, end);

  for (;;) {
    int len = (int)fread(Buff, 1, BUFFSIZE, stdin);
    if (ferror(stdin)) {
      fprintf(stderr, "Read error\n");
      exit(-1);
    }
    int done = feof(stdin);

    if (XML_Parse(p, Buff, len, done) == XML_STATUS_ERROR) {
      fprintf(stderr, "Parse error at line %ld:\n%s\n",
              XML_GetCurrentLineNumber(p),
              XML_ErrorString(XML_GetErrorCode(p)));
      exit(-1);
    }

    if (done)
      break;
  }
  XML_ParserFree(p);
  return 0;
}
