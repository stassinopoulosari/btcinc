container_tag := btcinc

src_path := btcinc
primary:= $(src_path)/btcinc.c
lib_path := $(src_path)/lib
dependencies_math_t := $(lib_path)/bigrsa/math_t/*.c
dependencies_list_t := $(lib_path)/list_t/*.c
dependencies_bigrsa := $(lib_path)/bigrsa/*.c $(dependencies_math_t)
dependencies_sha256 := $(lib_path)/sha256/*.c
dependencies := $(lib_path)/*.c $(dependencies_lib) $(dependencies_bigrsa) $(dependencies_sha256) $(dependencies_list_t)
gcc_flags := --std=c89 -Wall -Wextra -Werror -Wpedantic -O2 -lgmp
binary_name := btcinc.o

outie: podman_run

podman_run: podman_build
	podman run -it $(container_tag) make -f ./btcinc/Makefile innie

podman_build: style
	podman build -t $(container_tag) -f Containerfile .

style:
	astyle --style=java --suffix=none --recursive "*.c" "*.h"

innie: innie_run

innie_run: innie_compile
	./$(binary_name)

innie_compile:
	gcc $(primary) $(dependencies) -o $(binary_name) $(gcc_flags)
