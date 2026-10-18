#!/usr/bin/env bash
set -euo pipefail

workspace_root="$(cd "$(dirname "$0")/.." && pwd)"
board="${1:-}"

case "$board" in
    waveshare_5|waveshare_7b)
        ;;
    *)
        echo "Usage: $0 {waveshare_5|waveshare_7b}"
        exit 2
        ;;
esac

idf.py \
    -C "$workspace_root/boards/$board" \
    -B "$workspace_root/build/$board" \
    build
