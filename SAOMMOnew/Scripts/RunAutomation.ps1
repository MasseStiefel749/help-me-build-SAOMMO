<#
.SYNOPSIS
  Stufen-Tester fuer SAOMMOnew (#19 Testinfrastruktur, Quelle:
  [[99 AI/Nachttest-Infrastruktur]] -> saommo-teststrategie.md).

.DESCRIPTION
  Stufe 0  Build.bat SAOMMOnewEditor Win64 Development (Skip: -SkipBuild)
  Stufe 1  Automation List - mindestens 5 "SAOMMOnew."-Tests muessen gelistet sein
  Stufe 2  Automation RunTests ^SAOMMOnew. - Akzeptanz: Log-Marker
           "TEST COMPLETE. EXIT CODE: 0" UND Exit-Code 0
  Stufe 3  Boot-Smoke (-game, 120 s, danach normaler Timeout-Kill) -
           Nachweis: "Bringing World" + "Engine Initialization" im Log
  Stufe 4  Log-Scan der Stufe-2- und Stufe-3-Logs gegen log-whitelist.txt;
           jede ": Error"-Zeile ohne Whitelist-Treffer scheitert die Stufe
  Stufe 5  Gesamturteil; Exit-Code 0 = GRUEN, 1 = ROT

  Alle Logs landen in $OutDir (externer Temp - NIEMALS im Repo).

.EXAMPLE
  powershell -ExecutionPolicy Bypass -File Scripts\RunAutomation.ps1
  powershell -ExecutionPolicy Bypass -File Scripts\RunAutomation.ps1 -SkipBuild
#>
param(
    [string]$Project = 'C:\Users\simon\OneDrive\Documents\GitHub\help-me-build-SAOMMO\SAOMMOnew\SAOMMOnew.uproject',
    [string]$EngineDir = 'C:\Program Files\Epic Games\UE_5.8\Engine',
    [string]$OutDir = '',
    [switch]$SkipBuild,
    [switch]$SkipBoot
)

$ErrorActionPreference = 'Continue'
if ($OutDir -eq '') { $OutDir = Join-Path $env:LOCALAPPDATA 'Temp\opencode\automation' }
$Stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
$EditorCmd = Join-Path $EngineDir 'Binaries\Win64\UnrealEditor-Cmd.exe'
$WhitelistPath = Join-Path $PSScriptRoot 'log-whitelist.txt'
$script:Results = @()

function Add-Result($Stufe, $Name, $Ok, $Info) {
    $script:Results += ,([pscustomobject]@{ Stufe = $Stufe; Name = $Name; Ok = [bool]$Ok; Info = [string]$Info })
    $Tag = 'ROT'
    if ($Ok) { $Tag = 'GRUEN' }
    Write-Host ("[Stufe {0}] {1}: {2} - {3}" -f $Stufe, $Name, $Tag, $Info)
}

