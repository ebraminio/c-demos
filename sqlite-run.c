// cc sqlite-run.c `pkg-config --libs --cflags sqlite3` -Wall && ./a.out :memory: 'SELECT 1;'
// https://youtu.be/ZSKLA81tBis?t=982
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

int main(int argc, char **argv) {
  sqlite3 *db;
  assert(!sqlite3_open(argv[1], &db));
  sqlite3_stmt *pStmt;
  assert(!sqlite3_prepare(db, argv[2], strlen(argv[2]), &pStmt, NULL));
  int nCol = sqlite3_column_count(pStmt);
  while (sqlite3_step(pStmt) == SQLITE_ROW) {
    printf("Row:\n");
    for (unsigned i = 0; i < nCol; ++i) {
      printf(" %s = %s\n",
        sqlite3_column_name(pStmt, i),
        sqlite3_column_text(pStmt, i)
      );
    }
  }
  sqlite3_finalize(pStmt); 
  sqlite3_close(db);
  return 0;
}
