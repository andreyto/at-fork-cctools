/*
 * Copyright (C) 2013- The University of Notre Dame
 * This software is distributed under the GNU General Public License.
 * See the file COPYING for details.
 */

#ifndef CHIRP_SQLITE_H
#define CHIRP_SQLITE_H

#include "buffer.h"
#include "catch.h"
#include "json.h"
#include "json_aux.h"

#include <sqlite3.h>

#include <errno.h>
#include <string.h>

#define CHIRP_SQLITE_TIMEOUT (5000)

#define sqlcatchexec(db,sql) \
do {\
	sqlite3 *_db = (db);\
	char *errmsg;\
	rc = sqlite3_exec(_db, (sql), NULL, NULL, &errmsg);\
	if (rc) {\
		if (rc == SQLITE_BUSY) {\
			rc = EAGAIN;\
		} else {\
			debug(D_DEBUG, "[%s:%d] sqlite3 error: %d `%s': %s", __FILE__, __LINE__, rc, sqlite3_errstr(rc), errmsg);\
			if (rc == SQLITE_CONSTRAINT) {\
				rc = EINVAL;\
			} else {\
				rc = EIO;\
			}\
		}\
		sqlite3_free(errmsg);\
		goto out;\
	}\
} while (0)

#define sqlcatch(S) \
do {\
	rc = S;\
	if (rc) {\
		if (rc == SQLITE_BUSY) {\
			rc = EAGAIN;\
		} else {\
			debug(D_DEBUG, "[%s:%d] sqlite3 error: %d `%s': %s", __FILE__, __LINE__, rc, sqlite3_errstr(rc), sqlite3_errmsg(db));\
			if (rc == SQLITE_CONSTRAINT) {\
				rc = EINVAL;\
			} else {\
				rc = EIO;\
			}\
		}\
		sqlite3_finalize(stmt);\
		stmt = NULL;\
		goto out;\
	}\
} while (0)

#define sqlcatchcode(S, code) \
do {\
	rc = S;\
	if (rc != code) {\
		if (rc == SQLITE_BUSY) {\
			rc = EAGAIN;\
		} else {\
			debug(D_DEBUG, "[%s:%d] sqlite3 error: %d `%s': %s", __FILE__, __LINE__, rc, sqlite3_errstr(rc), sqlite3_errmsg(db));\
			if (rc == SQLITE_CONSTRAINT) {\
				rc = EINVAL;\
			} else {\
				rc = EIO;\
			}\
		}\
		sqlite3_finalize(stmt);\
		stmt = NULL;\
		goto out;\
	}\
} while (0)

/* This macro is part of the prologue for any procedure that begins/ends a
 * transaction. We cannot simply put the ROLLBACK conflict resolution
 * constraint in relevant SQL write operations. The reason for this is because
 * not all errors are in SQLite. For example, the code may do a waitpid which
 * fails, which requires the entire operation to fail. This macro does a
 * rollback of the entire transaction on error, so it handles that case.
 */
#define sqlend(db) \
do {\
	sqlite3 *_db = (db);\
	if (_db && rc) {\
		char *errmsg;\
		int erc = sqlite3_exec(_db, "ROLLBACK TRANSACTION;", NULL, NULL, &errmsg);\
		if (erc) {\
			if (erc == SQLITE_ERROR /* cannot rollback because no transaction is active */) {\
				; /* do nothing */\
			} else {\
				debug(D_DEBUG, "[%s:%d] sqlite3 error: %d `%s': %s", __FILE__, __LINE__, erc, sqlite3_errstr(erc), errmsg);\
			}\
			sqlite3_free(errmsg);\
		}\
	}\
} while (0)

#define IMMUTABLE(T) \
		"CREATE TRIGGER " T "ImmutableI BEFORE INSERT ON " T " FOR EACH ROW" \
		"    BEGIN" \
		"        SELECT RAISE(ABORT, 'cannot insert rows of immutable table');" \
		"    END;" \
		"CREATE TRIGGER " T "ImmutableU BEFORE UPDATE ON " T " FOR EACH ROW" \
		"    BEGIN" \
		"        SELECT RAISE(ABORT, 'cannot update rows of immutable table');" \
		"    END;" \
		"CREATE TRIGGER " T "ImmutableD BEFORE DELETE ON " T " FOR EACH ROW" \
		"    BEGIN" \
		"        SELECT RAISE(ABORT, 'cannot delete rows of immutable table');" \
		"    END;"

#define chirp_sqlite3_column_jsonify(stmt, n, B) \
do {\
	switch (sqlite3_column_type(stmt, n)) {\
		case SQLITE_NULL:\
			CATCHUNIX(buffer_putliteral(B, "null"));\
			break;\
		case SQLITE_INTEGER:\
			CATCHUNIX(buffer_putfstring(B, "%" PRId64, (int64_t) sqlite3_column_int64(stmt, n)));\
			break;\
		case SQLITE_FLOAT:\
			CATCHUNIX(buffer_putfstring(B, "%.*e", DBL_DIG, sqlite3_column_double(stmt, n)));\
			break;\
		case SQLITE_TEXT: {\
			CATCHUNIX(buffer_putliteral(B, "\""));\
			CATCHUNIX(jsonA_escapestring(B, (const char *)sqlite3_column_text(stmt, n)));\
			CATCHUNIX(buffer_putliteral(B, "\""));\
			break;\
		}\
		case SQLITE_BLOB:\
		default:\
			assert(0); /* we don't handle this */\
	}\
} while (0)

#define chirp_sqlite3_row_jsonify(stmt, B) \
do {\
	int _i;\
	int _first = 1;\
	sqlite3_stmt *_stmt = (stmt);\
	buffer_t *_B = (B);\
	CATCHUNIX(buffer_putliteral(_B, "{"));\
	for (_i = 0; _i < sqlite3_column_count(_stmt); _i++) {\
		if (!_first)\
			CATCHUNIX(buffer_putliteral(_B, ","));\
		_first = 0;\
		CATCHUNIX(buffer_putfstring(_B, "\"%s\":", sqlite3_column_name(_stmt, _i)));\
		chirp_sqlite3_column_jsonify(_stmt, _i, _B);\
	}\
	CATCHUNIX(buffer_putliteral(_B, "}"));\
} while (0)

#endif

/* vim: set noexpandtab tabstop=4: */
