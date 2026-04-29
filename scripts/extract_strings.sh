#!/usr/bin/env bash
# Extract strings from Sober binaries
# Usage: ./extract_strings.sh <binary_dir> <output_dir>

set -e

BINARY_DIR="${1:-.}"
OUTPUT_DIR="${2:-../analysis}"

mkdir -p "$OUTPUT_DIR"

for binary in sober sober_services libloader.so libbadcpu.so; do
    if [ -f "$BINARY_DIR/$binary" ]; then
        echo "Extracting strings from $binary..."
        strings -a -n 4 "$BINARY_DIR/$binary" | sort -u > "$OUTPUT_DIR/strings_${binary//./_}.txt"
        echo "  -> $(wc -l < "$OUTPUT_DIR/strings_${binary//./_}.txt") unique strings"
    fi
done

echo "Done."