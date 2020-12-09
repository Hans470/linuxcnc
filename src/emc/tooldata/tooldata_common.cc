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
#include <string.h>
#include <rtapi_string.h>
#include "tooldata.hh"

struct CANON_TOOL_TABLE tooldata_entry_init()
{
    struct CANON_TOOL_TABLE tdata;
    tdata.toolno = -1;
    tdata.pocketno = -1;
    ZERO_EMC_POSE(tdata.offset);
    tdata.diameter = 0.0;
    tdata.frontangle = 0.0;
    tdata.backangle = 0.0;
    tdata.orientation = 0;

    return tdata;
} // tooldata_entry_init()

static bool is_random_toolchanger = 0;
static int  nonrandom_idx = 0; // 'fakepocket' counter
static bool initialized = 0;

void tooldata_add_init(int nonrandom_start_idx,bool random_toolchanger)
{
    is_random_toolchanger = random_toolchanger ? 1 : 0;
    nonrandom_idx = nonrandom_start_idx;
    initialized   = 1;
    return;
} // tooldata_add_init()

int tooldata_add_line(CANON_TOOL_TABLE *result,
                      const char *input_line,
                      char *ttcomments[])
{
    char work_line[CANON_TOOL_ENTRY_LEN];
    const char *token;
    char *buff, *comment;
    int toolno,orientation,valid=1;
    EmcPose offset; //tlo
    double diameter, frontangle, backangle;
    int idx = 0;
    int realpocket = 0;

    if (!initialized) {
        fprintf(stderr,"!!! PROBLEM no init %s\n",__FILE__);
        return -1;
    }
    strcpy(work_line, input_line);

    CANON_TOOL_TABLE empty = tooldata_entry_init();
    toolno      = empty.toolno;
    diameter    = empty.diameter;
    frontangle  = empty.frontangle;
    backangle   = empty.backangle;
    orientation = empty.orientation;
    offset      = empty.offset;

    buff = strtok(work_line, ";");
    if (strlen(buff) <=1) {
        //fprintf(stderr,"skip blankline %s\n",__FILE__);
        return 0;
    }
    comment = strtok(NULL, "\n");

    token = strtok(buff, " ");
    while (token != NULL) {
        switch (toupper(token[0])) {
        case 'T':
            if (sscanf(&token[1], "%d", &toolno) != 1)
                valid = 0;
            break;
        case 'P':
            if (sscanf(&token[1], "%d", &idx) != 1) {
                valid = 0;
                break;
            }
            realpocket = idx;  //random toolchanger
            if (!is_random_toolchanger) {
                (nonrandom_idx)++;
                if (nonrandom_idx >= CANON_POCKETS_MAX) {
                    printf("too many tools. skipping tool %d\n", toolno);
                    valid = 0;
                    break;
                }
                idx = nonrandom_idx; // nonrandom toolchanger
            }
            if (idx < 0 || idx >= CANON_POCKETS_MAX) {
                printf("max pocket number is %d. skipping tool %d\n",
                       CANON_POCKETS_MAX - 1, toolno);
                valid = 0;
                break;
            }
            break;
        case 'D':
            if (sscanf(&token[1], "%lf", &diameter) != 1)
                valid = 0;
            break;
        case 'X':
            if (sscanf(&token[1], "%lf", &offset.tran.x) != 1)
                valid = 0;
            break;
        case 'Y':
            if (sscanf(&token[1], "%lf", &offset.tran.y) != 1)
                valid = 0;
            break;
        case 'Z':
            if (sscanf(&token[1], "%lf", &offset.tran.z) != 1)
                valid = 0;
            break;
        case 'A':
            if (sscanf(&token[1], "%lf", &offset.a) != 1)
                valid = 0;
            break;
        case 'B':
            if (sscanf(&token[1], "%lf", &offset.b) != 1)
                valid = 0;
            break;
        case 'C':
            if (sscanf(&token[1], "%lf", &offset.c) != 1)
                valid = 0;
            break;
        case 'U':
            if (sscanf(&token[1], "%lf", &offset.u) != 1)
                valid = 0;
            break;
        case 'V':
            if (sscanf(&token[1], "%lf", &offset.v) != 1)
                valid = 0;
            break;
        case 'W':
            if (sscanf(&token[1], "%lf", &offset.w) != 1)
                valid = 0;
            break;
        case 'I':
            if (sscanf(&token[1], "%lf", &frontangle) != 1)
                valid = 0;
            break;
        case 'J':
            if (sscanf(&token[1], "%lf", &backangle) != 1)
                valid = 0;
            break;
        case 'Q':
            if (sscanf(&token[1], "%d", &orientation) != 1)
                valid = 0;
            break;
        default:
            if (strncmp(token, "\n", 1) != 0)
                valid = 0;
            break;
        }
        token = strtok(NULL, " ");
    }

    if (valid) {
        result->toolno = toolno;
        result->pocketno = realpocket;
        result->offset = offset;
        result->diameter = diameter;
        result->frontangle = frontangle;
        result->backangle = backangle;
        result->orientation = orientation;
        tooldata_put(*result,idx);
        if (ttcomments && comment) {
             strcpy(ttcomments[idx], comment);
        }
    } else {
         return -1;
    }
    return 0;
} // tooldata_add_line()

