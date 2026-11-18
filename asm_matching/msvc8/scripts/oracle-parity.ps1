param([Parameter(Mandatory)][string]$Artifacts)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Read-One([string]$Text, [string]$Pattern, [string]$Name) {
    $found = [regex]::Matches($Text, $Pattern, [System.Text.RegularExpressions.RegexOptions]::Multiline)
    if ($found.Count -ne 1) { throw "Expected one $Name, found $($found.Count)" }
    return $found[0]
}

function Digest([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) { throw "Missing artifact: $Path" }
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()
}

function Read-Result([string]$Role) {
    $dir = Join-Path $Artifacts "oracle-$Role"
    $inputFile = Join-Path $dir 'oracle-inputs.json'
    $inputs = Get-Content -LiteralPath $inputFile -Raw | ConvertFrom-Json -AsHashtable
    if ($inputs.commit -notmatch '^[0-9a-f]{40}$' -or $inputs.sourceManifest -notmatch '^[0-9a-f]{64}$') {
        throw "$Role has invalid checkout/source provenance"
    }
    if (-not $inputs.ContainsKey('confirmedStarts') -or
        ($null -ne $inputs.confirmedStarts -and $inputs.confirmedStarts -notmatch '^[0-9a-f]{64}$')) {
        throw "$Role has invalid confirmed-start input provenance"
    }
    $evidence = Join-Path $dir 'asm_matching/msvc8/evidence'
    $audit = Get-Content -LiteralPath (Join-Path $evidence 'audit.tsv.summary') -Raw
    $libmap = Get-Content -LiteralPath (Join-Path $evidence 'libmap.tsv.summary') -Raw
    foreach ($name in @('MISSING', 'QUARANTINE')) {
        $m = Read-One $audit "^\s+$name\s+(\d+)\s*`$" "$Role $name"
        if ([long]$m.Groups[1].Value -ne 0) { throw "$Role has $name rows" }
    }
    $self = Read-One $audit '^self-check, names carrying their own address: (\d+) agree, (\d+) elsewhere\r?$' "$Role self-check"
    if ([long]$self.Groups[1].Value -le 0 -or [long]$self.Groups[2].Value -ne 0) { throw "$Role failed the address self-check" }
    $log = Get-Content -LiteralPath (Join-Path $dir 'oracle-all.log') -Raw
    if ($log -notmatch '(?m)^translate \(shipped ') { throw "$Role did not build the shipped translation" }
    $digests = [ordered]@{}
    foreach ($file in @('funcmap.tsv', 'libmap.tsv', 'audit.tsv', 'datamap.tsv')) {
        $digests[$file] = Digest (Join-Path $evidence $file)
    }
    $crtFile = Join-Path $evidence 'crtmap.tsv'
    $digests['crtmap.tsv'] = if (Test-Path -LiteralPath $crtFile -PathType Leaf) { Digest $crtFile } else { $null }
    if ($digests['crtmap.tsv'] -ne $inputs.crtmap) { throw "$Role changed the optional CRT input" }
    # Compare line order, independently of the host's stdout newline encoding.
    $order = @(Get-Content -LiteralPath (Join-Path $dir 'oracle-module-order.txt'))
    if ($order.Count -eq 0) { throw "$Role did not report module order" }
    return @{
        role = $Role
        inputs = $inputs
        headline = (Read-One $audit '^Without the C runtime: .+\r?$' "$Role headline").Value.TrimEnd("`r")
        totalMatch = [long](Read-One $audit '^\s+MATCH\s+(\d+)\s*\r?$' "$Role total MATCH").Groups[1].Value
        runtime = (Read-One $audit '^  of which the statically linked runtime \(libmap.tsv\): .+\r?$' "$Role runtime").Value.TrimEnd("`r")
        library = (Read-One $libmap '^paired with a library function: .+\r?$' "$Role library pairs").Value.TrimEnd("`r")
        digests = $digests
        moduleOrder = $order
    }
}

function Same-Inputs($A, $B) {
    foreach ($key in @('sourceManifest', 'confirmedStarts', 'crtmap', 'prebuilt', 'linkerMap')) {
        if ($A.inputs[$key] -cne $B.inputs[$key]) { return $false }
    }
    return $true
}

function Assert-Same($A, $B) {
    foreach ($key in @('headline', 'totalMatch', 'runtime', 'library')) {
        if ($A[$key] -cne $B[$key]) { throw "$($A.role) and $($B.role) differ: $key" }
    }
    foreach ($file in $A.digests.Keys) {
        if ($A.digests[$file] -cne $B.digests[$file]) { throw "$($A.role) and $($B.role) differ: $file digest" }
    }
    if ($A.moduleOrder.Count -ne $B.moduleOrder.Count) { throw 'Module-order lengths differ' }
    for ($i = 0; $i -lt $A.moduleOrder.Count; ++$i) {
        if ($A.moduleOrder[$i] -cne $B.moduleOrder[$i]) { throw "Module order differs at row $i" }
    }
}

$baseline = Read-Result 'linux-baseline'
$linux = Read-Result 'linux-candidate'
$windows = Read-Result 'windows-candidate'
if ($linux.inputs.commit -cne $windows.inputs.commit) { throw 'Candidate hosts checked out different commits' }
if (-not (Same-Inputs $linux $windows)) { throw 'Candidate hosts used different source/CRT/prebuilt/linker inputs' }
Assert-Same $linux $windows
$publishedParity = Same-Inputs $baseline $linux
if ($publishedParity) { Assert-Same $baseline $linux }
# A later source export may legitimately change the figures. Host parity remains
# mandatory; main is an additional control when its shipped inputs are identical.
[ordered]@{
    candidateCommit = $linux.inputs.commit
    mainControlCommit = $baseline.inputs.commit
    candidateHostParity = $true
    publishedMainInputsIdentical = $publishedParity
    publishedMainParity = if ($publishedParity) { $true } else { 'not comparable: source or reference inputs changed' }
    headline = $linux.headline
    totalMatch = $linux.totalMatch
    runtime = $linux.runtime
    library = $linux.library
    rowDigests = $linux.digests
    moduleOrderRows = $linux.moduleOrder.Count
} | ConvertTo-Json -Depth 6
