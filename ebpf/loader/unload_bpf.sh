#!/bin/bash

set -e

echo "🔻 Unloading pinned eBPF objects..."

FILES=(
  "/sys/fs/bpf/sched_switch_link"
  "/sys/fs/bpf/mutators"
  "/sys/fs/bpf/enabled"
  "/sys/fs/bpf/iowait"
)

for file in "${FILES[@]}"; do
  if [ -e "$file" ]; then
    echo "🗑️  Removing $file"
    sudo rm -f "$file"
  else
    echo "ℹ️  $file not found (already removed)"
  fi
done

echo "✅ All pinned eBPF objects removed."
