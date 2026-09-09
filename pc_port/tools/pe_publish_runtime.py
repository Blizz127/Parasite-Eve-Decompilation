#!/usr/bin/env python3
"""Publish a packaged Parasite Eve runtime to the private Banshee R2 channel.

Recreates the PACE1 publish flow (its original copy lived in /tmp and was
lost to a reboot). Steps, all verified before the channel moves:

  1. immutable archive upload to  <prefix>/builds/<BUILD>/<package>
     (an existing object with different bytes is a hard error)
  2. full remote SHA-256 readback == local
  3. signed 7-day package URL + ranged GET 0-1023 must answer 206
  4. pinned channel doc <prefix>/channels/dev-<BUILD>.json and the mutable
     <prefix>/channels/dev.json (previous dev.json saved locally first)
  5. readback of both channel docs == what was written
  6. signed 7-day channel URL minted into <stage>/channel-url.txt

Signed URLs are written only to private files under the stage directory
(mode 0600) and are never printed or logged.

This updates the private development channel ONLY. The launcher's default
channel is channels/parasite-eve.json on its separate public content origin.
To deliver a requested launcher update, also run the launcher repository's
scripts/publish-parasite-eve-channel.sh with the SAME build ID and
PE_LOCAL_PACKAGE_DIR pointing at this repository's pc_port/build-dist/dist.
Verify that default channel and both package downloads before reporting the
launcher update published; a successful private dev.json upload is insufficient.

Usage:
  PE_BUILD_ID=PE-XXX-<sha12> PE_RELEASE_NOTES="..." \
  python3 pc_port/tools/pe_publish_runtime.py [--stage DIR] [--dry-run]
"""
import datetime
import hashlib
import json
import os
import subprocess
import sys
import urllib.request
from pathlib import Path

os.umask(0o077)
ROOT = Path(__file__).resolve().parents[2]
REMOTE = "banshee-r2:banshee-preservation-private/"
PREFIX = "games/parasite-eve"
APP_ID = "banshee.parasite-eve"


def run(args, **kw):
    return subprocess.run(args, check=True, **kw)


def output(args):
    return subprocess.check_output(args, text=True).strip()


