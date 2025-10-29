#!/bin/bash
set -e

echo "🔍 Refreshing all Git submodules..."
echo

# Ensure we're at repo root
cd "$(git rev-parse --show-toplevel)"

# Step 1: Remove all submodules
echo "🚮 Removing all submodules and cached data..."
git submodule deinit -f --all || true

# Remove submodule references from index
for sub in $(git config --file .gitmodules --get-regexp path | awk '{ print $2 }'); do
  if [ -d "$sub" ]; then
    echo " - Unregistering submodule: $sub"
    git rm -f --cached "$sub" || true
    rm -rf "$sub"
  fi
done

# Remove leftover metadata
echo "🧹 Cleaning up .git/modules..."
rm -rf .git/modules/* || true

echo
echo "✅ All submodules removed from index and cache (but .gitmodules preserved)"
echo

# Step 2: Re-add submodules based on .gitmodules
echo "➕ Re-adding submodules from .gitmodules..."

while read -r key value; do
  if [[ $key =~ ^submodule\.(.*)\.path$ ]]; then
    name="${BASH_REMATCH[1]}"
    path="$value"
    url=$(git config --file .gitmodules --get "submodule.$name.url")

    if [ -z "$url" ]; then
      echo "⚠️ Skipping $path (no URL found)"
      continue
    fi

    echo " - Adding $name at $path"
    git submodule add "$url" "$path" || true
  fi
done < <(git config --file .gitmodules --get-regexp '^submodule\..*\.path$')

# Step 3: Sync and update
echo
echo "🔄 Syncing and initializing submodules..."
git submodule sync --recursive
git submodule update --init --recursive

echo
echo "✅ Submodule refresh complete!"
git submodule status
