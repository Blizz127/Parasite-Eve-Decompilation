#!/usr/bin/env bash
# One-time helper: detect whether the GitHub wiki git repo exists, print seed
# instructions if not, or clone + dry-run sync if yes.
#
# Does NOT push. Does NOT create the first page (GitHub requires a browser once).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WIKI_SRC="$ROOT/docs/wiki"
DEFAULT_CHECKOUT="$(dirname "$ROOT")/Parasite-Eve-Decompilation.wiki"
WIKI_CHECKOUT="${1:-$DEFAULT_CHECKOUT}"
REMOTE="https://github.com/Blizz127/Parasite-Eve-Decompilation.wiki.git"
WIKI_WEB="https://github.com/Blizz127/Parasite-Eve-Decompilation/wiki"

die() { echo "ERROR: $*" >&2; exit 1; }
info() { echo "  $*"; }

[[ -d "$WIKI_SRC" ]] || die "missing $WIKI_SRC — wiki source not in this checkout"
[[ -f "$WIKI_SRC/Home.md" ]] || die "missing $WIKI_SRC/Home.md"

echo "=== GitHub Wiki seed / status ==="
echo "  source:   $WIKI_SRC"
echo "  checkout: $WIKI_CHECKOUT"
echo "  remote:   $REMOTE"
echo

if git ls-remote "$REMOTE" >/dev/null 2>&1; then
    info "OK: wiki git remote exists (at least one page was created on GitHub)."
    if [[ ! -d "$WIKI_CHECKOUT/.git" ]]; then
        echo
        echo "Clone with:"
        echo "  git clone $REMOTE $WIKI_CHECKOUT"
        exit 0
    fi
    if [[ "$(basename "$WIKI_CHECKOUT")" != "Parasite-Eve-Decompilation.wiki" ]]; then
        die "checkout basename must be Parasite-Eve-Decompilation.wiki (got $(basename "$WIKI_CHECKOUT"))"
    fi
    echo
    echo "Preview (no write):"
    "$ROOT/scripts/preview_wiki_sync.sh" "$WIKI_CHECKOUT"
    echo
    echo "To publish (manual):"
    echo "  rsync -av --exclude README.md $WIKI_SRC/*.md $WIKI_CHECKOUT/"
    echo "  cd $WIKI_CHECKOUT && git add *.md && git status"
    echo "  git commit -m 'Update project wiki mirror' && git push"
    exit 0
fi

echo "Wiki git remote is NOT available yet."
echo "GitHub only creates *.wiki.git after the first page exists in the browser."
echo
echo "One-time seed steps:"
echo "  1. Open:  $WIKI_WEB"
echo "  2. Click: Create the first page"
echo "  3. Title: Home"
echo "  4. Paste contents of: $WIKI_SRC/Home.md"
echo "  5. Save the page"
echo "  6. Re-run:  scripts/seed_github_wiki.sh"
echo
echo "After seed, clone + rsync from docs/wiki/ (see docs/wiki/README.md)."
exit 1
