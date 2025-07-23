# wirecc

C++ utility functions for data encoding/decoding over the network.

## Build
```sh
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr/local -DCMAKE_BUILD_TYPE=Release ..
make
```

## Development

### Debug
```sh
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr/local -DCMAKE_BUILD_TYPE=Debug ..
make
```

### Testing (outputs Coverage)
```sh
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr/local -DCMAKE_BUILD_TYPE=Testing ..
cmake --build .
ctest --output-on-failure
```
