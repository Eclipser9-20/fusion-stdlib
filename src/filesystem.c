// Fusion stdlib module: filesystem — a real compiled lib (libfilesystem.dylib).
//
// Uses the Fusion string/blob ABI: a pointer to [u64 little-endian length][bytes...].
// Exports are bound dynamically by the loader (dyld) — no headers, no static ld.
//
// Every path argument is a Fusion blob; every blob returned is heap-allocated in
// the same [len][bytes] layout, so it round-trips through Fusion strings/buffers.
// Functions returning `long long` follow the convention: 0 = success, non-zero =
// failure, and query functions return 0/1 (or -1 where a size/none is meaningful).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

typedef struct { unsigned long long len; unsigned char bytes[]; } blob;

// ---- blob helpers -----------------------------------------------------------
static char* cstr_of(const blob* b) {           // NUL-terminated C string from a blob
    char* s = (char*)malloc(b->len + 1);
    if (!s) return 0;
    memcpy(s, b->bytes, b->len); s[b->len] = 0;
    return s;
}
static blob* blob_new(unsigned long long n) {   // a fresh blob with n payload bytes
    blob* b = (blob*)malloc(8 + n);
    if (b) b->len = n;
    return b;
}
static blob* blob_of(const char* data, unsigned long long n) {
    blob* b = blob_new(n);
    if (b && n) memcpy(b->bytes, data, n);
    return b;
}

// ---- read / write (original surface, preserved) -----------------------------
blob* fs_readFile(const blob* path) {
    char* p = cstr_of(path); if (!p) return 0;
    FILE* f = fopen(p, "rb"); free(p);
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 0; }
    long n = ftell(f);
    if (n < 0 || fseek(f, 0, SEEK_SET) != 0) { fclose(f); return 0; }
    blob* out = blob_new((unsigned long long)n);
    if (!out) { fclose(f); return 0; }
    if (n > 0 && fread(out->bytes, 1, (size_t)n, f) != (size_t)n) { fclose(f); free(out); return 0; }
    fclose(f); return out;
}
blob* fs_readBytes(const blob* path) { return fs_readFile(path); }

static long long write_mode(const blob* path, const blob* data, const char* mode) {
    char* p = cstr_of(path); if (!p) return 1;
    FILE* f = fopen(p, mode); free(p);
    if (!f) return 1;
    if (data->len && fwrite(data->bytes, 1, data->len, f) != data->len) { fclose(f); return 1; }
    fclose(f); return 0;
}
long long fs_writeFile (const blob* path, const blob* data) { return write_mode(path, data, "wb"); }
long long fs_writeBytes(const blob* path, const blob* data) { return write_mode(path, data, "wb"); }
long long fs_append    (const blob* path, const blob* data) { return write_mode(path, data, "ab"); }

// ---- queries ----------------------------------------------------------------
long long fs_exists(const blob* path) { char* p = cstr_of(path); if (!p) return 0; long long r = access(p, F_OK) == 0; free(p); return r; }

static int stat_of(const blob* path, struct stat* st) {
    char* p = cstr_of(path); if (!p) return -1;
    int r = stat(p, st); free(p); return r;
}
long long fs_isFile(const blob* path) { struct stat st; return stat_of(path, &st) == 0 && S_ISREG(st.st_mode); }
long long fs_isDir (const blob* path) { struct stat st; return stat_of(path, &st) == 0 && S_ISDIR(st.st_mode); }
long long fs_size  (const blob* path) { struct stat st; return stat_of(path, &st) == 0 ? (long long)st.st_size : -1; }

// ---- mutations --------------------------------------------------------------
long long fs_remove(const blob* path) { char* p = cstr_of(path); if (!p) return 1; long long r = remove(p) != 0; free(p); return r; }

// recursive mkdir (like `mkdir -p`); 0 on success (or already-exists)
long long fs_mkdir(const blob* path) {
    char* p = cstr_of(path); if (!p) return 1;
    long long rc = 0;
    for (char* s = p + (p[0] == '/' ? 1 : 0); *s; s++) {
        if (*s == '/') {
            *s = 0;
            if (mkdir(p, 0755) != 0 && errno != EEXIST) { rc = 1; break; }
            *s = '/';
        }
    }
    if (rc == 0 && mkdir(p, 0755) != 0 && errno != EEXIST) rc = 1;
    free(p); return rc;
}
long long fs_rmdir(const blob* path) { char* p = cstr_of(path); if (!p) return 1; long long r = rmdir(p) != 0; free(p); return r; }

long long fs_rename(const blob* from, const blob* to) {
    char* a = cstr_of(from); char* b = cstr_of(to);
    long long r = (a && b && rename(a, b) == 0) ? 0 : 1;
    free(a); free(b); return r;
}
long long fs_copy(const blob* from, const blob* to) {
    char* a = cstr_of(from); char* b = cstr_of(to);
    long long rc = 1;
    if (a && b) {
        FILE* in = fopen(a, "rb");
        if (in) {
            FILE* out = fopen(b, "wb");
            if (out) {
                char buf[65536]; size_t n; rc = 0;
                while ((n = fread(buf, 1, sizeof buf, in)) > 0)
                    if (fwrite(buf, 1, n, out) != n) { rc = 1; break; }
                fclose(out);
            }
            fclose(in);
        }
    }
    free(a); free(b); return rc;
}

// ---- directory listing ------------------------------------------------------
// Returns a blob of newline-separated entry names (excluding "." and ".."),
// no trailing newline. Empty blob for an empty (or unreadable) directory.
blob* fs_list(const blob* path) {
    char* p = cstr_of(path); if (!p) return blob_of("", 0);
    DIR* d = opendir(p); free(p);
    if (!d) return blob_of("", 0);
    char* acc = 0; size_t len = 0, cap = 0;
    struct dirent* e;
    while ((e = readdir(d))) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
        size_t nl = strlen(e->d_name);
        size_t need = len + (len ? 1 : 0) + nl;
        if (need > cap) { cap = need * 2 + 64; char* n = (char*)realloc(acc, cap); if (!n) break; acc = n; }
        if (len) acc[len++] = '\n';
        memcpy(acc + len, e->d_name, nl); len += nl;
    }
    closedir(d);
    blob* out = blob_of(acc ? acc : "", len);
    free(acc);
    return out;
}

// ---- working directory ------------------------------------------------------
blob* fs_cwd(void) {
    char buf[4096];
    if (getcwd(buf, sizeof buf)) return blob_of(buf, strlen(buf));
    return blob_of("", 0);
}
long long fs_chdir(const blob* path) { char* p = cstr_of(path); if (!p) return 1; long long r = chdir(p) != 0; free(p); return r; }
