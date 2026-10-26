$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$utf8 = [System.Text.UTF8Encoding]::new($false)
$u8g2Root = Join-Path $projectRoot 'Middlewares\U8g2'

# Keep the upstream license header and only the selected setup/buffer functions.
foreach ($item in @(
    @('u8g2_d_setup.c', 'void u8g2_Setup_ssd1306_i2c_128x64_noname_f('),
    @('u8g2_d_memory.c', 'uint8_t *u8g2_m_16_8_f(')
)) {
    $file = Join-Path $u8g2Root $item[0]
    $source = [IO.File]::ReadAllText($file)
    $headerEnd = $source.IndexOf('*/') + 2
    $start = $source.IndexOf($item[1])
    if ($start -lt 0) { throw "Function not found: $($item[1])" }
    $open = $source.IndexOf('{', $start)
    $depth = 1
    $end = $open + 1
    while ($depth -gt 0) {
        if ($source[$end] -eq '{') { $depth++ }
        if ($source[$end] -eq '}') { $depth-- }
        $end++
    }
    $trimmed = $source.Substring(0, $headerEnd) + "`n#include `"u8g2.h`"`n`n" + $source.Substring($start, $end - $start) + "`n"
    [IO.File]::WriteAllText($file, $trimmed, $utf8)
}

$demo = [IO.File]::ReadAllText((Join-Path $projectRoot 'Core\Src\oled_demo.c'))
$codes = @($demo.ToCharArray() | Where-Object { [int]$_ -gt 127 } | ForEach-Object { [int]$_ } | Sort-Object -Unique)
$map = '32-126,' + ($codes -join ',')
[IO.File]::WriteAllText((Join-Path $projectRoot 'Assets\font_map.txt'), $map, $utf8)

