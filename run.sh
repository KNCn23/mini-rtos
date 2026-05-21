#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"
command -v qemu-system-aarch64 >/dev/null || {
    echo "Install QEMU first:  brew install qemu  /  apt install qemu-system-arm"; exit 1; }
make -s
echo "─── Booting mini-rtos.  Exit:  Ctrl-A  then  x ───"
exec qemu-system-aarch64 -M virt -cpu cortex-a72 -m 256M -nographic -kernel kernel.elf
