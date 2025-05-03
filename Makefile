container_tag := btcinc
src_path := btcinc
dependencies_math_t := $(src_path)/lib/bigrsa/math_t/*.c
dependencies_bigrsa := $(src_path)/lib/bigrsa/*.c $(dependencies_math_t)
dependencies_sha256 := $(src_path)/lib/sha256/*.c
dependencies_lib := $(src_path)/lib/*.c
dependencies := $(src_path)/*.c $(dependencies_lib) $(dependencies_bigrsa) $(dependencies_sha256)
gcc_flags := --std=c89 -Wall -Wextra -Werror -Wpedantic -O2 -lgmp

outie: podman_run

podman_run: podman_build
	podman run -it $(container_tag) make -f ./btcinc/Makefile innie

podman_build: style
	podman build -t $(container_tag) -f Containerfile .

style:
	astyle --style=java --suffix=none --recursive "*.c" "*.h"

innie: innie_run

innie_run: innie_compile

innie_compile:
	gcc $(dependencies) -o btcinc.o $(gcc_flags)
