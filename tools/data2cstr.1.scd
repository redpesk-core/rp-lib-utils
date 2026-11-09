data2cstr(1)	"redpesk OS"

# NAME

data2cstr - Convert any data to C string

# SYNOPSIS

*data2cstr* [file ...]

# DESCRIPTION

The program *data2cstr* converts files to their C string representation.
The result is written to the standard output.

When no file is given or when the file is *-* (dash), *data2cstr* converts
its standard input.

The output only prints the C string, there is no variable declaration
or terminating semi-colon.

# EXAMPLE

The command `echo hello | tools/data2cstr` produces

```
        /* generated from /dev/stdin */
        "hello\n"
```

# AUTHOR

José Bollo <jose.bollo@iot.bzh>

