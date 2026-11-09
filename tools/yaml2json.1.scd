yaml2json(1)	"redpesk OS"

# NAME

yaml2json - Convert YAML file to JSON file

# SYNOPSIS

*yaml2json* [_option_] [file]

# DESCRIPTION

The program *yaml2json* converts YAML file to their JSON representation.
The result is written to the standard output.

When no file is given, *yaml2json* converts its standard input.

Note that because not all YAML feature are convertible to JSON,
*yaml2json* does its best but can't convert anything.
Its converts well string, dictionnaries, lists, numbers.


# OPTIONS

*-C, --color*
	Colorizes the output (defaut when output to console).

*-c, --no-color*
	Doesn't colorize output (default when output to stream).


# AUTHOR

José Bollo <jose.bollo@iot.bzh>