void tooldata_save_line(FILE* fp,int idx,int random_toolchanger,char *ttcomments[])
{
    CANON_TOOL_TABLE tdata = tooldata_get(idx);
    if (tdata.toolno != -1) {
        fprintf(fp, "T%d P%d", tdata.toolno,
                random_toolchanger? idx : tdata.pocketno);
        if (tdata.diameter)      fprintf(fp, " D%f",  tdata.diameter);
        if (tdata.offset.tran.x) fprintf(fp, " X%+f", tdata.offset.tran.x);
        if (tdata.offset.tran.y) fprintf(fp, " Y%+f", tdata.offset.tran.y);
        if (tdata.offset.tran.z) fprintf(fp, " Z%+f", tdata.offset.tran.z);
        if (tdata.offset.a)      fprintf(fp, " A%+f", tdata.offset.a);
        if (tdata.offset.b)      fprintf(fp, " B%+f", tdata.offset.b);
        if (tdata.offset.c)      fprintf(fp, " C%+f", tdata.offset.c);
        if (tdata.offset.u)      fprintf(fp, " U%+f", tdata.offset.u);
        if (tdata.offset.v)      fprintf(fp, " V%+f", tdata.offset.v);
        if (tdata.offset.w)      fprintf(fp, " W%+f", tdata.offset.w);
        if (tdata.frontangle)    fprintf(fp, " I%+f", tdata.frontangle);
        if (tdata.backangle)     fprintf(fp, " J%+f", tdata.backangle);
        if (tdata.orientation)   fprintf(fp, " Q%d",  tdata.orientation);
        fprintf(fp, " ;%s\n", ttcomments[idx]);
    }
} // tooldata_save_line()

int loadToolTable(const char *filename,
#ifdef TOOL_MMAP //{
                 // toolTable[] param not used for mmap
#else //}{
                 CANON_TOOL_TABLE toolTable[],
#endif //}
                 char *ttcomments[],
                 int random_toolchanger)
{
    int  idx;
    FILE *fp;
    char input_line[CANON_TOOL_ENTRY_LEN];
    char orig_line[CANON_TOOL_ENTRY_LEN];

    if(!filename) return -1;

    // open tool table file
    if (NULL == (fp = fopen(filename, "r"))) {
        // can't open file
        return -1;
    }
    // clear out tool table
    // (Set vars to indicate no tool in pocket):
    for (idx = random_toolchanger? 0: 1; idx < CANON_POCKETS_MAX; idx++) {
        CANON_TOOL_TABLE tdata = tooldata_entry_init();
        tooldata_put(tdata,idx);
        if(ttcomments) ttcomments[idx][0] = '\0';
    }

    // after initializing all available pockets:
    tooldata_last_index_set(0);
    // subsequent read from filename will update the last_index
    // which becomes available by tooldata_last_index_get()

    const int nonrandom_start_idx = 0; // when reading file start at 0
    tooldata_add_init(nonrandom_start_idx,random_toolchanger);

    while (!feof(fp)) {
        // for nonrandom machines, just read the tools into pockets 1..n
        // no matter their tool numbers.  NB leave the spindle pocket 0
        // unchanged/empty.
        if (NULL == fgets(input_line, CANON_TOOL_ENTRY_LEN, fp)) {
            break;
        }
        CANON_TOOL_TABLE tdata;
        strcpy(orig_line, input_line);

        // parse and store one line from tool table file
        if (tooldata_add_line(&tdata, //result
                                input_line,
                                ttcomments)) {
            printf("File: %s Unrecognized line skipped:\n    %s",filename, orig_line);
            continue;
        }

        CANON_TOOL_TABLE zdata = tooldata_get(0);
        if (!random_toolchanger && zdata.toolno == tdata.toolno) {
            zdata = tdata;
            tooldata_put(zdata,0);
        }
    } // while

    // close the file
    fclose(fp);

    return 0;
} // loadToolTable()

int saveToolTable(const char *filename,
#ifdef TOOL_MMAP //{
    // toolTable[] param not used for mmap
#else //}{
        CANON_TOOL_TABLE toolTable[],
#endif //}
        char *ttcomments[CANON_POCKETS_MAX],
        int random_toolchanger)
{
    int idx;
    FILE *fp;
    int start_idx;

    // check filename
    if (filename[0] == 0) {
        fprintf(stderr,"UNEXPECTED %s\n",__FILE__);
    }

    // open tool table file
    if (NULL == (fp = fopen(filename, "w"))) {
        // can't open file
        return -1;
    }

    start_idx = random_toolchanger ? 0 : 1;
    for (idx = start_idx; idx < CANON_POCKETS_MAX; idx++) {
        tooldata_save_line(fp,idx,random_toolchanger,ttcomments);
    }
    fclose(fp);
    return 0;
} //saveToolTable()
