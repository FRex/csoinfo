#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <wchar.h>

static const wchar_t * filepath_to_filename(const wchar_t * path)
{
    size_t i, len, lastslash;

    len = wcslen(path);
    lastslash = 0u;
    for(i = 0u; i < len; ++i)
        if(path[i] == L'/' || path[i] == L'\\')
            lastslash = i + 1;

    return path + lastslash;
}

typedef long long s64;

static s64 get_file_size(FILE * f)
{
    s64 ret = 0;

    if(0 == _fseeki64(f, 0, SEEK_END))
        ret = _ftelli64(f);

    if(ret == -1)
        ret = 0;

    return ret;
}

static s64 little_s64(const char * buff)
{
    s64 ret = 0;
    const unsigned char * b = (const unsigned char *)buff;

    ret = (ret | b[7]) << 8;
    ret = (ret | b[6]) << 8;
    ret = (ret | b[5]) << 8;
    ret = (ret | b[4]) << 8;
    ret = (ret | b[3]) << 8;
    ret = (ret | b[2]) << 8;
    ret = (ret | b[1]) << 8;
    ret = (ret | b[0]);

    return ret;
}

static double pretty_file_size_adjust(s64 filesize)
{
    double ret = (double)filesize;
    while(ret > 1024.0)
        ret /= 1024.0;

    return ret;
}

const wchar_t * pretty_file_size_unit(s64 filesize)
{
    if(filesize < 1024)
        return L"bytes";

    filesize /= 1024;
    if(filesize < 1024)
        return L"KiB";

    filesize /= 1024;
    if(filesize < 1024)
        return L"MiB";

    filesize /= 1024;
    if(filesize < 1024)
        return L"GiB";

    filesize /= 1024;
    if(filesize < 1024)
        return L"TiB";

    return L"TiB";
}

static int process_cso_file(const wchar_t * fname)
{
    char buff[32];
    size_t readcount;

    FILE * f = _wfopen(fname, L"rb");
    if(!f)
    {
        wprintf(L"%ls: _wfopen failed\n", fname);
        return 1;
    }

    readcount = fread(buff, 1, 24, f);
    if(readcount != 24)
    {
        wprintf(L"%ls: fread failed = %d\n", fname, (int)readcount);
        fclose(f);
        return 1;
    }
    else
    {
        if(0 != strncmp(buff, "CISO", 4u))
        {
            wprintf(L"%ls: no CISO 4 magic bytes\n", fname);
            fclose(f);
            return 1;
        }
        else
        {
            const s64 filesize = get_file_size(f);
            const s64 origsize = little_s64(buff + 8);
            const double percent = 100.0 * (filesize / (double)(origsize ? origsize : 1));
            wprintf(L"%ls: ", fname);
            wprintf(L"%llu/%llu, ", filesize, origsize);
            wprintf(L"%.3f %ls/%.3f %ls, %.2f%%\n",
                pretty_file_size_adjust(filesize), pretty_file_size_unit(filesize),
                pretty_file_size_adjust(origsize), pretty_file_size_unit(origsize),
                percent
            );
        }
    }

    fclose(f);
    return 0;
}

int wmain(int argc, wchar_t ** argv)
{
    int i, ret;

    if(argc < 2)
    {
        wprintf(L"Usage: %ls file.cso ...\n", filepath_to_filename(argv[0]));
        return 1;
    }

    ret = 0;
    for(i = 1; i < argc; ++i)
        if(process_cso_file(argv[i]))
            ret = 1;

    return ret;
}