def sha_file(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        while chunk := f.read(1 << 20):
            h.update(chunk)
    return h.hexdigest()


def remote_sha(path):
    p = subprocess.Popen(["rclone", "cat", path], stdout=subprocess.PIPE)
    h = hashlib.sha256()
    while chunk := p.stdout.read(1 << 20):
        h.update(chunk)
    if p.wait():
        raise RuntimeError("remote read failed for " + path)
    return h.hexdigest()


def remote_listing(dirpath):
    try:
        return json.loads(output(["rclone", "lsjson", "--files-only", dirpath]))
    except subprocess.CalledProcessError:
        return []


def copy(src, dst):
    run(["rclone", "copyto", str(src), dst, "--s3-no-check-bucket"], stdout=subprocess.DEVNULL)


def main():
    args = sys.argv[1:]
    stage = ROOT / "local" / "live" / "publish"
    dry = False
    while args:
        if args[0] == "--stage":
            stage = Path(args[1]); args = args[2:]
        elif args[0] == "--dry-run":
            dry = True; args = args[1:]
        else:
            raise SystemExit("unknown arg " + args[0])
    stage.mkdir(parents=True, exist_ok=True)
    build = os.environ["PE_BUILD_ID"]
    notes = os.environ.get("PE_RELEASE_NOTES", "")
    binaries = {"linux-x64": ROOT / "pc_port" / "build-dist" / "parasite-eve-port",
                "win-x64": ROOT / "pc_port" / "build-win" / "parasite-eve-port.exe"}
    exec_paths = {"linux-x64": "parasite-eve-port", "win-x64": "parasite-eve-port.cmd"}
    dist = ROOT / "pc_port" / "build-dist" / "dist"
    packages = {plat: dist / f"parasite-eve-{build}-{plat}.tar.zst" for plat in binaries}
    packages = {plat: pkg for plat, pkg in packages.items() if pkg.exists()}
    if not packages:
        raise SystemExit("no packages for %s under %s" % (build, dist))
    source_commit = output(["git", "-C", str(ROOT), "rev-parse", "--short", "HEAD"])
    dirty = bool(output(["git", "-C", str(ROOT), "status", "--porcelain", "--untracked-files=no"]))
    log = open(stage / "publish.log", "a")

    def say(s):
        print(s, flush=True); log.write(s + "\n"); log.flush()

    entries = {}
    summary_pkgs = {}
    for plat, package in packages.items():
        key = f"{PREFIX}/builds/{build}/{package.name}"
        size = package.stat().st_size
        sha = sha_file(package)
        meta = json.loads(output(["tar", "--zstd", "-xOf", str(package), "runtime/build-info.json"]))
        if meta["buildId"] != build or meta.get("platform") != plat:
            raise SystemExit("package metadata mismatch for %s" % package.name)
        binary_sha = sha_file(binaries[plat])
        if meta.get("binarySha256") != binary_sha:
            raise SystemExit("package binary hash does not match %s" % binaries[plat])
        source_commit = meta.get("sourceCommit", source_commit)
        say(f"== {build} {plat} package {size} B sha256 {sha} binary {binary_sha} commit {source_commit} dirty={dirty}")
        if dry:
            continue
        # 1. immutable archive
        existing = [e for e in remote_listing(REMOTE + f"{PREFIX}/builds/{build}") if e["Path"] == package.name]
        if existing:
            if remote_sha(REMOTE + key) != sha:
                raise SystemExit("IMMUTABLE ARCHIVE COLLISION: remote bytes differ for " + key)
            say("archive already present with identical bytes")
        else:
            say("uploading archive " + package.name); copy(package, REMOTE + key)
        # 2. readback
        remote_size = json.loads(output(["rclone", "size", "--json", REMOTE + key]))["bytes"]
        if remote_size != size:
            raise SystemExit("remote size mismatch")
        say("verifying full remote sha256")
        if remote_sha(REMOTE + key) != sha:
            raise SystemExit("remote sha256 mismatch")
        # 3. signed package url + ranged GET
        package_url = output(["rclone", "link", REMOTE + key, "--expire", "168h"])
        req = urllib.request.Request(package_url, headers={"Range": "bytes=0-1023"})
        with urllib.request.urlopen(req, timeout=60) as resp:
            if resp.status != 206 or len(resp.read()) != 1024:
                raise SystemExit("ranged GET on the signed package URL failed")
        say("signed package URL: ranged GET 206 ok (url kept private)")
        entries[plat] = {
            "objectKey": key, "downloadUrl": package_url, "packageSize": size, "sha256": sha,
            "archiveFormat": "tar.zst", "executablePath": exec_paths[plat],
            "binarySha256": binary_sha,
        }
        summary_pkgs[plat] = {"objectKey": key, "packageSize": size, "sha256": sha, "binarySha256": binary_sha}
    if dry:
        say("dry run: no upload"); return
    # 4. channel docs
    prev = None
    try:
        prev = output(["rclone", "cat", REMOTE + f"{PREFIX}/channels/dev.json"])
        (stage / "previous-channel.json").write_text(prev)
        prev_id = json.loads(prev).get("buildId")
    except subprocess.CalledProcessError:
        prev_id = None
    doc = {
        "schemaVersion": 1,
        "appId": APP_ID,
        "buildId": build,
        "sourceCommit": source_commit,
        "sourceDirty": dirty,
        "publishedAt": datetime.datetime.now(datetime.timezone.utc).isoformat().replace("+00:00", "Z"),
        "runtimeLayoutVersion": 1,
        "releaseNotes": notes,
        "platforms": entries,
    }
    text = json.dumps(doc, indent=2) + "\n"
    local_doc = stage / "dev.json"
    local_doc.write_text(text)
    pinned = f"{PREFIX}/channels/dev-{build}.json"
    say("publishing pinned channel doc, then mutable dev.json")
    copy(local_doc, REMOTE + pinned)
    copy(local_doc, REMOTE + f"{PREFIX}/channels/dev.json")
    # 5. readback
    if output(["rclone", "cat", REMOTE + pinned]).strip() != text.strip():
        raise SystemExit("pinned channel readback mismatch")
    if output(["rclone", "cat", REMOTE + f"{PREFIX}/channels/dev.json"]).strip() != text.strip():
        raise SystemExit("dev.json readback mismatch")
    say("channel readback ok")
    # 6. signed channel url (private file only)
    channel_url = output(["rclone", "link", REMOTE + f"{PREFIX}/channels/dev.json", "--expire", "168h"])
    (stage / "channel-url.txt").write_text(channel_url + "\n")
    with urllib.request.urlopen(channel_url, timeout=60) as resp:
        if json.loads(resp.read().decode()).get("buildId") != build:
            raise SystemExit("signed channel URL does not serve the new build")
    summary = {
        "buildId": build, "previousBuildId": prev_id, "sourceCommit": source_commit,
        "sourceDirty": dirty, "platforms": summary_pkgs, "channel": f"{PREFIX}/channels/dev.json",
        "archiveReadbackSha256Verified": True, "signedPackageRangeVerified": True,
        "pinnedAndMutableChannelVerified": True, "signedChannelVerified": True,
    }
    (stage / "published-summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    say(json.dumps(summary))
    say("Private development channel published. The launcher default channel still requires "
        "scripts/publish-parasite-eve-channel.sh " + build + " in the launcher repository.")


if __name__ == "__main__":
    main()
