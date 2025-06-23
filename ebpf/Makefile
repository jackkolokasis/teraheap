CXX = g++
CXXFLAGS = -Wall -O2 -fPIC -Iinclude
LDFLAGS = -shared -lbpf

SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = build
LIB_DIR = lib
EBPF_DIR = ebpf_tracing
LOADER_DIR = loader

OBJ = $(OBJ_DIR)/ioWait.o
BPF_OBJ = $(OBJ_DIR)/sched_switch.bpf.o
BPF_SRC = $(EBPF_DIR)/sched_switch.bpf.c
VMLINUX_H = $(EBPF_DIR)/vmlinux.h

TARGET = $(LIB_DIR)/libebpf_profiler.so
LOADER_SRC = $(LOADER_DIR)/loader_bpf_setup.cpp
LOADER_OBJ = $(OBJ_DIR)/loader_bpf_setup.o
LOADER_BIN = $(LOADER_DIR)/load_bpf_setup

.PHONY: all clean load unload

# Default rule
all: $(TARGET) $(LOADER_BIN)

# Create output dirs
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

# Generate vmlinux.h
$(VMLINUX_H):
	bpftool btf dump file /sys/kernel/btf/vmlinux format c > $(VMLINUX_H)

# Compile BPF C source to .o
$(BPF_OBJ): $(BPF_SRC) $(VMLINUX_H) | $(OBJ_DIR)
	clang -O2 -g -target bpf -D__TARGET_ARCH_x86 \
		-I$(EBPF_DIR) -c $< -o $@

# Compile ioWait.o
$(OBJ): $(SRC_DIR)/ioWait.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link shared library
$(TARGET): $(OBJ) $(BPF_OBJ) | $(LIB_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(LDFLAGS)

# Compile loader setup binary
$(LOADER_BIN): $(LOADER_SRC) | $(OBJ_DIR)
	$(CXX) -Wall -O2 -o $@ $< -lbpf

# Clean rule
clean:
	rm -rf $(OBJ_DIR) $(LIB_DIR) $(VMLINUX_H) $(LOADER_BIN)

load:
	cd loader && sudo ./load_bpf.sh

unload:
	cd loader && sudo ./unload_bpf.sh
