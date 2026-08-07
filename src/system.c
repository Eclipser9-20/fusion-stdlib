// libsystem — the Fusion `system` module. Fetched into the shared modules dir,
// unlocked with `import system`. Runs a shell command line.
//
// Uses the Fusion string/blob ABI: a pointer to [u64 little-endian length][bytes...].
// The one export, `call`, is bound dynamically by the loader (dyld) — no headers,
// no static ld — so Fusion reaches it as `system::call("...")`.
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

typedef struct { unsigned long long len; unsigned char bytes[]; } blob;

// system::call("cmd") — run a command line through the shell.
// Returns its exit code (0 = success), or -1 if the command could not be started.
long call(const blob* cmd) {
    if (!cmd) return -1;
    char* c = (char*)malloc(cmd->len + 1);
    if (!c) return -1;
    memcpy(c, cmd->bytes, cmd->len);
    c[cmd->len] = 0;
    int rc = system(c);                 // libc: fork + /bin/sh -c c + wait
    free(c);
    if (rc == -1) return -1;            // couldn't fork/exec the shell
    if (WIFEXITED(rc)) return WEXITSTATUS(rc);   // normal exit -> clean 0..255 code
    if (WIFSIGNALED(rc)) return 128 + WTERMSIG(rc);   // killed by signal (shell convention)
    return rc;
}
