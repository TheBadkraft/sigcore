#!/bin/bash
# Bundle script for creating fat objects from source lists
# Usage: ./bundle.sh <bundle_name>
# e.g., ./bundle.sh collection

set -e

# Source configuration
source config.sh

# Check argument
if [ $# -ne 1 ]; then
    echo "Usage: $0 <bundle_name>"
    echo "Bundles objects listed in \${BUNDLE_NAME}_SOURCES from config.sh"
    exit 1
fi

bundle_name=$1
sources_var="${bundle_name^^}_SOURCES"  # Uppercase, e.g., COLLECTION_SOURCES

# Get the sources list
sources_value="${!sources_var}"
if [ -z "$sources_value" ]; then
    echo "No sources defined for bundle '$bundle_name' (variable $sources_var not set in config.sh)"
    exit 1
fi

sources=($sources_value)
bundle_objects=()
for name in "${sources[@]}"; do
    obj="$BUILD_DIR/$name.o"
    if [ -f "$obj" ]; then
        bundle_objects+=("$obj")
    else
        echo "Warning: $obj not found, skipping"
    fi
done

if [ ${#bundle_objects[@]} -gt 0 ]; then
    bundle_target="$BUILD_DIR/sigma.$bundle_name.o"
    echo "Bundling $bundle_name -> $bundle_target"
    ld -r "${bundle_objects[@]}" -o "$bundle_target"
else
    echo "No objects found to bundle for $bundle_name"
fi