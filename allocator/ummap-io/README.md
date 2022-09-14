# User-level Memory-mapped I/O

## How to compile 
The `src` folder contains the source code and all the necessary
elements. Type the following to compile both the library and a simple
example application:

```
make rebuild
```

If the compilation is successful, you will now observe inside
`ummap-io/` a new `build/` folder that contains the following
structure:

- `ummap-io/build/bin`: The executable of the source code example is located here.
- `ummap-io/build/lib`: Static library for **uMMAP-IO**, named `libummapio.a`. It can be utilized to compile with your code by including `-lummapio`. Optionally, some compilers might require `-pthread -lrt` as well.
- `ummap-io/build/inc`: A header file, named `ummap.h`, is provided with the definition of the API.
- `ummap-io/build/...`: Other temporary folders might be created (ignore).
