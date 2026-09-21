/*
 * test_decomp_ports_manifest.c — reproducibility regression for the generated
 * host TUs under pc_port/game/decomp/.
 *
 * Three checks, all against the checked-in authority:
 *
 *   1. every generated TU with a matching src/ leaf regenerates byte-for-byte
 *      from tools/analysis/gen_decomp_ports.py (the generator's --check);
 *   2. every generated TU without a matching src/ leaf is on the checked-in
 *      orphan allowlist (tools/analysis/decomp_port_orphans.txt), and no
 *      allowlisted name has quietly gained a src/ leaf (which would mean the
 *      allowlist is stale);
 *   3. the orphan set on disk is exactly the allowlist.
 *
 * A failure here means a generated TU stopped matching its src/ authority, a
 * new TU was added with no authority, or an orphan was fixed without updating
 * the manifest.  Run from the repository root.
 */
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define DECOMP_DIR "pc_port/game/decomp"
#define SRC_DIR    "src"
#define MANIFEST   "tools/analysis/decomp_port_orphans.txt"
#define GENERATOR  "tools/analysis/gen_decomp_ports.py"

#define MAX_NAMES 1024

static char listed[MAX_NAMES][64];
static int listed_count;

static int file_exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static int load_manifest(void)
{
    FILE *f = fopen(MANIFEST, "r");
    char line[256];
    if (!f) {
        fprintf(stderr, "manifest %s missing\n", MANIFEST);
        return -1;
    }
    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        while (*p == ' ' || *p == '\t')
            p++;
        if (*p == '#' || *p == '\n' || *p == '\0')
            continue;
        {
            size_t n = strlen(p);
            while (n && (p[n - 1] == '\n' || p[n - 1] == '\r'))
                p[--n] = '\0';
        }
        if (listed_count >= MAX_NAMES) {
            fclose(f);
            fprintf(stderr, "manifest too large\n");
            return -1;
        }
        snprintf(listed[listed_count], sizeof(listed[0]), "%s", p);
        listed_count++;
    }
    fclose(f);
    return 0;
}

static int is_listed(const char *name)
{
    int i;
    for (i = 0; i < listed_count; i++)
        if (strcmp(listed[i], name) == 0)
            return 1;
    return 0;
}

static int check_orphans_on_disk(void)
{
    DIR *d = opendir(DECOMP_DIR);
    struct dirent *e;
    int orphans = 0, missing = 0, i, failures = 0;
    char seen[MAX_NAMES][64];
    int seen_count = 0;

    if (!d) {
        fprintf(stderr, "cannot open %s\n", DECOMP_DIR);
        return 1;
    }
    while ((e = readdir(d)) != NULL) {
        char name[64], srcpath[512];
        size_t n = strlen(e->d_name);
        if (n <= 7 || strcmp(e->d_name + n - 7, "_port.c") != 0)
            continue;
        n -= 7;
        if (n >= sizeof(name))
            continue;
        memcpy(name, e->d_name, n);
        name[n] = '\0';
        snprintf(srcpath, sizeof(srcpath), "%s/%s.c", SRC_DIR, name);
        if (file_exists(srcpath)) {
            if (is_listed(name)) {
                fprintf(stderr, "stale allowlist: %s now has a src/ leaf\n", name);
                failures++;
            }
        } else {
            orphans++;
            if (!is_listed(name)) {
                fprintf(stderr, "new orphan (no src/ leaf, not allowlisted): %s\n",
                        name);
                failures++;
            }
        }
        if (seen_count < MAX_NAMES)
            snprintf(seen[seen_count++], sizeof(seen[0]), "%s", name);
    }
    closedir(d);

    for (i = 0; i < listed_count; i++) {
        int j, found = 0;
        for (j = 0; j < seen_count; j++)
            if (strcmp(listed[i], seen[j]) == 0) {
                found = 1;
                break;
            }
        if (!found) {
            fprintf(stderr, "manifest lists a TU that does not exist: %s\n",
                    listed[i]);
            failures++;
        }
    }

    printf("decomp ports: %d generated, %d orphan(s), allowlist %d\n",
           seen_count, orphans, listed_count);
    (void)missing;
    return failures;
}

int main(void)
{
    int failures = 0;
    int rc;

    if (!file_exists(GENERATOR)) {
        fprintf(stderr, "generator %s missing\n", GENERATOR);
        return 1;
    }
    if (load_manifest() != 0)
        return 1;

    failures += check_orphans_on_disk();

    rc = system("python3 " GENERATOR " --check --allow-orphans");
    if (rc != 0) {
        fprintf(stderr, "generator --check failed (status %d)\n", rc);
        failures++;
    }

    if (failures) {
        fprintf(stderr, "FAIL: decomp-port reproducibility: %d problem(s)\n",
                failures);
        return 1;
    }
    printf("PASS: decomp ports reproduce from src/ authority;"
           " orphans match the allowlist\n");
    return 0;
}
