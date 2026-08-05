# Fusion stdlib

Prebuilt standard-library modules for the Fusion language, hosted for
`fuse fetch`. Each module is a compiled shared library that Fusion imports and
binds dynamically at load — no headers, no static linking.

## Layout

```
<platform>/lib<module>.dylib      # e.g. darwin-arm64/libfilesystem.dylib
src/<module>.c                    # module source
```

`fuse fetch <module>` downloads `<platform>/lib<module>.dylib` from the base URL
in `FUSION_STDLIB_URL` and installs it into the shared modules directory.

```sh
export FUSION_STDLIB_URL="https://raw.githubusercontent.com/<owner>/<repo>/<ref>"
fuse fetch filesystem
```

`<ref>` is the branch or tag to pull from.

## Modules

### filesystem

File and directory operations, using the Fusion blob ABI
(`[u64 little-endian length][bytes...]`).

| function | returns | description |
|----------|---------|-------------|
| `fs_readFile(path)`  | blob   | whole file as a string/blob (0 on error) |
| `fs_readBytes(path)` | blob   | alias of `fs_readFile` |
| `fs_writeFile(path, data)`  | int | write; 0 = ok |
| `fs_writeBytes(path, data)` | int | write; 0 = ok |
| `fs_append(path, data)` | int | append; 0 = ok |
| `fs_exists(path)` | int | 0/1 |
| `fs_isFile(path)` | int | 0/1 |
| `fs_isDir(path)`  | int | 0/1 |
| `fs_size(path)`   | int | byte size, or -1 |
| `fs_mkdir(path)`  | int | recursive make; 0 = ok |
| `fs_rmdir(path)`  | int | remove empty dir; 0 = ok |
| `fs_remove(path)` | int | remove file; 0 = ok |
| `fs_rename(from, to)` | int | 0 = ok |
| `fs_copy(from, to)`   | int | 0 = ok |
| `fs_list(path)` | blob | newline-separated entry names |
| `fs_cwd()`      | blob | current working directory |
| `fs_chdir(path)` | int | 0 = ok |

Import with `import-all filesystem` to call functions bare (`fs_size(p)`), or
`import filesystem` and qualify them (`filesystem::fs_size(p)`).

Note: blob-returning functions currently surface to Fusion as a raw pointer
until the front end infers their return type; the integer-returning functions
are fully usable today.

## License

Public domain / unlicensed — use freely.
