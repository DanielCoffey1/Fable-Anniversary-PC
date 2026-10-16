param([Parameter(Mandatory)][string]$Recompiler,[Parameter(Mandatory)][string]$Manifest,[Parameter(Mandatory)][string]$Stamp)
$ErrorActionPreference='Stop'
$before=Get-FileHash -LiteralPath $Manifest -Algorithm SHA256
$writeTime=(Get-Item -LiteralPath $Manifest).LastWriteTimeUtc
& $Recompiler codegen $Manifest
if($LASTEXITCODE -ne 0){exit $LASTEXITCODE}
# v0.10.0 rewrites its version stamp even when the manifest bytes are unchanged.
# Preserve metadata only for an exact hash match; real edits remain dependencies.
if($before.Hash -eq (Get-FileHash -LiteralPath $Manifest -Algorithm SHA256).Hash){
    (Get-Item -LiteralPath $Manifest).LastWriteTimeUtc=$writeTime
}
[IO.Directory]::CreateDirectory((Split-Path -Parent $Stamp)) | Out-Null
if (!(Test-Path -LiteralPath $Stamp)) { [IO.File]::WriteAllText($Stamp,'') }
[IO.File]::SetLastWriteTimeUtc($Stamp,[datetime]::UtcNow)
