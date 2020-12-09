/*
** Copyright: 2020
** Author:    Dewey Garrett <dgarrett@panix.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "tooldata.hh"

static CANON_TOOL_TABLE *the_table;
static int *the_last_idx;

int tool_nml_register(CANON_TOOL_TABLE *tblptr,int *last_idx_ptr)
{
    if (!tblptr) {
        fprintf(stderr,"%5d!!!PROBLEM tool_nml_register()\n",getpid());
        return -1;
    }
    the_table     = tblptr;
    the_last_idx = last_idx_ptr; // not used by sai
    return 0;
} //tool_nml_register

void tooldata_last_index_set(int idx)  //force last_index
{
    if (the_last_idx) {
        *the_last_idx = idx;
    }
    return;
} //tooldata_last_index_set()

int tooldata_last_index_get(void)
{
    if (the_last_idx) {
        return *the_last_idx;
    }
    return CANON_POCKETS_MAX -1;
} // tooldata_last_index_get()


int tooldata_put(CANON_TOOL_TABLE from,int idx)
{
    if (!the_table) {
        fprintf(stderr,"%5d!!!tool_nml_put() NOT INITIALIZED\n",getpid());
    }
    if (idx < 0 || idx >= CANON_POCKETS_MAX) {
        fprintf(stderr,"!!!%5d PROBLEM: tooldata_put(): bad idx=%d, maxallowed=%d\n",
                getpid(),idx,CANON_POCKETS_MAX-1);
        idx = 0;
        fprintf(stderr,"!!!continuing using idx=%d\n",idx);
    }
    *(the_table + idx) = from;

    return 0;
} // tooldata_put()

struct CANON_TOOL_TABLE tooldata_get(int idx)
{
    if (idx < 0 || idx >= CANON_POCKETS_MAX) {
        fprintf(stderr,"!!!%5d PROBLEM tooldata_get(): idx=%d, maxallowed=%d\n",
                getpid(),idx,CANON_POCKETS_MAX-1);
        idx = 0;
        fprintf(stderr,"!!!continuing using idx=%d\n",idx);
    }

    if (!the_table) {
        fprintf(stderr,"!!!%5d PROBLEM tooldata_get(): idx=%d the_table=%p\n",
                getpid(),idx,the_table);
        return (*(struct CANON_TOOL_TABLE*)(NULL));
    }

    return  *(struct CANON_TOOL_TABLE*)(the_table + idx);
} // tooldata_get()

int tooldata_find_index_for_tool(int toolno) {
    int foundidx = -1;
    for(int idx = 0;idx < CANON_POCKETS_MAX;idx++){
        if ((the_table+idx)->toolno == toolno) {
            foundidx = idx;
        }
    }
    return foundidx;
} //tooldata_find_index_for_tool()
