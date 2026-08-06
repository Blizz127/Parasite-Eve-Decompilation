#!/usr/bin/env bash
# Preview what would be copied from docs/wiki/ into a GitHub wiki checkout.
# Does NOT commit, push, or modify the wiki repo.
#
# Usage:
#   scripts/preview_wiki_sync.sh /path/to/Parasite-Eve-Decompilation.wiki
#   WIKI_CHECKOUT=/path/to/wiki scripts/preview_wiki_sync.sh
#
# Requires: rsync (for --dry-run listing)
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WIKI_SRC="$ROOT/docs/wiki"
WIKI_CHECKOUT="${1:-${WIKI_CHECKOUT:-}}"

if [[ -z "$WIKI_CHECKOUT" ]]; then
    echo "Usage: $0 /path/to/Parasite-Eve-Decompilation.wiki" >&2
    echo "   or: WIKI_CHECKOUT=/path/to/wiki $0" >&2
    exit 2
fi

# Resolve to absolute path for clearer output.
WIKI_CHECKOUT="$(cd "$WIKI_CHECKOUT" 2>/dev/null && pwd)" || {
    echo "ERROR: wiki checkout path does not exist: ${1:-$WIKI_CHECKOUT}" >&2
    exit 1
}

# Fail loudly if this does not look like the Parasite Eve wiki repo.
WIKI_BASENAME="$(basename "$WIKI_CHECKOUT")"
if [[ "$WIKI_BASENAME" != "Parasite-Eve-Decompilation.wiki" ]]; then
    echo "ERROR: expected wiki checkout basename Parasite-Eve-Decompilation.wiki" >&2
    echo "       got: $WIKI_BASENAME" >&2
    echo "       path: $WIKI_CHECKOUT" >&2
    exit 1
fi

if [[ ! -d "$WIKI_SRC" ]]; then
    echo "ERROR: missing wiki source directory: $WIKI_SRC" >&2
    exit 1
fi

if ! command -v rsync >/dev/null 2>&1; then
    echo "ERROR: rsync is required for preview" >&2
    exit 1
fi

mapfile -t WIKI_PAGES < <(find "$WIKI_SRC" -maxdepth 1 -type f -name '*.md' | sort)
if [[ ${#WIKI_PAGES[@]} -eq 0 ]]; then
    echo "ERROR: no *.md files found in $WIKI_SRC" >&2
    exit 1
fi

echo "Wiki sync preview (dry-run only — no writes, no commit, no push)"
echo "  source:   $WIKI_SRC"
echo "  dest:     $WIKI_CHECKOUT"
echo "  pages:    ${#WIKI_PAGES[@]}"
echo

for page in "${WIKI_PAGES[@]}"; do
    printf '  %s\n' "$(basename "$page")"
done

echo
echo "rsync --dry-run:"
rsync -av --dry-run "${WIKI_PAGES[@]}" "$WIKI_CHECKOUT/"

echo
echo "Next steps (manual, after review):"
echo "  rsync -av $WIKI_SRC/*.md $WIKI_CHECKOUT/"
echo "  cd $WIKI_CHECKOUT && git status && git add *.md && git commit && git push"
