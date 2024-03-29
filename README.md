# csoinfo
Small Windows specific (see end of this readme for an explanation) single file C
program to **print compressed and original size of CSO/ZSO file (compressed ISO).**

```
$ csoinfo.exe test.cso
test.cso: cso, 46096341/58433536, 43.961 MiB/55.727 MiB, 78.89%, 2048 byte blocks
```

**If you're looking for a description of the cso/zso format or for a program
to compress iso to cso/zso and decompress iso from cso/zso then go to
[unknownbrackets/maxcso](https://github.com/unknownbrackets/maxcso/).**

Go to **releases** on this repo to get a self contained 32 bit Windows exe (made with Pelles C).

A Python implementation that functions the same is also available in this repo,
it should work with Python 2.7 and Python 3.x

It reads the iso size and block size from the cso/zso header and the cso/zso
size is the size of the input file itself.


# Invoking and Output

If you run it with no arguments you get a short usage message. Otherwise you get
as many lines of output as the amount of arguments, no exceptions. All the output
is to stdout, nothing goes to stderr. Stdin is not read at all either.

If there are any errors then the line format is `filename: error message`.

If there are no errors then the line format is
`filename: type, csobytes/isobytes, csosize/isosize, percent, blocksize` where
`type` is `cso` or `zso` (based on header, regardless of file extension),
`csosize` and `isosize` are 'human friendly' units, `percent` is how much
space does the cso take in comparison to the original iso (e.g. a 3 GiB cso
made out of 4 GiB iso would have 75% displayed there) and `blocksize` is in
the form of `XXXX byte blocks`.

The output is very friendly to scripts. The error message won't contain a `/`
nor `:` so if you split an output line by `:` and then check for presence of `/`
you can tell if the line is an erorr one or a valid output one. On Windows
filenames can't contain `:` so any `:` character will do, but on Linux you
should split by last `:` just in case. You can then split the valid output line
by `,` and get the five parts: type, bytes, human readable, percent, block size and
further split the sizes by `/` and strip the fluff text and parse them all.

If your iso and cso are both under 1 KiB (1024 bytes) then the human readable
part will read `xxx.000 bytes` (where `xxx` is the correct number) but that's
so unlikely to ever happen in practice that I didn't bother with that edge case.

If **first** argument is exactly `-t` then it will print a sum total at the
end in the same format as successfully scanned cso files with name 'TOTAL',
`type` 'total' and blocksize of 0 bytes.
If you need to open a file named `-t` then you should pass it as `./-t` or
as argument other than the first one, but you probably shouldn't store any
cso files inside a file named `-t` with no extension anyway.

See examples below.


# Examples

```
$ csoinfo.exe
Usage: csoinfo.exe [-t] file.cso file.zso ...
```

```
$ csoinfo.exe test.cso test.iso
test.cso: cso, 46096341/58433536, 43.961 MiB/55.727 MiB, 78.89%, 2048 byte blocks
test.iso: no CISO or ZISO 4 magic bytes
```

```
$ csoinfo.exe -t test.cso test8192.cso test4096.zso
test.cso: cso, 46096341/58433536, 43.961 MiB/55.727 MiB, 78.89%, 2048 byte blocks
test8192.cso: cso, 44504019/58433536, 42.442 MiB/55.727 MiB, 76.16%, 8192 byte blocks
test4096.zso: zso, 49247866/58433536, 46.966 MiB/55.727 MiB, 84.28%, 4096 byte blocks
TOTAL: total, 139848226/175300608, 133.370 MiB/167.180 MiB, 79.78%, 0 byte blocks
```

```
$ csoinfo.exe test.iso nofile.hehe smallfile
test.iso: no CISO or ZISO 4 magic bytes
nofile.hehe: _wfopen failed
smallfile: fread failed = 2
```

```
$ csoinfo.exe just-header.cso
just-header.cso: cso, 24/58433536, 24.000 bytes/55.727 MiB, 0.00%, 2048 byte blocks
```

# Windows specific?

Yes, I happen to use Windows 10 right now and I wanted the tool to handle
Unicode filenames (not that common but nice to have) and cso files bigger than
2 GiB (very common). Both of these are done via Windows specific code.

Unicode filenames are handled by using `wide`/`wchar_t` functions, `wmain`,
`wprintf`, `wfopen`, etc. everywhere instead of `char` ones that are usually
UTF-8 on Linux and others.

Big files are handled by using Windows specific version of `ftell` and `fseek`.
I use `ftell` to get file size of the input cso file but on Windows it always
returns a `long` which is always signed 4 bytes. On Linux and others it depends.
POSIX and Windows both have their own replacements/versions of those functions
that handle big files but they are named differently and so on. There are other
ways to get file size (`stat`, `GetFileSizeEx`) but they'd also need to be
used in wide version on Windows (`stat`) or are OS specific (`GetFileSizeEx`).

**If you happen to want to use this on Linux feel free to open an issue and
I'll add the required code.** If you don't want to do that you can just do it
yourself: replace all the `wchar_t` stuff with the `char` equivalents of it and
replace `_fseeki64` and `_ftelli64` with `fseeko` and `ftello` and define right
macros to make `off_t` 64 bit.

I've tested it with both Pelles C and Visual Studio 2017 on Windows 10.

There are some differences with some format specifiers related to wide chars in
wprintf between those two so be careful if you make any changes there. Look
for warnings during compilation and test the program in both to avoid problems.
