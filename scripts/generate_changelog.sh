#!/bin/bash

# Generate CHANGELOG.md based on git history
# Usage: ./scripts/generate_changelog.sh [version] [previous_version]

set -e

VERSION=${1:-"Unreleased"}
PREV_VERSION=${2:-""}

# Output file
CHANGELOG_FILE="CHANGELOG.md"
TEMP_FILE="/tmp/changelog_temp.md"

# Get the date for the version
DATE=$(date +%Y-%m-%d)

# Function to categorize commits
categorize_commit() {
    local commit_msg="$1"
    local commit_type=""

    # Extract conventional commit type if present
    if [[ $commit_msg =~ ^(feat|feature|add)(\(.*\))?!?:.*$ ]]; then
        echo "### Added"
    elif [[ $commit_msg =~ ^(fix|bugfix)(\(.*\))?!?:.*$ ]]; then
        echo "### Fixed"
    elif [[ $commit_msg =~ ^(perf|performance)(\(.*\))?!?:.*$ ]]; then
        echo "### Changed"
    elif [[ $commit_msg =~ ^(refactor|style|docs|doc)(\(.*\))?!?:.*$ ]]; then
        echo "### Changed"
    elif [[ $commit_msg =~ ^(test|tests)(\(.*\))?!?:.*$ ]]; then
        echo "### Changed"
    elif [[ $commit_msg =~ ^(chore|ci|build)(\(.*\))?!?:.*$ ]]; then
        echo "### Changed"
    elif [[ $commit_msg =~ ^(remove|delete|rm)(\(.*\))?!?:.*$ ]]; then
        echo "### Removed"
    elif [[ $commit_msg =~ ^(break|breaking)(\(.*\))?!?:.*$ ]] || [[ $commit_msg =~ !: ]]; then
        echo "### Changed"
    else
        # Default categorization based on keywords
        if [[ $commit_msg =~ [Aa]dd|[Nn]ew|[Ii]mplement|[Cc]reate ]]; then
            echo "### Added"
        elif [[ $commit_msg =~ [Ff]ix|[Bb]ug|[Rr]epair|[Cc]orrect ]]; then
            echo "### Fixed"
        elif [[ $commit_msg =~ [Rr]emove|[Dd]elete|[Dd]rop ]]; then
            echo "### Removed"
        elif [[ $commit_msg =~ [Uu]pdate|[Cc]hange|[Mm]odify|[Rr]efactor|[Ii]mprove ]]; then
            echo "### Changed"
        else
            echo "### Changed"
        fi
    fi
}

# Function to clean commit message
clean_commit_message() {
    local msg="$1"
    # Remove conventional commit prefix
    msg=$(echo "$msg" | sed -E 's/^(feat|fix|docs|style|refactor|test|chore|perf|build|ci|revert)(\([^)]*\))?!?:\s*//')
    # Remove merge commit messages
    if [[ $msg =~ ^Merge ]]; then
        return 1
    fi
    # Capitalize first letter
    msg="$(echo "${msg:0:1}" | tr '[:lower:]' '[:upper:]')${msg:1}"
    echo "- $msg"
}

# Start building the new changelog
echo "# Changelog" > "$TEMP_FILE"
echo "" >> "$TEMP_FILE"
echo "All notable changes to this project will be documented in this file." >> "$TEMP_FILE"
echo "" >> "$TEMP_FILE"
echo "The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)," >> "$TEMP_FILE"
echo "and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html)." >> "$TEMP_FILE"
echo "" >> "$TEMP_FILE"

# Add unreleased section if this is not a release
if [ "$VERSION" = "Unreleased" ]; then
    echo "## [Unreleased]" >> "$TEMP_FILE"
    echo "" >> "$TEMP_FILE"
else
    echo "## [Unreleased]" >> "$TEMP_FILE"
    echo "" >> "$TEMP_FILE"
    echo "## [$VERSION] - $DATE" >> "$TEMP_FILE"
    echo "" >> "$TEMP_FILE"
fi

# Determine commit range
if [ -n "$PREV_VERSION" ] && git rev-parse "$PREV_VERSION" >/dev/null 2>&1; then
    COMMIT_RANGE="${PREV_VERSION}..HEAD"
    echo "Generating changelog from $PREV_VERSION to HEAD"
else
    # If no previous version, get all commits
    COMMIT_RANGE="HEAD"
    echo "Generating changelog for all commits"
fi

# Get commits and categorize them
declare -A categories
categories["### Added"]=""
categories["### Changed"]=""
categories["### Fixed"]=""
categories["### Removed"]=""

