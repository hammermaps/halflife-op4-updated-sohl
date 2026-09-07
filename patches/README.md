# Patches

Project directive (added 2026-09-08): whenever a working, verified change is
completed on this branch, export it as a standalone patch file here so it
can be transferred to sibling projects (other SoHL/HLSDK forks) later
without re-deriving the change from scratch.

## Naming

`patches/<date>-<slug>.patch`, e.g. `2026-09-08-hybrid-ai-core-phase-a.patch`.

## Generating one

For a single commit that represents a complete, working change:

```bash
git format-patch -1 <commit-sha> --stdout > patches/<date>-<slug>.patch
```

For a range of commits that together make up one logical change:

```bash
git format-patch <base-sha>..<tip-sha> --stdout > patches/<date>-<slug>.patch
```

## Applying one to another project

```bash
git am < patches/<date>-<slug>.patch          # preserves author/message
# or, if the target tree has diverged too much for a clean am:
git apply --3way patches/<date>-<slug>.patch
```

Patches are plain `git format-patch` output (unified diff + commit
metadata) - portable as long as the target file paths line up closely
enough for `git apply`/`patch` to resolve hunks, which is the common case
across sibling HLSDK forks sharing most of their directory layout.
