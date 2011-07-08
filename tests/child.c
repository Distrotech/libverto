/*
 * Copyright 2011 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <sys/types.h>
#include <sys/wait.h>

#include <verto-module.h>
#include "test.h"

#define EXITCODE 17

static int exitstatus;

void
exit_cb(verto_ev_ctx *ctx, verto_ev *ev)
{
    if (WEXITSTATUS(exitstatus) != EXITCODE) {
        printf("ERROR: Child event never fired!\n");
        retval = 1;
    }

    verto_break(ctx);
}

void
cb(verto_ev_ctx *ctx, verto_ev *ev)
{
    exitstatus = verto_get_proc_status(ev);
}

int
do_test(verto_ev_ctx *ctx)
{
    pid_t pid;

    exitstatus = 0;

    pid = fork();
    if (pid < 0)
        return 1;
    else if (pid == 0) {
        usleep(50000); /* 0.05 seconds */
        exit(EXITCODE);
    }

    verto_add_timeout(ctx, VERTO_EV_FLAG_NONE, exit_cb, NULL, 100);

    /* Persist makes no sense for children events */
    assert(!verto_add_child(ctx, VERTO_EV_FLAG_PERSIST, cb, NULL, pid));

    if (!verto_add_child(ctx, VERTO_EV_FLAG_NONE, cb, NULL, pid)) {
        printf("WARNING: Child not supported!\n");
        usleep(100000);
        waitpid(pid, &exitstatus, 0);
    }

    return 0;
}
