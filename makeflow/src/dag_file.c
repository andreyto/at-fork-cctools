/*
Copyright (C) 2014- The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file COPYING for details.
*/

#include "dag_file.h"

#include "xxmalloc.h"
#include "list.h"

#include <stdlib.h>

struct dag_file * dag_file_create( const char *filename )
{
	struct dag_file *f = malloc(sizeof(*f));
	f->filename = xxstrdup(filename);
	f->needed_by = list_create();
	f->created_by = 0;
	f->ref_count = 0;
	return f;
}

int dag_file_is_source( const struct dag_file *f )
{
	if(f->created_by)
		return 0;
	else
		return 1;
}

int dag_file_is_sink( const struct dag_file *f )
{
	if(list_size(f->needed_by) > 0)
		return 0;
	else
		return 1;
}

/* vim: set noexpandtab tabstop=4: */
