/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "qpid/dispatch/platform.h"

#include "config.h"

#include <stdio.h>
//#include <sys/time.h>
//#include <sys/resource.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#include <qpid/dispatch/platform.h>
#include <qpid/dispatch/ctools.h>
#include <Python.h>  // just  for ssize_t

// https://stackoverflow.com/questions/735126/are-there-alternate-implementations-of-gnu-getline-interface/735472#735472
// https://stackoverflow.com/a/47067149



// http://cvsweb.netbsd.org/bsdweb.cgi/pkgsrc/pkgtools/libnbcompat/files/getdelim.c?only_with_tag=MAIN
static inline ssize_t
getdelim(char **buf, size_t *bufsiz, int delimiter, FILE *fp)
{
    char *ptr, *eptr;


    if (*buf == NULL || *bufsiz == 0) {
        *bufsiz = BUFSIZ;
        if ((*buf = malloc(*bufsiz)) == NULL)
            return -1;
    }

    for (ptr = *buf, eptr = *buf + *bufsiz;;) {
        int c = fgetc(fp);
        if (c == -1) {
            if (feof(fp)) {
                ssize_t diff = (ssize_t)(ptr - *buf);
                if (diff != 0) {
                    *ptr = '\0';
                    return diff;
                }
            }
            return -1;
        }
        *ptr++ = c;
        if (c == delimiter) {
            *ptr = '\0';
            return ptr - *buf;
        }
        if (ptr + 2 >= eptr) {
            char *nbuf;
            size_t nbufsiz = *bufsiz * 2;
            ssize_t d = ptr - *buf;
            if ((nbuf = realloc(*buf, nbufsiz)) == NULL)
                return -1;
            *buf = nbuf;
            *bufsiz = nbufsiz;
            eptr = nbuf + nbufsiz;
            ptr = nbuf + d;
        }
    }
}

static inline ssize_t
getline(char **buf, size_t *bufsiz, FILE *fp)
{
    return getdelim(buf, bufsiz, '\n', fp);
}


uintmax_t qd_platform_memory_size(void)
{
    bool found = false;
    uintmax_t rlimit = UINTMAX_MAX;

#if QD_HAVE_GETRLIMIT
    {
        // determine if this process has a hard or soft limit set for its total
        // virtual address space
        struct rlimit rl = {0};
        // note rlim_max >= rlim_cur (see man getrlimit) use smallest value
        if (getrlimit(RLIMIT_AS, &rl) == 0) {
            if (rl.rlim_cur != RLIM_INFINITY) {
                rlimit = (uintmax_t)rl.rlim_cur;
                found = true;
            } else if (rl.rlim_max != RLIM_INFINITY) {
                rlimit = (uintmax_t)rl.rlim_max;
                found = true;
            }
        }
    }
#endif // QD_HAVE_GETRLIMIT

    // although a resource limit may be set be sure it does not exceed the
    // available "fast" memory.

    // @TODO(kgiusti) this is linux-specific (see man proc)
    uintmax_t mlimit = UINTMAX_MAX;
    FILE *minfo_fp = fopen("/proc/meminfo", "r");
    if (minfo_fp) {
        size_t buflen = 0;
        char *buffer = 0;
        uintmax_t tmp;
        while (getline(&buffer, &buflen, minfo_fp) != -1) {
            if (sscanf(buffer, "MemTotal: %"SCNuMAX, &tmp) == 1) {
                mlimit = tmp * 1024;  // MemTotal is in KiB
                found = true;
                break;
            }
        }
        free(buffer);  // allocated by getline
        fclose(minfo_fp);
    }

    // and if qdrouterd is running within a container check the cgroups memory
    // controller. Hard and soft memory limits can be set.

    uintmax_t climit = UINTMAX_MAX;
    {
        uintmax_t soft = UINTMAX_MAX;
        uintmax_t hard = UINTMAX_MAX;
        bool c_set = false;

        FILE *cg_fp = fopen("/sys/fs/cgroup/memory/memory.limit_in_bytes", "r");
        if (cg_fp) {
            if (fscanf(cg_fp, "%"SCNuMAX, &hard) == 1) {
                c_set = true;
            }
            fclose(cg_fp);
        }

        cg_fp = fopen("/sys/fs/cgroup/memory/memory.soft_limit_in_bytes", "r");
        if (cg_fp) {
            if (fscanf(cg_fp, "%"SCNuMAX, &soft) == 1) {
                c_set = true;
            }
            fclose(cg_fp);
        }

        if (c_set) {
            climit = MIN(soft, hard);
            found = true;
        }
    }

    if (found) {
        uintmax_t tmp = MIN(mlimit, climit);
        return MIN(rlimit, tmp);
    }

    return 0;
}
