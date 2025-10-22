# configure.ps1
# Usage: .\configure.ps1 [-Url <download_url>] [-OutDir <res_dir>] [-Force]
# Downloads the traderapi zip (raw GitHub URL) and extracts it into the specified res directory.

param(
    [string]$Url = "https://raw.githubusercontent.com/hanjingo/livermore-res/main/v6.7.11_20250714_traderapi.zip",
    [string]$OutDir = "${PWD}\res",
    [switch]$Force
)

function Write-Err($msg) {
    Write-Host "[ERROR] $msg" -ForegroundColor Red
}
function Write-Info($msg) {
    Write-Host "[INFO] $msg" -ForegroundColor Cyan
}

# Ensure OutDir exists
if (-not (Test-Path -Path $OutDir)) {
    New-Item -Path $OutDir -ItemType Directory -Force | Out-Null
}

$zipPath = Join-Path -Path $OutDir -ChildPath "ctp_download.zip"

if (Test-Path -Path $zipPath) {
    if ($Force) {
        Remove-Item -Path $zipPath -Force
        Write-Info "Removed existing $zipPath due to -Force"
    } else {
        Write-Info "$zipPath already exists. Use -Force to overwrite or remove it manually."
        exit 0
    }
}

Write-Info "Downloading $Url to $zipPath ..."
try {
    # Prefer Invoke-WebRequest (PowerShell), fallback to curl if available
    if (Get-Command Invoke-WebRequest -ErrorAction SilentlyContinue) {
        Invoke-WebRequest -Uri $Url -OutFile $zipPath -UseBasicParsing -ErrorAction Stop
    } elseif (Get-Command curl -ErrorAction SilentlyContinue) {
        curl -L -o $zipPath $Url
    } else {
        Write-Err "Neither Invoke-WebRequest nor curl is available. Install curl or use PowerShell 3+"
        exit 1
    }
} catch {
    Write-Err "Download failed: $_"
    if (Test-Path -Path $zipPath) { Remove-Item -Path $zipPath -Force }
    exit 1
}

Write-Info "Download complete. Extracting to $OutDir ..."
try {
    Expand-Archive -LiteralPath $zipPath -DestinationPath $OutDir -Force
} catch {
    Write-Err "Extraction failed: $_"
    if (Test-Path -Path $zipPath) { Remove-Item -Path $zipPath -Force }
    exit 1
}

# remove zip
Remove-Item -Path $zipPath -Force
Write-Info "Extraction complete. Files are in: $OutDir"
Write-Info "If you are using CMake, remove the build/ directory and re-run configuration to pick up the headers."

exit 0
