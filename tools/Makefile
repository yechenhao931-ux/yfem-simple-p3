CXX ?= c++
BUILD_TYPE ?= Debug

TARGET := generate_litho_mask_mesh
SOURCE := generate_litho_mask_mesh.cpp
MFEM_LIB := ../build/libmfem.a

CPPFLAGS := -I..
CXXFLAGS := -std=c++17
LDFLAGS :=

ifeq ($(BUILD_TYPE),Debug)
	CXXFLAGS += -O0 -g
else ifeq ($(BUILD_TYPE),Release)
	CXXFLAGS += -O3 -DNDEBUG
else ifeq ($(BUILD_TYPE),RelWithDebInfo)
	CXXFLAGS += -O2 -g -DNDEBUG
else ifeq ($(BUILD_TYPE),MinSizeRel)
	CXXFLAGS += -Os -DNDEBUG
else
	$(error Unsupported BUILD_TYPE='$(BUILD_TYPE)'. Use Debug, Release, RelWithDebInfo, or MinSizeRel)
endif

.PHONY: all clean run check-lib

all: $(TARGET)

$(TARGET): $(SOURCE) $(MFEM_LIB)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< $(MFEM_LIB) $(LDFLAGS) -o $@

$(MFEM_LIB):
	$(error Missing $(MFEM_LIB). Please build libmfem.a in the project build directory first)

run: $(TARGET)
	cd .. && tools/$(TARGET)

check-lib:
	@test -f $(MFEM_LIB) && echo "Found $(MFEM_LIB)"

clean:
	rm -f $(TARGET)
