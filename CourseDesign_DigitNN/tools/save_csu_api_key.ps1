$ErrorActionPreference = "Stop"

$root = Resolve-Path (Join-Path $PSScriptRoot "..")
$envPath = Join-Path $root ".env.local"

Write-Host "DigitNN CSU API key setup" -ForegroundColor Cyan
Write-Host "Token name: fa_8202240417"
Write-Host "Base URL:   https://api.chat.csu.edu.cn/v1"
Write-Host ""

$secure = Read-Host "Enter API key, input is hidden" -AsSecureString
$bstr = [Runtime.InteropServices.Marshal]::SecureStringToBSTR($secure)
try {
    $plain = [Runtime.InteropServices.Marshal]::PtrToStringBSTR($bstr)
} finally {
    [Runtime.InteropServices.Marshal]::ZeroFreeBSTR($bstr)
}

if ([string]::IsNullOrWhiteSpace($plain)) {
    throw "API key is empty. Nothing was saved."
}

$model = Read-Host "Vision model name [qwen-vl-plus]"
if ([string]::IsNullOrWhiteSpace($model)) {
    $model = "qwen-vl-plus"
}

$lines = @(
    "CSU_API_TOKEN_NAME=fa_8202240417",
    "CSU_API_BASE_URL=https://api.chat.csu.edu.cn/v1",
    "CSU_API_MODEL=$model",
    "CSU_API_KEY=$plain"
)

[System.IO.File]::WriteAllLines($envPath, $lines, [System.Text.UTF8Encoding]::new($false))
Write-Host ""
Write-Host "Saved to $envPath" -ForegroundColor Green
Write-Host "This file is ignored by git. Return to the dashboard and click Refresh."
