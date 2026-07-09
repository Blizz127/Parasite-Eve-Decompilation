# GitHub Wiki source (mirror)

These Markdown files are a **curated, human-readable mirror** of project status.
They are **not** the source of truth.

## Source of truth (main repo)

| Topic | Authoritative path |
| --- | --- |
| Current working state | `docs/ai_context/ACTIVE_HANDOFF.md` |
| Phase roadmap | `docs/project_plan.md` |
| Disc hashes and extraction | `docs/disc_info.md` |
| Split policy | `docs/splitting.md` |
| Splat config | `configs/USA/disc1.yaml` |
| Agent rules | `CLAUDE.md` |

Update wiki pages only after **durable milestones** (merged PR, completed phase,
verified boundary discovery, build/split process change, porting-queue change).
Do **not** auto-push on every commit.

## Why the live GitHub Wiki may be empty

GitHub only creates the cloneable `*.wiki.git` repository after **at least one
wiki page** exists on the website. Until then:

```bash
git clone git@github.com:Blizz127/Parasite-Eve-Decompilation.wiki.git
# → Repository not found
```

**One-time seed:** open

https://github.com/Blizz127/Parasite-Eve-Decompilation/wiki

and click **Create the first page** (title `Home`). Paste `docs/wiki/Home.md`
content, save. After that, clone + rsync work.

## Preview sync (safe, no push)

```bash
scripts/preview_wiki_sync.sh /path/to/Parasite-Eve-Decompilation.wiki
```

Does not commit or push. Fails if the checkout basename is wrong.

## Manual publish (after seed)

```bash
rsync -av --exclude README.md docs/wiki/*.md /path/to/Parasite-Eve-Decompilation.wiki/
cd /path/to/Parasite-Eve-Decompilation.wiki
git status
git add *.md
git commit -m "Update project wiki mirror"
git push
```
