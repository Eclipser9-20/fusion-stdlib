# Fusion stdlib

Standard-library modules for the Fusion language, hosted for `fuse`. Two kinds:

- **compiled** — a prebuilt shared library Fusion binds dynamically at load.
- **source** — a `.fsn` module Fusion compiles/merges into the program (used by
  `fuse --low-level` for freestanding builds; needs no host runtime).

## Layout

```
<platform>/lib<module>.dylib      # compiled module   (e.g. darwin-arm64/libfilesystem.dylib)
src/<module>.c                    # compiled-module source
src/<module>.fsn                  # source module     (e.g. src/lowlevel.fsn)
```

Resolution pulls from a base URL — `FUSION_STDLIB_URL`, a `~/.Fusion/stdlib.url`
file, or the built-in default — as `<base>/<platform>/lib<module>.dylib`
(compiled) or `<base>/src/<module>.fsn` (source).

```sh
fuse fetch filesystem        # compiled: -> shared modules dir
import-all lowlevel          # source:   pulled + merged at --low-level build
```

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

### lowlevel  (source)

The bare-metal floor: raw MMIO and a freestanding console with no OS beneath it.
Requires `@enable unsafe-memory` (it declares this itself). Provides byte-wise
MMIO (`peekB`/`pokeB`), a UART console (`putc`/`puts`/`consoleBase`), and `halt`.
Used by freestanding `fuse --low-level` builds; `print(...)` is retargeted to its
UART writer.

### memory  (source)

Typed wrappers over raw memory (`@enable unsafe-memory`):
`alloc`, `readWord`/`writeWord`, `readByte`/`writeByte`, `fill`, `copy`.
`import-all memory`.

### random  (compiled)

Pseudorandom helpers.

### chains  (compiled)

Windowing / drawing (`chains::clear`, `fillRect`, `line`, …). From-scratch —
links only system libraries, no third-party dependencies.

### system  (compiled)

Run shell commands.

| function | returns | description |
|----------|---------|-------------|
| `call(cmd)` | int | run `cmd` through the shell; returns its exit code (0 = success, -1 = couldn't start) |

```fusion
import system
OnStart() {
    system::call("clear")
    var code = system::call("ls -la")     # exit code back
    return 0
}
```

### sdl3  (compiled, third-party)

SDL (Simple DirectMedia Layer) itself, redistributed so you can `import sdl3` and
call SDL from Fusion. zlib-licensed — see [THIRD-PARTY.md](THIRD-PARTY.md).

## License

The modules in this repository are public domain / unlicensed — use freely.

**Exception:** the `sdl3` module (`darwin-arm64/libsdl3.dylib`) is SDL itself
(© 1997-2025 Sam Lantinga), redistributed under the **zlib license** — see
[THIRD-PARTY.md](THIRD-PARTY.md). It retains its own license and is not part of
the public-domain dedication.
