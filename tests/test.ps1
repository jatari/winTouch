<#
.SYNOPSIS
    Smoke-test suite for the touch.exe binary.

.PARAMETER ExePath
    Path to touch.exe. Defaults to bin\x64\Release\touch.exe relative to the
    repository root.
#>
param(
    [string]$ExePath = "$PSScriptRoot\..\bin\x64\Release\touch.exe"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$passed = 0
$failed = 0

function Pass([string]$msg) { Write-Host "PASS: $msg" -ForegroundColor Green; $script:passed++ }
function Fail([string]$msg) { Write-Host "FAIL: $msg" -ForegroundColor Red;  $script:failed++ }
function Assert([bool]$cond, [string]$msg) { if ($cond) { Pass $msg } else { Fail $msg } }

# Run the executable while suppressing NativeCommandError.
# In Windows PowerShell 5.1 (used by this workflow) any stderr output from a
# native process is converted into a NativeCommandError/RemoteException that
# $ErrorActionPreference='Stop' turns into a terminating error.  Setting the
# local preference to SilentlyContinue inside the function scope prevents that
# without affecting the rest of the script.  $LASTEXITCODE is a global automatic
# variable updated by the runtime after the process exits, so it is still
# readable in the caller after this function returns.
function Invoke-Touch ([string[]]$ExeArgs) {
    $local:ErrorActionPreference = 'SilentlyContinue'
    & $ExePath @ExeArgs 2>&1 | Out-Null
}

if (-not (Test-Path $ExePath)) {
    Write-Error "Executable not found: $ExePath"
    exit 1
}

Write-Host "Testing: $ExePath`n"

# ── Test 1: no arguments → exit code 1 ────────────────────────────────────────
Invoke-Touch
Assert ($LASTEXITCODE -eq 1) "No arguments: exit code 1"

# ── Test 2: create a new file ─────────────────────────────────────────────────
$tmp = [IO.Path]::GetTempFileName()
Remove-Item $tmp -Force
Assert (-not (Test-Path $tmp)) "Pre-condition: new-file target does not yet exist"
Invoke-Touch $tmp
Assert ($LASTEXITCODE -eq 0) "Create new file: exit code 0"
Assert (Test-Path $tmp) "Create new file: file exists afterwards"
Remove-Item $tmp -Force -ErrorAction SilentlyContinue

# ── Test 3: update the timestamp of an existing file ──────────────────────────
$tmp = [IO.Path]::GetTempFileName()
$before = (Get-Item $tmp).LastWriteTimeUtc
Start-Sleep -Milliseconds 1100   # ensure a ≥1-second gap (FAT-style resolution)
Invoke-Touch $tmp
Assert ($LASTEXITCODE -eq 0) "Touch existing file: exit code 0"
$after = (Get-Item $tmp).LastWriteTimeUtc
Assert ($after -gt $before) "Touch existing file: write time advanced"
Remove-Item $tmp -Force -ErrorAction SilentlyContinue

# ── Test 4: multiple files at once ────────────────────────────────────────────
$temps = 1..3 | ForEach-Object {
    $f = [IO.Path]::GetTempFileName()
    Remove-Item $f -Force
    $f
}
Invoke-Touch @temps
Assert ($LASTEXITCODE -eq 0) "Multiple files: exit code 0"
foreach ($f in $temps) {
    Assert (Test-Path $f) "Multiple files: $([IO.Path]::GetFileName($f)) created"
    Remove-Item $f -Force -ErrorAction SilentlyContinue
}

# ── Test 5: invalid path → exit code 2 ───────────────────────────────────────
$bad = "C:\NonExistentDirectory_winTouchTest_$([guid]::NewGuid().Guid)\test.txt"
Invoke-Touch $bad
Assert ($LASTEXITCODE -eq 2) "Invalid path: exit code 2"

# ── Test 6: failure on one file does not abort remaining files ────────────────
$good = [IO.Path]::GetTempFileName()
Remove-Item $good -Force
Invoke-Touch $bad $good
Assert ($LASTEXITCODE -eq 2) "Mixed input: overall exit code 2 (partial failure)"
Assert (Test-Path $good) "Mixed input: valid file was still created"
Remove-Item $good -Force -ErrorAction SilentlyContinue

# ── Summary ───────────────────────────────────────────────────────────────────
Write-Host "`n$passed passed, $failed failed."
if ($failed -gt 0) { exit 1 }
