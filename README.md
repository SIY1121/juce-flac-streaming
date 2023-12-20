# how to build
## client

- node 18
- pnpm

```
$ cd client
$ pnpm install
$ pnpm run build
```

## plugin

- cmake
- MSVC (for windows)
- XCode (for macOS)

```
$ cd plugin
$ mkdir build && cd build
$ cmake ../
$ cmake --build .
```