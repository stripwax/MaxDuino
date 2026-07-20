#!/bin/bash

set -e

# Usage: ./maxduinobatch.sh [-p <path>] <input_dir> <output_dir> [<match_dir>]
if [ $# -lt 1 ]; then
    echo "Usage: $0 [-p <maxduino_path>] <input_dir> <output_dir> [<match_dir>]" >&2
    exit 1
fi

MAXDUINO=""
INPUT_DIR=""
OUTPUT_DIR=""
MATCH_DIR=""

while getopts ":p:i:o:m:" opt; do
    case $opt in
        p) MAXDUINO="$OPTARG" ;;
        i) INPUT_DIR="$OPTARG" ;;
        o) OUTPUT_DIR="$OPTARG" ;;
        m) MATCH_DIR="$OPTARG" ;;
        \?) echo "Invalid option: -$OPTARG" >&2; exit 1 ;;
    esac
done

# Shift past --parsed args and positional args
shift $((OPTIND - 1))

if [ $# -lt 2 ]; then
    echo "Usage: $0 [-p <maxduino_path>] <input_dir> <output_dir> [<match_dir>]" >&2
    exit 1
fi

INPUT_DIR="$1"
OUTPUT_DIR="$2"
MATCH_DIR="${3:-}"

# Default to 'maxduino' if not specified
[ -z "$MAXDUINO" ] && MAXDUINO="maxduino"

mismatches=0
done_count=0

# Count total files
total=0
for _ in "$INPUT_DIR"/*; do
    total=$((total + 1))
done

# Process all files in input directory
for input_file in "$INPUT_DIR"/*; do
    if [ ! -f "$input_file" ]; then
        continue
    fi
    
    filename=$(basename "$input_file")
    output_file="$OUTPUT_DIR/$filename.wav"
    
    done_count=$((done_count + 1))
    
    # Run maxduino (count failures as mismatches)
    if ! "$MAXDUINO" -i "$input_file" -o "$output_file"; then
        echo "Mismatch: $filename.wav (maxduino failed)" >&2
        mismatches=$((mismatches + 1))
        continue
    fi
    
    # Compare if match_dir specified and golden file exists
    if [ -n "$MATCH_DIR" ] && [ -f "$MATCH_DIR/$filename.wav" ]; then
        if ! cmp -s "$output_file" "$MATCH_DIR/$filename.wav"; then
            echo "Mismatch: $filename.wav" >&2
            mismatches=$((mismatches + 1))
        fi
    fi
    
    # Progress
    echo "${done_count}/${total} test cases processed" >&2
done

if [ $mismatches -gt 0 ]; then
    echo "Total mismatches: $mismatches" >&2
    exit 1
fi

exit 0