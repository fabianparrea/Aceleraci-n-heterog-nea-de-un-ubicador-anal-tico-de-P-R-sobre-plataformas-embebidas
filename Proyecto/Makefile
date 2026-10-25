CC = gcc
CFLAGS = -std=gnu11 -O3 -fopenmp -Wall -Wextra -MMD -MP -Iinclude
LDLIBS = -lm -fopenmp

# version CUDA (make gpu). Por defecto apunta a la Jetson Nano (Maxwell, sm_53)
CUDA_HOME ?= /usr/local/cuda
NVCC ?= $(CUDA_HOME)/bin/nvcc
CUDA_ARCH ?= sm_53
NVCCFLAGS = -O3 -arch=$(CUDA_ARCH) -Iinclude
CUDA_LDLIBS = -L$(CUDA_HOME)/lib64 -Wl,-rpath,$(CUDA_HOME)/lib64 -lcufft -lcudart -lstdc++

SRCS = $(filter-out src/main.c, $(wildcard src/*.c))
OBJS = $(SRCS:src/%.c=build/%.o)
TESTS = $(patsubst tests/%.c,build/%,$(filter-out tests/test_gpu.c, $(wildcard tests/test_*.c)))

# en la version CUDA solo cambia backend.c (compilado con USE_CUDA) y se suman
# los .cu; el resto de los objetos es el mismo de la version CPU
CU_SRCS = $(wildcard src/*.cu)
GPU_OBJS = $(filter-out build/backend.o, $(OBJS)) build/gpu/backend.o \
           $(CU_SRCS:src/%.cu=build/gpu/%.o)
CU_HEADERS = include/gpu.h include/gpu_internal.h include/prof.h include/netlist.h include/grid.h

all: build/placer

gpu: build/placer_gpu

build/placer: build/main.o $(OBJS)
	$(CC) $^ -o $@ $(LDLIBS)

build/placer_gpu: build/main.o $(GPU_OBJS)
	$(CC) $^ -o $@ $(LDLIBS) $(CUDA_LDLIBS)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/gpu/backend.o: src/backend.c | build/gpu
	$(CC) $(CFLAGS) -DUSE_CUDA -c $< -o $@

build/gpu/%.o: src/%.cu $(CU_HEADERS) | build/gpu
	$(NVCC) $(NVCCFLAGS) -c $< -o $@

build/test_%: tests/test_%.c $(OBJS) | build
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LDLIBS)

build/gpu/test_gpu: tests/test_gpu.c $(GPU_OBJS) | build/gpu
	$(CC) $(CFLAGS) $< $(GPU_OBJS) -o $@ $(LDLIBS) $(CUDA_LDLIBS)

test: $(TESTS)
	@for t in $(TESTS); do ./$$t || exit 1; done

# compara la version GPU contra la de CPU con los mismos datos
test-gpu: build/gpu/test_gpu
	./build/gpu/test_gpu

build build/gpu:
	mkdir -p $@

clean:
	rm -rf build

.PHONY: all gpu test test-gpu clean

-include build/*.d build/gpu/*.d
