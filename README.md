# winTouch

A minimal Windows command-line `touch` utility. It updates the last-access and
last-write timestamps of one or more files to the current time, creating them
if they do not already exist.

## Usage

```
touch <file> [file ...]
```

### Examples

```cmd
touch foo.txt                     :: create or update a single file
touch foo.txt bar.txt baz.txt     :: create or update multiple files
```

All files in the argument list are always processed; a failure on one file does
not prevent the remaining files from being touched.

### Exit codes

| Code | Meaning                             |
|------|-------------------------------------|
| 0    | All files touched successfully      |
| 1    | No arguments supplied               |
| 2    | One or more files could not be touched |

## Building

Open `winTouch.sln` in **Visual Studio 2022** and build, or use MSBuild from
the Developer Command Prompt:

```cmd
msbuild winTouch.sln /p:Configuration=Release /p:Platform=x64
```

The executable is written to `bin\<Platform>\<Configuration>\touch.exe`.

## Testing

Run the PowerShell test suite against the built binary:

```powershell
powershell -ExecutionPolicy Bypass -File tests\test.ps1
```

Pass `-ExePath` to point at a specific build:

```powershell
powershell -ExecutionPolicy Bypass -File tests\test.ps1 -ExePath bin\Win32\Release\touch.exe
```

## License

[MIT](LICENSE)
