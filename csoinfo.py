from __future__ import print_function
import sys
import os

def pretty_print_filesize(fsize):
    if fsize < 1024:
        return '{} bytes'.format(fsize)

    if fsize < 1024 ** 2:
        return '{:.3f} KiB'.format(fsize / 1024)

    if fsize < 1024 ** 3:
        return '{:.3f} MiB'.format(fsize / (1024 ** 2))

    return '{:.3f} GiB'.format(fsize / (1024 ** 3))

def main(argv0, args):
    if not args:
        print("Usage: {} [-t] file.cso file.zso ...".format(argv0))
        return 1

    printtotal = args[0] == '-t'
    if printtotal:
        args.pop(0)

    totalsize = 0
    totalorig = 0
    for fname in args:
        try:
            with open(fname, 'rb') as f:
                data = f.read(24)
            if len(data) != 24:
                raise RuntimeError("{}: failed to read 24 bytes, read only {}".format(fname, len(data)))

            if data[:4] not in (b'CISO', b'ZISO'):
                raise RuntimeError("{}: no CISO or ZISO 4 magic bytes".format(fname))

            filesize = os.path.getsize(fname)
            origsize = int.from_bytes(data[8:16], 'little')
            blocksize = int.from_bytes(data[16:20], 'little')
            totalsize += filesize
            totalorig += origsize

            print("{}: {}, {}/{}, {}/{}, {:.2f}%, {} byte blocks".format(
                fname, 'cso' if data[:4] == b'CISO' else 'zso',
                filesize, origsize,
                pretty_print_filesize(filesize), pretty_print_filesize(origsize),
                (100.0 * filesize) / origsize, blocksize
            ))

        except Exception as e:
            print(e)

    if printtotal:
        print("TOTAL: total, {}/{}, {}/{}, {:.2f}%, {} byte blocks".format(
            totalsize, totalorig,
            pretty_print_filesize(totalsize), pretty_print_filesize(totalorig),
            (100.0 * totalsize) / totalorig, 0
        ))

if __name__ == '__main__':
    exit(main(sys.argv[0], sys.argv[1:]))
