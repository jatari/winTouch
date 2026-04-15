/*
The MIT License (MIT)

Copyright (c) 2015 jatari

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

static const int EXIT_OK           = 0;
static const int EXIT_BAD_ARGS     = 1;
static const int EXIT_TOUCH_FAILED = 2;

static void print_touch_error(const TCHAR *file_name, DWORD error_code)
{
    LPVOID raw_message = NULL;
    DWORD len = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error_code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&raw_message,
        0,
        NULL);

    if (raw_message != NULL)
    {
        LPTSTR message = (LPTSTR)raw_message;
        /* Strip trailing CR/LF that FormatMessage appends */
        while (len > 0 && (message[len - 1] == _T('\r') || message[len - 1] == _T('\n')))
            message[--len] = _T('\0');
        _ftprintf(stderr, _T("touch: %s: error %lu: %s\n"), file_name, (unsigned long)error_code, message);
        LocalFree(raw_message);
    }
    else
    {
        _ftprintf(stderr, _T("touch: %s: error %lu\n"), file_name, (unsigned long)error_code);
    }
}

static BOOL touch_file(const TCHAR *file_name)
{
    /* FILE_WRITE_ATTRIBUTES is sufficient to update timestamps.
       OPEN_ALWAYS creates the file if it does not exist (standard touch behaviour). */
    HANDLE handle = CreateFile(
        file_name,
        FILE_WRITE_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (handle == INVALID_HANDLE_VALUE)
        return FALSE;

    SYSTEMTIME system_time;
    FILETIME current_time;
    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &current_time);

    /* Update last-access and last-write times; leave creation time unchanged. */
    BOOL success = SetFileTime(handle, NULL, &current_time, &current_time);

    CloseHandle(handle);
    return success;
}

int _tmain(int argc, _TCHAR *argv[])
{
    if (argc < 2)
    {
        _ftprintf(stderr, _T("Usage: touch <file> [file ...]\n"));
        return EXIT_BAD_ARGS;
    }

    int exit_code = EXIT_OK;
    for (int i = 1; i < argc; ++i)
    {
        if (!touch_file(argv[i]))
        {
            /* Capture the error code immediately; subsequent calls may overwrite it. */
            DWORD err = GetLastError();
            print_touch_error(argv[i], err);
            exit_code = EXIT_TOUCH_FAILED;
        }
    }
    return exit_code;
}

