# Useful Commands

## Extract / split / verify / build

```bash
scripts/extract_us.sh 1
scripts/setup_env.sh
scripts/split_us.sh --check
scripts/split_us.sh
scripts/verify_us.sh
scripts/build_us.sh
```

## Toolchain (Distrobox)

```bash
distrobox enter pe-mipsel -- mipsel-linux-gnu-gcc --version
distrobox enter pe-mipsel -- mipsel-linux-gnu-as --version
```

## Scratch C codegen probe

```bash
# edit /tmp/probe.c then:
distrobox enter pe-mipsel -- mipsel-linux-gnu-gcc \
  -EL -mips1 -mfp32 -mabi=32 -G0 -fno-pic -mno-abicalls \
  -ffreestanding -fno-builtin -O1 \
  -c -o /tmp/probe.o /tmp/probe.c
distrobox enter pe-mipsel -- mipsel-linux-gnu-objdump -d /tmp/probe.o
```

## Git hygiene

```bash
git switch main && git pull
git switch -c phase…-next-c-leaf
# … one function …
git push -u origin HEAD
```

## Wiki sync (after first page exists)

```bash
# clone once (only works after GitHub has at least one wiki page):
git clone git@github.com:Blizz127/Parasite-Eve-Decompilation.wiki.git \
  ~/Projects/Parasite-Eve-Decompilation.wiki

# dry-run from main repo:
scripts/preview_wiki_sync.sh ~/Projects/Parasite-Eve-Decompilation.wiki

# real copy + manual commit in wiki repo:
rsync -av docs/wiki/*.md ~/Projects/Parasite-Eve-Decompilation.wiki/
# optional: skip docs/wiki/README.md (repo-local only)
cd ~/Projects/Parasite-Eve-Decompilation.wiki
git add *.md
git commit -m "Update project wiki mirror"
git push
```
