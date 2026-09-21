# Parasite Eve agent/IDE settings — consolidated inventory (2026-09-21)

Scope: the durable *configuration* used for Parasite Eve work across the machines
that hold it. Session transcripts and terminal state are deliberately **out of
scope** (see "Left alone" below).

## Machines surveyed

| Machine | Reached | Project path | Git state | Agent stores |
| --- | --- | --- | --- | --- |
| here (this box) | — | `/home/blizz/dev/Parasite-Eve-Decompilation` | `main` @ `040f8fc3` (ours) | grok 29M, deepcode 28M, cursor 25M, claude 856K |
| `vps-b1952d16` (100.104.53.1) | ssh ✓ | `~/dev/parasite-eve` (+ `parasite-eve-port-black`, `-worktrees`, `-scratch`) | **stale**: `main` @ `2cd7293a` | grok 2.6G, claude 3.9G, deepcode 66M, cursor 28M |
| `alienware-bazzite` | ssh ✓ | `~/dev/Parasite-Eve-Decompilation` | `cursor/cd-sector-backpressure-6f51` @ `21a24a82` | grok 1.8G, claude 515M, deepcode 150M, cursor 1.2G |
| `matts-macbook` (macOS) | **offline**, last seen 12h | unknown | unknown | not read |
| `macserver` | ssh refused (publickey,password) | unknown | unknown | not read |
| `github.com` | ssh ✓ | remote of record, 39 branches | `main` @ `040f8fc3` | — |

## The rule file is the same file, and it is already in git

`CLAUDE.md` is **tracked** in the repo, so it travels with `git push`/`pull` and
does not need a separate merge:

| Machine | Size | Date | Verdict |
| --- | --- | --- | --- |
| here | **48,047 B** | 2026-09-12 | canonical |
| vps | 8,801 B | 2026-08-25 | stale |
| alienware | 8,801 B | 2026-09-09 | stale |

The two stale copies are byte-identical to each other. The 48 KB copy here is a
**heading superset** of them, and at line level the only things it lacks are six
lines of *old status prose* (a stale leaf count, old lane names) — no rules. So the
consolidation action is simply **`git pull` on the other machines**; the canonical
rules went out in commit `040f8fc3`.

## `~/.claude/settings.json` — portable preferences merged

The three copies genuinely differ. Merged union:

`configs/agent/claude-settings.consolidated.json` (11 keys, generated from the union,
preferring the richest copy per key).

| key | here | vps | alienware | merged value |
| --- | :-: | :-: | :-: | --- |
| `effortLevel` | – | ✓ | ✓ | `xhigh` |
| `modelSettings` | – | ✓ | ✓ | per-model effort map |
| `statusLine` | – | ✓ | ✓ | Orca statusline shim |
| `theme` | ✓ | ✓ | ✓ | `auto` |
| `tui` | – | ✓ | ✓ | `fullscreen` |
| `agentPushNotifEnabled` | – | ✓ | ✓ | `true` |
| `skipDangerousModePermissionPrompt` | – | ✓ | ✓ | `true` |
| `enabledPlugins` | – | – | ✓ | `clangd-lsp@claude-plugins-official` |
| `extraKnownMarketplaces` | – | – | ✓ | `addy-agent-skills` |
| `env` | – | – | ✓ | `API_TIMEOUT_MS=3000000`, `CLAUDE_CODE_DISABLE_NONESSENTIAL_TRAFFIC=1` |
| `sandbox` | – | – | ✓ | enabled, excludes `orca-ide`, allows all unix sockets |

**This box is the bare one** — it only had `theme` (+ `hooks`). Everything else in
the table came from the vps or alienware.

## Deliberately NOT merged

- **`hooks`** — these are Orca IDE plumbing (`ORCA_AGENT_HOOK_PORT` / `_TOKEN` /
  `ORCA_PANE_KEY`) injected per machine, and the *keys differ* per machine (`here`
  has `SessionStart`; vps and alienware have `UserPromptSubmit`). Merging them would
  invent behaviour the IDE did not configure.
- **`remote`** (`defaultEnvironmentId: env_01WsJsnULr4wYsacaakWhcyw`) — a vps cloud
  environment binding, meaningless elsewhere.
- **`autoMode` (vps only)** — **this is not a Parasite Eve setting.** Its content is
  the security/trust-boundary preamble for a different repository
  (`Blizz127/DIT-Human-Behavior-Platform`), including that project's org
  description, sensitive-data locations and deployment rules. It is recorded here so
  nobody re-merges it by accident; it belongs with the DIT project.

## How to apply

```sh
# 1. rules (canonical, via git)
git pull                      # brings the 48 KB CLAUDE.md

# 2. portable preferences
python3 - <<'PY'
import json, pathlib
src = json.load(open("configs/agent/claude-settings.consolidated.json"))
p = pathlib.Path.home()/".claude/settings.json"
cur = json.load(open(p)) if p.exists() else {}
keep = {k: v for k, v in cur.items() if k in ("hooks", "remote", "autoMode")}
cur.update(src); cur.update(keep)          # never clobber machine-local keys
json.dump(cur, open(p, "w"), indent=2)
PY
```

## Left alone on purpose

Session/terminal stores were **not** copied or merged: `~/.grok/sessions/<project>`,
`~/.deepcode/projects/<project>`, `~/.cursor/projects/<project>`, `~/.claude/projects/*`,
`~/.cursor/agent-transcripts`, orca terminal history. They are per-machine
transcripts in three incompatible formats; combining them would produce a corpus no
tool can read. On request they can be archived read-only instead.

## Still open

- **`matts-macbook` is offline** and `macserver` refuses this key, so the Mac's
  copy of these files has not been read. To finish the survey: bring the Mac onto
  the tailnet, or add a host alias with a key that works, then re-run the table.
- The vps checkout `~/dev/parasite-eve` is a **stale clone on the old main**; it
  needs `git pull` before it can be treated as current. The alienware box is on
  `cursor/cd-sector-backpressure-6f51`, i.e. it holds branch work not yet in main —
  that is the branch-consolidation track, not the settings track.
