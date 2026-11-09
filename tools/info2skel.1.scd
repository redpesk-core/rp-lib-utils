info2skel(1)	"redpesk OS"

# NAME

info2skel - Produce a C or C++ skeleton for a redpesk binding described by its
info JSON.

# SYNOPSIS

*info2skel* [_option_ ...] [file ...]

# DESCRIPTION

The program *info2skel* converts JSON info describing a binding to a C or C++
skeleton that can be used for implementing the binding.

When no file is given or when the file is *-* (dash), *info2skel* converts its
standard input.

When more than one file is given, *info2skel* outputs the skeleton of
all the given files.


# OPTIONS

*-v, --version*
	Prints version of *info2skel* and exits.

*-h, --help*
	Prints a small help and exits.

*-e, --extern*
	Unimplemented functions (verbs and control) are declared `extern`.

*-x, --c++*
	Prints C++ skeleton instead of C skeleton.
	This option can be repeated for increasing the level of C++ stuff used.

# INFO DESCRIPTION

The format of info files is described in the documentation of redpesk [1], [2].
Info shemas processed by *info2skel* is `afb-api-info/v2` only.

The file info can contain directives for *info2skel* in the main entry `x-afb-genskel`.
The below example shows a full featured info.

```
{
	"$shema": "afb-api-info/v2",
	"x-afb-genskel": {
		"mainctl": STRING,
		"scope": STRING,
		"prefix": STRING,
		"postfix": STRING,
		"provide-class": STRING,
		"require-class": STRING,
		"require-api": STRING,
		"private": BOOLEAN,
		"noconcurrency": BOOLEAN,
		"noexcept": BOOLEAN,
		"version": INTEGER,
		"c++": INTEGER,
		"declare": BOOLEAN,
		"name": STRING,
		"info": STRING
	},
	...
}
```

Known entries, all optional, are:

*mainctl* (STRING)
	Name of the main control callback for the binding.

*scope* (STRING)
	Should be either `static`, for in module functions, or `extern`,
	for outside of module functions.

*prefix* (STRING)
	Prefix of verb callback function names, default is `afb\_verb\_`.

*postfix* (STRING)
	Suffix of verb callback function names, default is `\_cb`.

*provide-class* (STRING)
	List of provided clases.

*require-class* (STRING)
	List of required classes.

*require-api* (STRING)
	List of required APIs.

*private* (BOOLEAN)
	If true, doesn't exports the binding structure, default is false.

*noconcurrency* (BOOLEAN)
	If true, the binding doesn't allow concurrent access, default is false.

*noexcept* (BOOLEAN, C++ only)
	If true, functions are declared with attribute `noexcept`, default is false.

*version* (INTEGER)
	The version of binding to declare, only 4 is supported.

*c++* (INTEGER)
	Level of C++: 0, no C++; 1, basic C++; 2, advanced C++.

*declare* (BOOLEAN)
	If false, doesn't declare the functions, default is true.

*name* (STRING)
	Name of the API to be declared, default is in info at path `/name`.

*info* (STRING)
	Info about the binding, default is in info at path `/info`.

# LINKS

*[1]* https://docs.redpesk.bzh/

*[2]* https://docs.redpesk.bzh/docs/en/master/developer-guides/afb-binding-info.html

# AUTHOR

José Bollo <jose.bollo@iot.bzh>

