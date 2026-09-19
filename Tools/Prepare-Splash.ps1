# Create Unreal's native Windows splash from the original, preserving its aspect ratio.
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
$projectRoot = Split-Path -Parent $PSScriptRoot
$outputDir = Join-Path $projectRoot 'Content\Splash'
New-Item -ItemType Directory -Force $outputDir | Out-Null
$source = [System.Drawing.Image]::FromFile((Join-Path $projectRoot '.art\splashscreen.png'))
try {
    $width = 1000
    $height = [int][Math]::Round($source.Height * $width / $source.Width)
    $bitmap = New-Object System.Drawing.Bitmap($width, $height)
    try {
        $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
        try {
            $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
            $graphics.DrawImage($source, 0, 0, $width, $height)
        } finally { $graphics.Dispose() }
        $bitmap.Save((Join-Path $outputDir 'Splash.bmp'), [System.Drawing.Imaging.ImageFormat]::Bmp)
    } finally { $bitmap.Dispose() }
} finally { $source.Dispose() }