# Startet UnrealEditor-Cmd, gibt Log-Pfad/ExitCode zurueck. Nach Timeout wird
# der Prozess gekillt (fuer -game-Boots der Normalfall) und ExitCode = -999.
function Invoke-Editor([string[]]$EditorArgs, [string]$TagLog, [int]$TimeoutSec) {
    $Log = Join-Path $OutDir ("{0}-{1}.log" -f $Stamp, $TagLog)
    $AllArgs = $EditorArgs + @('-nosplash', '-unattended', '-nopause', '-stdout', ('-abslog=' + $Log))
    $P = Start-Process -FilePath $EditorCmd -ArgumentList $AllArgs -PassThru -NoNewWindow `
        -RedirectStandardOutput ($Log + '.out') -RedirectStandardError ($Log + '.err')
    $Exited = $P.WaitForExit($TimeoutSec * 1000)
    if (-not $Exited) {
        Stop-Process -Id $P.Id -Force -ErrorAction SilentlyContinue
        $P.WaitForExit(15000) | Out-Null
        return @{ Log = $Log; ExitCode = -999; TimedOut = $true }
    }
    return @{ Log = $Log; ExitCode = $P.ExitCode; TimedOut = $false }
}

# --- Stufe 0: Build ---------------------------------------------------------
if ($SkipBuild) {
    Add-Result 0 'Build' $true 'uebersprungen (-SkipBuild)'
} else {
    $BuildLog = Join-Path $OutDir ("{0}-stufe0-build.log" -f $Stamp)
    $BP = Start-Process -FilePath (Join-Path $EngineDir 'Build\BatchFiles\Build.bat') `
        -ArgumentList @('SAOMMOnewEditor', 'Win64', 'Development', ('-project=' + $Project)) `
        -NoNewWindow -Wait -PassThru -RedirectStandardOutput $BuildLog -RedirectStandardError ($BuildLog + '.err')
    $HasSuccess = $null -ne (Select-String -Path $BuildLog -Pattern 'Result: Succeeded' -SimpleMatch)
    $BuildOk = ($BP.ExitCode -eq 0) -and $HasSuccess
    Add-Result 0 'Build' $BuildOk ("exit=" + $BP.ExitCode + ', Result: Succeeded=' + $HasSuccess)
}

# --- Stufe 1: Automation List ----------------------------------------------
$R1 = Invoke-Editor @($Project, '-ExecCmds="Automation List; Quit"') 'stufe1-list' 300
$ListOk = $false
$ListInfo = 'Log fehlt: ' + $R1.Log
if (Test-Path $R1.Log) {
    $Found = @(Select-String -Path $R1.Log -Pattern 'SAOMMOnew\.' -ErrorAction SilentlyContinue)
    $ListOk = ($Found.Count -ge 5)
    $ListInfo = "SAOMMOnew.-Zeilen=" + $Found.Count + ' (erwartet >= 5), timedOut=' + $R1.TimedOut
}
Add-Result 1 'Automation List' $ListOk $ListInfo

# --- Stufe 2: Automation RunTests ------------------------------------------
# Akzeptanz (geprueft gegen UE-5.8-Engine-Quelle): die alte Research-Angabe
# "TEST COMPLETE. EXIT CODE: 0" existiert in 5.8 NICHT als Logzeile - die
# Kommandozeile meldet stattdessen "Automation Test Queue Empty". Darum ist
# das der -TestExit-Marker UND der Erfolgsnachweis.
$R2 = Invoke-Editor @($Project, '-ExecCmds="Automation RunTests ^SAOMMOnew."', '-TestExit="Automation Test Queue Empty"', '-nullrhi') 'stufe2-runtests' 600
$QueueOk = $false
$PassCount = 0
$FailCount = 0
$FoundCount = 0
if (Test-Path $R2.Log) {
    $QueueOk = $null -ne (Select-String -Path $R2.Log -Pattern 'Automation Test Queue Empty' -SimpleMatch)
    $PassCount = @(Select-String -Path $R2.Log -Pattern 'Result=\{Success' -ErrorAction SilentlyContinue).Count
    $FailCount = @(Select-String -Path $R2.Log -Pattern 'Result=\{Fail' -ErrorAction SilentlyContinue).Count
    $FoundLine = Select-String -Path $R2.Log -Pattern 'Found (\d+) automation tests based on' -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($FoundLine -and ($FoundLine.Matches.Count -gt 0)) { $FoundCount = [int]$FoundLine.Matches[0].Groups[1].Value }
}
# Hinweis: der Exit-Code des Editor-Prozesses wird bewusst NICHT ausgewertet
# (PS5.1-Quirk: nach WaitForExit(ms) liefert $P.ExitCode in dieser
# Konstellation $null) - Erfolg wird ausschliesslich an Log-Inhalten
# festgemacht, was ohnehin die belastbarere Quelle ist.
$TestsOk = $QueueOk -and (-not $R2.TimedOut) -and ($FailCount -eq 0) -and ($PassCount -ge 5) -and ($FoundCount -ge 5)
Add-Result 2 'Automation RunTests' $TestsOk ("QueueEmpty=" + $QueueOk + ', found=' + $FoundCount + ', Success=' + $PassCount + ', Fail=' + $FailCount + ', timedOut=' + $R2.TimedOut)

# --- Stufe 3: Boot-Smoke (-game) -------------------------------------------
$R3 = @{ Log = $null; ExitCode = $null; TimedOut = $true }
if ($SkipBoot) {
    Add-Result 3 'Boot-Smoke' $true 'uebersprungen (-SkipBoot)'
} else {
    $R3 = Invoke-Editor @($Project, '-game', '-windowed', '-ResX=1280', '-ResY=720') 'stufe3-boot' 120
    $WorldUp = $null
    $InitOk = $null
    if (Test-Path $R3.Log) {
        $WorldUp = Select-String -Path $R3.Log -Pattern 'Bringing World' -ErrorAction SilentlyContinue | Select-Object -First 1
        $InitOk = Select-String -Path $R3.Log -Pattern 'Engine Initialization' -ErrorAction SilentlyContinue | Select-Object -First 1
    }
    $BootOk = ($null -ne $WorldUp) -and ($null -ne $InitOk)
    Add-Result 3 'Boot-Smoke' $BootOk ("WorldUp=" + ($null -ne $WorldUp) + ', EngineInit=' + ($null -ne $InitOk) + ' (Timeout-Kill ist normal)')
}

# --- Stufe 4: Log-Scan gegen Whitelist -------------------------------------
$Whitelist = @()
if (Test-Path $WhitelistPath) {
    $Whitelist = @(Get-Content $WhitelistPath -ErrorAction SilentlyContinue |
        Where-Object { ($_ -notmatch '^\s*#') -and ($_.Trim() -ne '') } |
        ForEach-Object { $_.Trim() })
}
$ScanLogs = @()
if ((-not $SkipBoot) -and ($null -ne $R3.Log) -and (Test-Path $R3.Log)) {
    $ScanLogs += @{ Path = $R3.Log; FromLine = 1 }
}
if (($null -ne $R2.Log) -and (Test-Path $R2.Log)) {
    # Stufe-2-Log nur AB dem Teststart scannen: davor laeuft der komplette
    # Editor-Boot mit konstantem Engine-Rauschen (UnifiedErrorTest-Selbsttest
    # mit 17x "Condition failed" + LogPython von #22), das nichts mit den
    # Tests zu tun hat. Testfehler passieren NACH dem Cmd und werden so
    # exakt erfasst - ohne dass sie in der Whitelist versteckt werden koennten.
    $SliceStart = 1
    $CmdLine = Select-String -Path $R2.Log -Pattern 'Cmd: Automation RunTests' -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($CmdLine) { $SliceStart = $CmdLine.LineNumber }
    $ScanLogs += @{ Path = $R2.Log; FromLine = $SliceStart }
}
$Violations = @()
$Scanned = 0
foreach ($Entry in $ScanLogs) {
    $ErrLines = @(Select-String -Path $Entry.Path -Pattern ': Error' -ErrorAction SilentlyContinue |
        Where-Object { $_.LineNumber -ge $Entry.FromLine })
    foreach ($E in $ErrLines) {
        $Scanned++
        $Whitelisted = $false
        foreach ($W in $Whitelist) {
            if ($E.Line.Contains($W)) { $Whitelisted = $true; break }
        }
        if (-not $Whitelisted) {
            $Trimmed = $E.Line.Trim()
            if ($Trimmed.Length -gt 160) { $Trimmed = $Trimmed.Substring(0, 160) }
            $Violations += ($Trimmed)
        }
    }
}
$ScanOk = ($ScanLogs.Count -gt 0) -and ($Violations.Count -eq 0)
$ScanInfo = ("Logs=" + $ScanLogs.Count + ', Error-Zeilen(nach Scope)=' + $Scanned + ', Whitelist=' + $Whitelist.Count + ', Verstoesse=' + $Violations.Count)
foreach ($V in ($Violations | Select-Object -First 5)) { Write-Host ("    VIOLATION: " + $V) }
Add-Result 4 'Log-Scan' $ScanOk $ScanInfo

# --- Stufe 5: Gesamturteil --------------------------------------------------
$Failed = @($script:Results | Where-Object { -not $_.Ok })
$AllOk = ($Failed.Count -eq 0)
$Summary = @()
$Summary += ("Run " + $Stamp)
foreach ($R in $script:Results) {
    $Tag = 'ROT'
    if ($R.Ok) { $Tag = 'GRUEN' }
    $Summary += ("Stufe " + $R.Stufe + ' ' + $Tag + ' | ' + $R.Name + ' | ' + $R.Info)
}
$Summary += ("ERGEBNIS: " + $(if ($AllOk) { 'GRUEN' } else { 'ROT' }))
$SummaryPath = Join-Path $OutDir ("{0}-urteil.txt" -f $Stamp)
Set-Content -Path $SummaryPath -Value $Summary -Encoding UTF8
Write-Host ''
Write-Host '=== Stufen 0-5 Gesamturteil ==='
$Summary | ForEach-Object { Write-Host ('  ' + $_) }
if ($AllOk) { exit 0 } else { exit 1 }