# Get commit messages
while IFS= read -r commit; do
    if [ -n "$commit" ]; then
        category=$(categorize_commit "$commit")
        cleaned_msg=$(clean_commit_message "$commit")
        if [ $? -eq 0 ] && [ -n "$cleaned_msg" ]; then
            if [ -n "${categories[$category]}" ]; then
                categories[$category]="${categories[$category]}\n$cleaned_msg"
            else
                categories[$category]="$cleaned_msg"
            fi
        fi
    fi
done < <(git log --format=%s $COMMIT_RANGE 2>/dev/null || echo "")

# Add categories to changelog if they have content
for category in "### Added" "### Changed" "### Fixed" "### Removed"; do
    if [ -n "${categories[$category]}" ]; then
        echo "$category" >> "$TEMP_FILE"
        echo "" >> "$TEMP_FILE"
        echo -e "${categories[$category]}" >> "$TEMP_FILE"
        echo "" >> "$TEMP_FILE"
    fi
done

# Add existing changelog content if it exists (for versions other than current)
if [ -f "$CHANGELOG_FILE" ] && [ "$VERSION" != "Unreleased" ]; then
    # Skip the header and add existing content
    tail -n +8 "$CHANGELOG_FILE" | grep -A 10000 "^## \[" >> "$TEMP_FILE" 2>/dev/null || true
fi

# Add footer
echo "" >> "$TEMP_FILE"
echo "---" >> "$TEMP_FILE"
echo "" >> "$TEMP_FILE"
echo "*This changelog is automatically updated during releases.*" >> "$TEMP_FILE"

# Replace the original file
mv "$TEMP_FILE" "$CHANGELOG_FILE"

echo "Changelog updated: $CHANGELOG_FILE"

# If this is a version release, also create a release notes file
if [ "$VERSION" != "Unreleased" ]; then
    RELEASE_NOTES_FILE="release_notes_${VERSION}.md"
    echo "# vv-dsp Release $VERSION" > "$RELEASE_NOTES_FILE"
    echo "" >> "$RELEASE_NOTES_FILE"

    # Extract just this version's changes
    sed -n "/^## \[$VERSION\]/,/^## \[/p" "$CHANGELOG_FILE" | head -n -1 | tail -n +3 >> "$RELEASE_NOTES_FILE"

    echo "" >> "$RELEASE_NOTES_FILE"
    echo "## Installation Methods" >> "$RELEASE_NOTES_FILE"
    echo "" >> "$RELEASE_NOTES_FILE"
    echo "### vcpkg" >> "$RELEASE_NOTES_FILE"
    echo "\`\`\`bash" >> "$RELEASE_NOTES_FILE"
    echo "vcpkg install vv-dsp" >> "$RELEASE_NOTES_FILE"
    echo "\`\`\`" >> "$RELEASE_NOTES_FILE"
    echo "" >> "$RELEASE_NOTES_FILE"
    echo "### CMake FetchContent" >> "$RELEASE_NOTES_FILE"
    echo "\`\`\`cmake" >> "$RELEASE_NOTES_FILE"
    echo "include(FetchContent)" >> "$RELEASE_NOTES_FILE"
    echo "FetchContent_Declare(" >> "$RELEASE_NOTES_FILE"
    echo "  vv-dsp" >> "$RELEASE_NOTES_FILE"
    echo "  GIT_REPOSITORY https://github.com/crlotwhite/vv-dsp.git" >> "$RELEASE_NOTES_FILE"
    echo "  GIT_TAG        v$VERSION" >> "$RELEASE_NOTES_FILE"
    echo ")" >> "$RELEASE_NOTES_FILE"
    echo "FetchContent_MakeAvailable(vv-dsp)" >> "$RELEASE_NOTES_FILE"
    echo "target_link_libraries(your_target PRIVATE vv-dsp)" >> "$RELEASE_NOTES_FILE"
    echo "\`\`\`" >> "$RELEASE_NOTES_FILE"
    echo "" >> "$RELEASE_NOTES_FILE"
    echo "### Pre-compiled Binaries" >> "$RELEASE_NOTES_FILE"
    echo "Download the appropriate archive for your platform from the release assets." >> "$RELEASE_NOTES_FILE"
    echo "" >> "$RELEASE_NOTES_FILE"
    echo "## Documentation" >> "$RELEASE_NOTES_FILE"
    echo "" >> "$RELEASE_NOTES_FILE"
    echo "- [Integration Guide](https://github.com/crlotwhite/vv-dsp/blob/main/docs/integration.md)" >> "$RELEASE_NOTES_FILE"
    echo "- [API Documentation](https://github.com/crlotwhite/vv-dsp/tree/main/docs)" >> "$RELEASE_NOTES_FILE"
    echo "- [Examples](https://github.com/crlotwhite/vv-dsp/tree/main/examples)" >> "$RELEASE_NOTES_FILE"

    echo "Release notes created: $RELEASE_NOTES_FILE"
fi
