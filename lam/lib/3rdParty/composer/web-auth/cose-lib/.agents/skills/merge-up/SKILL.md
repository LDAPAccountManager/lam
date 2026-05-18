---
name: merge-up
description: >
  Cascade-merge maintained branches from oldest to newest (e.g.
  3.1.x → 3.2.x → 3.3.x → 4.0.x). Use when the user says "merge branches",
  "merge up", "cascade merge", "sync branches", or "update branches".
---

# Branch Cascade Merge

Merges each maintained branch into the next one, from oldest to newest.

## Progress checklist

- [ ] Step 0: Pre-flight checks
- [ ] Step 1: Determine maintained branches and pull them
- [ ] Step 2: Cascade merge loop

---

## Confirmation rule

Whenever the skill says **"Wait for confirmation"**, treat anything other than an
explicit affirmative as **no**: stop and ask the user how they want to proceed.

---

## Step 0 — Pre-flight checks

```bash
git status --porcelain --untracked-files=no
```

If any output, **stop**:
> "The working tree is not clean. Please commit or stash your changes first."

---

## Step 1 — Determine maintained branches and pull them

### 1a. Get the branch list

Ask the user which branches to cascade-merge. The user should provide an
ordered list from oldest to newest (e.g. `3.1.x 3.2.x 3.3.x 4.0.x`).

If the user doesn't provide a list, determine it from remote branches:

```bash
git branch -r --list 'origin/*.*.x' | sed 's|origin/||' | sort -V
```

Present the list and **wait for confirmation** before proceeding. The user may
want to exclude some branches (e.g. EOL branches). Store the confirmed list as
`BRANCHES`.

### 1b. Pull every branch

For each branch in `BRANCHES`:

```bash
git checkout <branch>
git pull --ff-only origin <branch>
```

Using `--ff-only` ensures local branches haven't diverged from origin. If the
pull fails, **stop** and report the error.

---

## Step 2 — Cascade merge loop

For each consecutive pair `(SOURCE, TARGET)` in `BRANCHES`:

### 2a. Merge

```bash
git checkout <TARGET>
git merge <SOURCE>
```

Three outcomes are possible:

- **Already up-to-date:** print "✓ `<TARGET>` already up-to-date with `<SOURCE>`"
  and skip to the next pair.
- **Clean merge (no conflicts):** git creates the merge commit automatically.
  Proceed directly to step 2c.
- **Conflicts:** proceed to step 2b.

### 2b. Resolve conflicts (only when git reports conflicts)

List conflicts:

```bash
git diff --name-only --diff-filter=U
```

Read each conflicted file, resolve it, then `git add` it. When all are resolved:

```bash
git commit --no-edit
```

#### Conflict resolution rules

| File pattern | Strategy |
|---|---|
| `CHANGELOG*.md` | Keep entries from both sides; newer branch entries on top |
| Version constants, `composer.json` branch aliases | Keep the TARGET branch value |
| `composer.json` dependency versions | Keep the TARGET branch value (newer branch may require higher versions) |
| Code files | Merge logically based on context; when unsure, ask the user |

After resolving, show `git diff HEAD~1` (first parent of the merge commit, i.e.
the previous TARGET state) and wait for the user to confirm the resolution looks
correct before proceeding.

### 2c. Run tests

Run the test suite to verify the merge didn't break anything:

```bash
composer install
vendor/bin/phpunit
```

If the project uses `castor`, prefer:

```bash
composer install
castor phpunit
```

If tests fail, first check whether the failure is pre-existing: run the same
test on the TARGET branch before the merge. Only fix failures introduced by the
merge:
1. Analyze and fix the code.
2. Commit the fix with a descriptive message.
3. Re-run failing tests until green.

Report any pre-existing failures to the user without attempting to fix them.

### 2d. Ask for confirmation before pushing

Show:

```
Merge: <SOURCE> → <TARGET>
Tests: all passing

Commits since origin/<TARGET>:
git log --oneline origin/<TARGET>..<TARGET>

Ready to push? (yes / no)
```

**Wait for confirmation.** The user may make changes themselves before confirming.

### 2e. Push and continue

```bash
git push origin <TARGET>
```

If the push fails, **stop** and report the error.

Print "✓ `<SOURCE>` → `<TARGET>` done." and continue to the next pair.

---

## Final summary

```
All merges complete:
  3.1.x → 3.2.x  ✓
  3.2.x → 3.3.x  ✓
  3.3.x → 4.0.x  ✓
```

---

## Gotchas

- `CHANGELOG.md` conflicts are the most common; entries must be kept from both
  sides, never dropped.
- A merge can introduce test failures even without conflicts, because behavior
  from the older branch may be incompatible with newer code. Always run tests.
- When merging across major versions (e.g. 3.x → 4.x), pay extra attention to
  breaking changes, removed deprecations, and updated PHP version requirements.

## Error handling

- **Never** force-push or rewrite history.
- **Never** use `--no-verify` on commits.
- **Never** auto-recover from a failed `git push` or `git pull`. Stop and hand
  control back to the user.
