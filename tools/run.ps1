param(
    [switch]$Windowed,
    [ValidateSet('720p','1080p','1440p','4k')][string]$Resolution='720p',
    [switch]$DryRun,
    [ValidateSet('none','fast','some','full')][string]$ReadbackResolve='none',
    [ValidateRange(0,240)][int]$FrameLimit=0
)
$ErrorActionPreference='Stop'
$projectRoot=Split-Path $PSScriptRoot -Parent
$executable=Join-Path $projectRoot 'out/build/win-amd64-release/fable_anniversary.exe'
$game=Join-Path $projectRoot 'game'
if(!(Test-Path -LiteralPath $executable)){throw 'Build the standalone runtime with tools/build.ps1 first.'}
if(!(Test-Path -LiteralPath (Join-Path $game 'default.xex'))){throw 'Supply your own extracted disc files under game/ first.'}
$launchArguments=@('--game_data_root',('"'+$game+'"'),"--fullscreen=$((!$Windowed.IsPresent).ToString().ToLowerInvariant())","--resolution=$Resolution",'--resolution_scale=1','--vsync=true','--async_shader_compilation=false')
$launchArguments += "--readback_resolve=$ReadbackResolve"
$launchArguments += "--anniversary_frame_limit=$FrameLimit"
if ($FrameLimit -gt 0) { $launchArguments = @($launchArguments | Where-Object {$_ -ne '--vsync=true'}) + '--vsync=false' }
if($DryRun){$launchArguments;return}
Start-Process -FilePath $executable -WorkingDirectory (Split-Path $executable) -ArgumentList $launchArguments -WindowStyle Normal
