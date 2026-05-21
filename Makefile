ifeq ($(origin CROSS),undefined)
  ifneq ($(shell command -v aarch64-elf-gcc 2>/dev/null),)
    CROSS := aarch64-elf-
  else ifneq ($(shell command -v aarch64-unknown-linux-gnu-gcc 2>/dev/null),)
    CROSS := aarch64-unknown-linux-gnu-
  else ifneq ($(shell command -v aarch64-linux-gnu-gcc 2>/dev/null),)
    CROSS := aarch64-linux-gnu-
  else
    $(error Install an AArch64 cross-compiler)
  endif
endif

CC      = $(CROSS)gcc
LD      = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy
CFLAGS  = -ffreestanding -fno-builtin -fno-stack-protector \
          -nostdlib -mcpu=cortex-a72 -Wall -Wextra -O2 \
          -Iinclude -Idrivers -Ikernel
ASFLAGS = -mcpu=cortex-a72

SRC_S = boot/startup.S kernel/context_switch.S
SRC_C = drivers/uart.c drivers/timer.c \
        kernel/task.c kernel/sched.c kernel/sync.c kernel/kernel.c

OBJS  = $(SRC_S:.S=.o) $(SRC_C:.c=.o)
TARGET = kernel.elf

.PHONY: all clean run debug
all: $(TARGET)

$(TARGET): $(OBJS) linker.ld
	$(LD) -T linker.ld -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
%.o: %.S
	$(CC) $(ASFLAGS) -c $< -o $@

run: $(TARGET)
	@echo "Booting mini-rtos. Exit QEMU with Ctrl-A then x"
	qemu-system-aarch64 -M virt -cpu cortex-a72 -m 256M \
	  -nographic -kernel $(TARGET)

debug: $(TARGET)
	qemu-system-aarch64 -M virt -cpu cortex-a72 -m 256M \
	  -nographic -kernel $(TARGET) -s -S

clean:
	rm -f $(OBJS) $(TARGET)
