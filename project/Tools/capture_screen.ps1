param([Parameter(Mandatory=$true)][string]$Name, [int]$ProcessId)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
if ($ProcessId) {
    Add-Type @'
using System;
using System.Runtime.InteropServices;
public class Lab6Window {
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int command);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
}
'@
    $handle = (Get-Process -Id $ProcessId).MainWindowHandle
    [void][Lab6Window]::ShowWindow($handle, 9)
    [void][Lab6Window]::ShowWindow($handle, 3)
    [void][Lab6Window]::SetForegroundWindow($handle)
    Start-Sleep -Milliseconds 700
}
$bounds = [Windows.Forms.Screen]::PrimaryScreen.Bounds
$bitmap = [Drawing.Bitmap]::new($bounds.Width, $bounds.Height)
$graphics = [Drawing.Graphics]::FromImage($bitmap)
$graphics.CopyFromScreen($bounds.Location, [Drawing.Point]::Empty, $bounds.Size)
$pictureRoot = Join-Path (Split-Path -Parent (Split-Path -Parent $PSScriptRoot)) 'picture'
$destination = Join-Path $pictureRoot $Name
$bitmap.Save($destination, [Drawing.Imaging.ImageFormat]::Png)
$graphics.Dispose()
$bitmap.Dispose()
Write-Output $destination
