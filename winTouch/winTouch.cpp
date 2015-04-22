/*
The MIT License(MIT)

Copyright(c) < year > <copyright holders>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "stdafx.h"
#include "stdio.h"

#include "windows.h"

static const TCHAR ERROR_NO_ARGUMENTS[] = _T("No arguments were given!\n");
static const TCHAR ERROR_TOUCHING_FAILED_S[] = _T("Touching failed: %s\n");

static const int ERROR_NONE = 0;
static const int ERROR_CODE_ARGUMENT = 1;
static const int ERROR_CODE_TOUCH_FAILED = 2;

BOOL touch_file(TCHAR* file_name)
{
	BOOL success = FALSE;

	HANDLE handle = CreateFile(
		file_name,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if( handle != INVALID_HANDLE_VALUE )
	{
		SYSTEMTIME system_time;
		FILETIME current_time;

		GetSystemTime(&system_time);
		SystemTimeToFileTime(&system_time, &current_time);
		success = SetFileTime(handle, &current_time, &current_time, &current_time);
	}

	CloseHandle(handle);

	return success;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int error = 0;

	if( argc == 1 )
	{
		_tprintf(ERROR_NO_ARGUMENTS);
		error = ERROR_CODE_ARGUMENT;
	}
	else if( argc > 1 )
	{
		for( int i = 1; error == ERROR_NONE && i < argc; ++i )
		{
			TCHAR* arg_file = argv[i];
			BOOL success = touch_file(arg_file);
			if( !success )
			{
				error = ERROR_CODE_TOUCH_FAILED;
				_tprintf(ERROR_TOUCHING_FAILED_S, arg_file);
			}
		}
	}
	return error;
}

