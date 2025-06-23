CXX = g++

INCLUDE_DIR = /home1/public/kolokasis/github/latest_version/teraheap/ebpf/include
LIB_DIR     = /home1/public/kolokasis/github/latest_version/teraheap/ebpf/lib

CXXFLAGS = -Wall -O2 -fPIC -I$(INCLUDE_DIR)
LDFLAGS  = -L$(LIB_DIR) -lebpf_profiler -lbpf -Wl,-rpath=$(LIB_DIR)

SRC_DIR = src
BUILD_DIR = build

# Map source files to output binaries
SRC_FILES = \
  $(SRC_DIR)/test_null_block.cpp \
  $(SRC_DIR)/test_null_block_multi_thread.cpp \
  $(SRC_DIR)/test_null_block_multi_thread_same_core.cpp \
  $(SRC_DIR)/test_ebpf_overhead.cpp \
	$(SRC_DIR)/test_mutex_no_io.cpp \
	$(SRC_DIR)/test_null_block_mmap.cpp

BINARIES = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%, $(SRC_FILES))

all: $(BUILD_DIR) $(BINARIES)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)
