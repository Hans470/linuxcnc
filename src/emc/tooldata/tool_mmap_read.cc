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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "tooldata.hh"

static void usage(char*prog) {
    fprintf(stderr,"Usage: %s [nbegin [nend]]\n",prog);
    fprintf(stderr,"       Require:    0 <= nbegin <= %d\n",CANON_POCKETS_MAX -1);
    fprintf(stderr,"       Require: nend <= nbegin\n");
    exit(1);
} // usage()

int main (int argc,char**argv) {
    int idx_begin =  0; //default
    int idx_end   = 10; //default
    int idx, idx_last;
    struct CANON_TOOL_TABLE tdata;

    if (argc >1 && index(argv[1],'-')) {usage(argv[0]);}

    if (argc>=2) {idx_begin = atoi(argv[1]);idx_end=idx_begin;}
    if (argc>=3) {idx_end   = atoi(argv[2]);}

    tool_mmap_user();
    idx_last = tooldata_last_index_get();
    if (idx_last < 0) {
        fprintf(stderr,"%s: tool_mmap not initialized\n",argv[0]);
        exit(1);
    }
    if (   (idx_begin <  0)
        || (idx_end   <  idx_begin)
        || (idx_end   >  CANON_POCKETS_MAX-1)
       ) usage(argv[0]);

    if (idx_end   > idx_last) {idx_end   = idx_last;}
    if (idx_begin > idx_last) {idx_begin = idx_last;}

    fprintf(stdout,"%s pockets: %d-->%d lastpocket=%d (maxallowed=%d)\n"
           ,argv[0]
           ,idx_begin,idx_end,idx_last,CANON_POCKETS_MAX-1);
    // u,v,w offsets not printed
    fprintf(stdout,"%5s %5s %6s %6s %6s %6s %6s %6s %6s %6s %6s\n"
                  ,"Index"
                  ,"Last"
                  ,"toolno"
                  ,"pocket"
                  ,"diam"
                  ,"x-off"
                  ,"y-off"
                  ,"z-off"
                  ,"f-ang"
                  ,"b-ang"
                  ,"orient"
                  );
    for (idx=idx_begin; idx<=idx_end; idx++) {
        tdata = tooldata_get(idx);
        fprintf(stdout,"%5d %5d %6d %6d %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6d\n"
               ,idx
               ,tooldata_last_index_get()
               ,tdata.toolno
               ,tdata.pocketno
               ,tdata.diameter
               ,tdata.offset.tran.x
               ,tdata.offset.tran.y
               ,tdata.offset.tran.z
               ,tdata.frontangle
               ,tdata.backangle
               ,tdata.orientation
               );
   }
} // main()
