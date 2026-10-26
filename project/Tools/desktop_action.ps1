param([int]$X=-1, [int]$Y=-1, [string]$Keys)
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class Lab6Mouse {
    [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
    [DllImport("user32.dll")] public static extern void mouse_event(uint f, uint x, uint y, uint d, UIntPtr e);
}
'@
if ($X -ge 0 -and $Y -ge 0) {
    [void][Lab6Mouse]::SetCursorPos($X, $Y)
    [Lab6Mouse]::mouse_event(2,0,0,0,[UIntPtr]::Zero)
    [Lab6Mouse]::mouse_event(4,0,0,0,[UIntPtr]::Zero)
}
if ($Keys) { (New-Object -ComObject WScript.Shell).SendKeys($Keys) }
Start-Sleep -Milliseconds 600
