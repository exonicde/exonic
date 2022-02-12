# exonic

The program provides a webview, which loads a content from a url or a file. Additionally the webview provides some js-extensions to allow the host system access (see [exonicapi.js](./blob/main/exonicapi.js)).

## Prerequirements

* [Qt 5.15](https://doc.qt.io/qt-5.15/)
* [gcc](https://gcc.gnu.org/) or [clang](https://clang.llvm.org/)

## Building

```sh
qmake
make -j$(nproc)
```

## License

[LICENSE](./LICENSE) GPLv3
