$ErrorActionPreference = "Stop"

$root = Resolve-Path (Join-Path $PSScriptRoot "..")
$envPath = Join-Path $root ".env.local"

Write-Host "DigitNN Alibaba Cloud DashScope API setup" -ForegroundColor Cyan
Write-Host "Use the API Key from Bailian / DashScope API-Key page." -ForegroundColor Yellow
Write-Host "Do NOT enter AccessKey ID or AccessKey Secret here." -ForegroundColor Yellow
Write-Host "Default OpenAI-compatible BASE_URL: https://dashscope.aliyuncs.com/compatible-mode/v1"
Write-Host "If your Model Studio workspace requires a WorkspaceId URL, paste it when prompted."
Write-Host ""

while ($true) {
    $secure = Read-Host "Enter DashScope API Key, input is hidden" -AsSecureString
    $bstr = [Runtime.InteropServices.Marshal]::SecureStringToBSTR($secure)
    try {
        $plain = [Runtime.InteropServices.Marshal]::PtrToStringBSTR($bstr)
    } finally {
        [Runtime.InteropServices.Marshal]::ZeroFreeBSTR($bstr)
    }

    if ([string]::IsNullOrWhiteSpace($plain)) {
        Write-Host "API key is empty. Please try again." -ForegroundColor Red
        continue
    }
    if ($plain.StartsWith("LTA")) {
        Write-Host "This looks like an AccessKey ID, not a DashScope API Key. Please use Bailian / DashScope API-Key." -ForegroundColor Red
        continue
    }
    if (-not $plain.StartsWith("sk-")) {
        Write-Host "Warning: DashScope API keys usually start with sk-." -ForegroundColor Yellow
        $confirm = Read-Host "Save this value anyway? [y/N]"
        if ($confirm -notin @("y", "Y", "yes", "YES")) {
            continue
        }
    }
    break
}

$baseUrl = Read-Host "BASE_URL [https://dashscope.aliyuncs.com/compatible-mode/v1]"
if ([string]::IsNullOrWhiteSpace($baseUrl)) {
    $baseUrl = "https://dashscope.aliyuncs.com/compatible-mode/v1"
}

$model = Read-Host "Vision model name [qwen-vl-max]"
if ([string]::IsNullOrWhiteSpace($model)) {
    $model = "qwen-vl-max"
}

$existing = @{}
if (Test-Path $envPath) {
    foreach ($line in [System.IO.File]::ReadAllLines($envPath)) {
        if ($line.Trim().Length -eq 0 -or $line.Trim().StartsWith("#") -or -not $line.Contains("=")) {
            continue
        }
        $parts = $line.Split("=", 2)
        $existing[$parts[0].Trim()] = $parts[1].Trim()
    }
}

$existing["CHINESE_API_PROVIDER"] = "aliyun"
$existing["ALIYUN_API_BASE_URL"] = $baseUrl
$existing["ALIYUN_API_MODEL"] = $model
$existing["ALIYUN_API_KEY"] = $plain
$existing["DASHSCOPE_API_KEY"] = $plain

$orderedKeys = @(
    "CHINESE_API_PROVIDER",
    "CSU_API_TOKEN_NAME",
    "CSU_API_BASE_URL",
    "CSU_API_MODEL",
    "CSU_API_KEY",
    "ALIYUN_API_BASE_URL",
    "ALIYUN_API_MODEL",
    "ALIYUN_API_KEY",
    "DASHSCOPE_API_KEY"
)

$lines = New-Object System.Collections.Generic.List[string]
foreach ($key in $orderedKeys) {
    if ($existing.ContainsKey($key)) {
        $lines.Add("$key=$($existing[$key])")
        $existing.Remove($key)
    }
}
foreach ($key in ($existing.Keys | Sort-Object)) {
    $lines.Add("$key=$($existing[$key])")
}

[System.IO.File]::WriteAllLines($envPath, $lines, [System.Text.UTF8Encoding]::new($false))
Write-Host ""
Write-Host "Saved to $envPath" -ForegroundColor Green
Write-Host "Provider switched to aliyun. Return to the dashboard and click Refresh."
