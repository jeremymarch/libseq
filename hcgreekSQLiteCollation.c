/*
** 2017-04-16
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file implements a run-time loadable extension to SQLite that
** registers a sqlite3_collation_needed() callback to register a fake
** collating function for any unknown collating sequence.  The fake
** collating function works like BINARY.
**
** This extension can be used to load schemas that contain one or more
** unknown collating sequences.

https://stackoverflow.com/questions/30898113/how-to-compile-an-extension-into-sqlite
https://www.sqlite.org/loadext.html
//1. compile extension
gcc -std=c99 -pedantic -fPIC -shared hcgreekSQLiteCollation.c accent.c utilities.c -o hcgreek.dylib
or: gcc -std=c99 -pedantic -fPIC -dynamiclib hcgreekSQLiteCollation.c accent.c utilities.c -o hcgreek.dylib
 
//2. comile sqlite
gcc -DHAVE_READLINE -omysqlite shell.c sqlite3.c -lpthread -ldl -lreadline
 
//3. launch
./mysqlite hcdatadb1-5.sqlite

//4. load extension and query
.load hcgreek
select lemma from hqvocab order by lemma collate hcgreek limit 20;
 
*/
#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1
#include <string.h>
#include <stdio.h>
#include <accent.h>
#include <assert.h>

//When using statically:
//1. define SQLITE_CORE
//2. register with sqlite3_auto_extension() to use extension on all db connections
//see: https://www.sqlite.org/loadext.html - 6. Statically Linking A Run-Time Loadable Extension

static void uchex(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    assert(argc == 1);
    if (sqlite3_value_type(argv[0]) == SQLITE_TEXT)
    {
        unsigned const char *a = sqlite3_value_text(argv[0]);
        int bufferLen = 1024;
        char buffer[bufferLen];
        hcucHex(a, bufferLen, buffer);

        sqlite3_result_text(ctx, buffer, -1, SQLITE_TRANSIENT);
    }
    else
    {
      sqlite3_result_null(ctx);
    }
}

static void containsPUA(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    assert(argc == 1);
    const unsigned char *a = sqlite3_value_text(argv[0]);
    int containsPUA = hccontainsPUA(a);
    sqlite3_result_int(ctx, containsPUA);
}

static int hcgreekFunc(
  void *NotUsed,
  int nKey1, const void *pKey1,
  int nKey2, const void *pKey2
){
    return compareSort(nKey1, pKey1, nKey2, pKey2);
}

static void hcgreekNeeded(
  void *NotUsed,
  sqlite3 *db,
  int eTextRep,
  const char *zCollName
){
  sqlite3_create_collation(db, "hcgreek", eTextRep, 0, hcgreekFunc);
}

#ifdef _WIN32
__declspec(dllexport)
#endif
int sqlite3_hcgreek_init(
  sqlite3 *db,
  char **pzErrMsg,
  const sqlite3_api_routines *pApi
){
  int rc = SQLITE_OK;
  SQLITE_EXTENSION_INIT2(pApi);
  rc = sqlite3_collation_needed(db, 0, hcgreekNeeded);
    
    sqlite3_create_function(db, "isPUA", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, &containsPUA, NULL, NULL);
    sqlite3_create_function(db, "HCHEX", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, &uchex, NULL, NULL);
    
  return rc;
}
