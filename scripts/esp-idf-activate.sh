#!/usr/bin/env bash
# esp-idf-activate.sh - Cross-platform ESP-IDF environment activation
# Usage:
#   source esp-idf-activate.sh              # Activate only
#   esp-idf-activate.sh build               # Activate + run idf.py command
#   esp-idf-activate.sh -p COM8 flash       # Activate + idf.py with args
#
# Config: Looks for idf-env.sh, idf-env.ps1, or idf-env.bat in project root
# Falls back to $HOME/.esp-idf-build.* or $ESP_IDF_BUILD_CONFIG env var.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Detect platform
detect_platform() {
    case "$(uname -s)" in
        CYGWIN*|MINGW*|MSYS*) echo "windows" ;;
        Darwin*) echo "macos" ;;
        Linux*) echo "linux" ;;
        *) echo "unknown" ;;
    esac
}

PLATFORM=$(detect_platform)

# Find config file
find_config() {
    if [ -n "$ESP_IDF_BUILD_CONFIG" ] && [ -f "$ESP_IDF_BUILD_CONFIG" ]; then
        echo "$ESP_IDF_BUILD_CONFIG"
        return
    fi
    if [ -f "$SCRIPT_DIR/idf-env.sh" ]; then
        echo "$SCRIPT_DIR/idf-env.sh"
        return
    fi
    if [ "$PLATFORM" = "windows" ]; then
        if [ -f "$SCRIPT_DIR/idf-env.ps1" ]; then
            echo "$SCRIPT_DIR/idf-env.ps1"
            return
        fi
        if [ -f "$SCRIPT_DIR/idf-env.bat" ]; then
            echo "$SCRIPT_DIR/idf-env.bat"
            return
        fi
        if [ -f "$HOME/.esp-idf-build.ps1" ]; then
            echo "$HOME/.esp-idf-build.ps1"
            return
        fi
    else
        if [ -f "$HOME/.esp-idf-build.sh" ]; then
            echo "$HOME/.esp-idf-build.sh"
            return
        fi
    fi
    echo ""
}

# Parse config file for IDF_PATH
parse_idf_path() {
    local config="$1"
    local path=""

    case "$config" in
        *.ps1)
            # Extract $IDF_PATH = "..." from PowerShell
            path=$(grep -E '^\$IDF_PATH\s*=' "$config" | sed 's/.*"\(.*\)".*/\1/' | tr -d '"')
            ;;
        *.bat)
            # Extract set "IDF_PATH=..." from batch
            path=$(grep -E '^set.*IDF_PATH=' "$config" | sed 's/.*="\([^"]*\)".*/\1/')
            ;;
        *.sh)
            # Source the shell config
            . "$config"
            path="$IDF_PATH"
            ;;
    esac

    echo "$path"
}

CONFIG_FILE=$(find_config)

if [ -z "$CONFIG_FILE" ]; then
    echo "ERROR: No ESP-IDF config found."
    echo "Create idf-env.sh (or idf-env.ps1/idf-env.bat) in project root."
    echo "Example idf-env.sh:"
    echo '  export IDF_PATH="/path/to/esp-idf"'
    exit 1
fi

echo "Config: $CONFIG_FILE"

IDF_PATH=$(parse_idf_path "$CONFIG_FILE")

if [ -z "$IDF_PATH" ]; then
    echo "ERROR: Could not parse IDF_PATH from $CONFIG_FILE"
    exit 1
fi

# Expand Windows paths (handle D:\Tools\... format in git bash)
if [ "$PLATFORM" = "windows" ]; then
    IDF_PATH=$(echo "$IDF_PATH" | sed 's/\\/\//g' | sed 's/^\([a-zA-Z]\):/\/\1/')
fi

if [ ! -d "$IDF_PATH" ]; then
    echo "ERROR: IDF_PATH does not exist: $IDF_PATH"
    exit 1
fi

export IDF_PATH

# Activate ESP-IDF environment
case "$PLATFORM" in
    windows)
        # In git bash, try to call PowerShell for activation
        echo "ESP-IDF path: $IDF_PATH"
        echo "Note: For full Windows activation (EIM, Python venv), use:"
        echo "  powershell -NoProfile -Command \". '$SCRIPT_DIR/build_esp32.ps1' -ActivateOnly; idf.py <args>\""
        ;;
    macos|linux)
        . "$IDF_PATH/export.sh"
        echo "ESP-IDF activated: $IDF_PATH"
        ;;
esac

# If arguments provided, run idf.py with them
if [ $# -gt 0 ]; then
    idf.py "$@"
fi