# Save just the required BDF glyphs, so the full upstream Chinese font is unnecessary.
$bdfFile = Join-Path $projectRoot 'Assets\lab6_subset.bdf'
$fullBdf = Join-Path $projectRoot 'Assets\wqy_full.bdf'
if (Test-Path -LiteralPath $fullBdf) {
    $bdf = [IO.File]::ReadAllText($fullBdf)
    $header = $bdf.Substring(0, $bdf.IndexOf('CHARS '))
    $glyphs = @([regex]::Matches($bdf, '(?ms)^STARTCHAR .*?^ENDCHAR\r?\n') | Where-Object {
        $code = [int][regex]::Match($_.Value, '(?m)^ENCODING (\d+)').Groups[1].Value
        ($code -ge 32 -and $code -le 126) -or ($codes -contains $code)
    })
    if ($glyphs.Count -ne 95 + $codes.Count) { throw 'Missing font glyphs' }
    [IO.File]::WriteAllText($bdfFile, $header + "CHARS $($glyphs.Count)`n" + (($glyphs | ForEach-Object Value) -join '') + "ENDFONT`n", $utf8)
}
$fontFile = Join-Path $projectRoot 'Core\Src\lab6_font.c'
& (Join-Path $PSScriptRoot 'bdfconv.exe') -b 0 -f 1 -m $map -n lab6_font -o $fontFile $bdfFile
if ($LASTEXITCODE -ne 0) { throw 'Font conversion failed' }
$font = [IO.File]::ReadAllText($fontFile)
[IO.File]::WriteAllText($fontFile, "#include `"u8g2.h`"`n" + $font, $utf8)
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'u8g2_font_6x12_tf.c') -Destination (Join-Path $projectRoot 'Core\Src\u8g2_font_6x12_tf.c') -Force
$asciiFile = Join-Path $projectRoot 'Core\Src\u8g2_font_6x12_tf.c'
$asciiFont = [IO.File]::ReadAllText($asciiFile)
if (-not $asciiFont.Contains('#include')) {
    [IO.File]::WriteAllText($asciiFile, "#include `"u8g2.h`"`n" + $asciiFont, $utf8)
}

$uvFile = Join-Path $projectRoot 'MDK-ARM\Lab6_OLED.uvprojx'
[xml]$uv = Get-Content -LiteralPath $uvFile -Raw
$target = $uv.Project.Targets.Target
foreach ($entry in @(@('pCCUsed','5060528::V5.06 update 5 (build 528)::ARMCC'), @('uAC6','0'))) {
    $node = $target.SelectSingleNode($entry[0])
    if ($null -eq $node) { $node = $uv.CreateElement($entry[0]); $null = $target.AppendChild($node) }
    $node.InnerText = $entry[1]
}
$common = $target.TargetOption.TargetCommonOption
$common.Cpu = 'IRAM(0x20000000,0x5000) IROM(0x08000000,0x10000) CPUTYPE("Cortex-M3") CLOCK(64000000) ELITTLE'
foreach ($entry in @(@('PackID','Keil.STM32F1xx_DFP.2.2.0'), @('PackURL','http://www.keil.com/pack/'))) {
    $node = $common.SelectSingleNode($entry[0])
    if ($null -eq $node) { $node = $uv.CreateElement($entry[0]); $null = $common.AppendChild($node) }
    $node.InnerText = $entry[1]
}
$common.SFDFile = '$$Device:STM32F103C8$SVD\STM32F103xx.svd'
$common.FlashDriverDll = 'UL2CM3(-S0 -C0 -P0 -FD20000000 -FC1000 -FN1 -FF0STM32F10x_128 -FS08000000 -FL010000 -FP0($$Device:STM32F103C8$Flash\STM32F10x_128.FLM))'
$common.OutputDirectory = '.\Objects\'
$common.ListingPath = '.\Objects\'
$cads = $target.TargetOption.TargetArmAds.Cads
$cads.VariousControls.IncludePath = '../Core/Inc;../Middlewares/U8g2;../Drivers/STM32F1xx_HAL_Driver/Inc;../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy;../Drivers/CMSIS/Device/ST/STM32F1xx/Include;../Drivers/CMSIS/Include'
$cads.VariousControls.Define = 'USE_HAL_DRIVER,STM32F103xB'
$cads.VariousControls.MiscControls = '--no_multibyte_chars'
$cads.uC99 = '1'

# Normalize paths emitted by CubeMX before relocating the generated framework.
$seen = @{}
foreach ($group in @($target.Groups.Group)) {
    foreach ($file in @($group.SelectNodes('Files/File'))) {
        $value = $file.FilePath
        if ($value -match '/Core/(Src|Inc)/(.+)$') { $file.FilePath = '..\Core\' + $Matches[1] + '\' + $Matches[2] }
        if ($seen.ContainsKey($file.FilePath)) { $null = $group.Files.RemoveChild($file) }
        else { $seen[$file.FilePath] = $true }
    }
    if ($group.GroupName -like 'Application/User/E:*') { $null = $target.Groups.RemoveChild($group) }
}

foreach ($groupName in @('OLED application', 'U8g2 (SSD1306 only)')) {
    $old = $target.Groups.Group | Where-Object GroupName -eq $groupName
    if ($null -ne $old) { $null = $target.Groups.RemoveChild($old) }
    $group = $uv.CreateElement('Group')
    $name = $uv.CreateElement('GroupName'); $name.InnerText = $groupName; $null = $group.AppendChild($name)
    $files = $uv.CreateElement('Files'); $null = $group.AppendChild($files)
    if ($groupName -eq 'OLED application') {
        $sources = @('oled_port.c','oled_demo.c','lab6_font.c','u8g2_font_6x12_tf.c') | ForEach-Object { Get-Item -LiteralPath (Join-Path $projectRoot "Core\Src\$_") }
    } else { $sources = Get-ChildItem -LiteralPath $u8g2Root -Filter '*.c' | Sort-Object Name }
    foreach ($source in $sources) {
        $fileNode = $uv.CreateElement('File')
        $relative = [IO.Path]::GetRelativePath((Join-Path $projectRoot 'MDK-ARM'), $source.FullName)
        foreach ($item in @(@('FileName',$source.Name), @('FileType','1'), @('FilePath',$relative))) {
            $node = $uv.CreateElement($item[0]); $node.InnerText = $item[1]; $null = $fileNode.AppendChild($node)
        }
        $null = $files.AppendChild($fileNode)
    }
    $null = $target.Groups.AppendChild($group)
}
$settings = [Xml.XmlWriterSettings]::new()
$settings.Indent = $true
$settings.Encoding = $utf8
$writer = [Xml.XmlWriter]::Create($uvFile, $settings)
$uv.Save($writer)
$writer.Close()
Write-Output "Prepared project; Chinese glyphs: $($codes.Count); font map: $map"

foreach ($file in Get-ChildItem -LiteralPath $u8g2Root -File | Where-Object Extension -in '.c','.h') {
    $text = [IO.File]::ReadAllText($file.FullName)
    if (-not $text.EndsWith("`n")) { [IO.File]::WriteAllText($file.FullName, $text + "`n", $utf8) }
}
