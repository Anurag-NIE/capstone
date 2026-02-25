Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Generate a Word-openable document (RTF) containing all repository code + brief guidelines.
# Output: Capstone_Code_Documentation.rtf in repo root.

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
Set-Location $repoRoot

function Escape-RtfText {
    param([string]$Text)

    if ($null -eq $Text) { return '' }

    # Normalize newlines
    $t = $Text -replace "`r`n", "`n" -replace "`r", "`n"

    # Escape RTF control chars
    $t = $t.Replace('\', '\\').Replace('{', '\{').Replace('}', '\}')

    # Convert newlines to RTF paragraph breaks
    return ($t -replace "`n", "\\par`n")
}

# Prefer tracked files (avoids build artifacts)
$files = @()
try {
    $files = git ls-files 2>$null
} catch {
    $files = @()
}

if (-not $files -or $files.Count -eq 0) {
    $files = Get-ChildItem -File -Recurse | ForEach-Object {
        $_.FullName.Substring($repoRoot.Length).TrimStart('\','/') -replace '/', '\'
    }
}

$includeExt = @(
    '.cpp', '.c', '.h', '.hpp',
    '.sln', '.vcxproj', '.filters',
    '.bat', '.cmd', '.ps1',
    '.md', '.txt', '.lcp'
)

$files = $files |
    Where-Object { $_ -and (Test-Path $_) } |
    Where-Object { $includeExt -contains [IO.Path]::GetExtension($_).ToLowerInvariant() } |
    Sort-Object

$outPath = Join-Path $repoRoot 'Capstone_Code_Documentation.rtf'

$nl = "`r`n"
$rtf = New-Object System.Text.StringBuilder

# RTF header + fonts
[void]$rtf.Append('{\rtf1\ansi\deff0')
[void]$rtf.Append($nl)
[void]$rtf.Append('{\fonttbl{\f0 Courier New;}{\f1 Arial;}}')
[void]$rtf.Append($nl)

# Title
[void]$rtf.Append('\f1\fs32\b Capstone Code Documentation\b0\par')
[void]$rtf.Append($nl)
[void]$rtf.Append('\fs20 Generated from the current workspace.\par')
[void]$rtf.Append($nl)
[void]$rtf.Append('\par')
[void]$rtf.Append($nl)

# Brief guidelines
[void]$rtf.Append('\b Brief Guidelines\b0\par')
[void]$rtf.Append($nl)
[void]$rtf.Append('1. Open \b CapstoneSystem.sln\b0 in Visual Studio.\par')
[void]$rtf.Append($nl)
[void]$rtf.Append('2. Select configuration \b x64\b0 and \b Debug\b0 (or Release) and build the solution.\par')
[void]$rtf.Append($nl)
[void]$rtf.Append('3. Run the system using \b run_system.bat\b0 (repo root) or \b x64\\Debug\\run_capstone.bat\b0 after a build.\par')
[void]$rtf.Append($nl)
[void]$rtf.Append('4. Recipes are stored under \b recipes\\\b0 (e.g., recipe1.lcp, recipe2.lcp).\par')
[void]$rtf.Append($nl)
[void]$rtf.Append('5. Runtime logs are written to \b C:\\Capstone\\logs\\\b0 (see Logger implementations).\par')
[void]$rtf.Append($nl)
[void]$rtf.Append('\par')
[void]$rtf.Append($nl)

# File contents
foreach ($file in $files) {
    [void]$rtf.Append('\f1\b File: ' + (Escape-RtfText $file) + '\b0\par')
    [void]$rtf.Append($nl)
    [void]$rtf.Append('\f0\fs18')
    [void]$rtf.Append($nl)

    $content = Get-Content -Raw -LiteralPath $file
    $escaped = Escape-RtfText $content

    # Put the code on its own paragraph block
    [void]$rtf.Append($escaped)
    [void]$rtf.Append($nl)
    [void]$rtf.Append('\par\par')
    [void]$rtf.Append($nl)
}

[void]$rtf.Append('}')

# Write as ASCII-compatible RTF (UTF8 is generally fine too, but ASCII avoids older Word issues)
[IO.File]::WriteAllText($outPath, $rtf.ToString(), [Text.Encoding]::UTF8)

Write-Host "Generated: $outPath"