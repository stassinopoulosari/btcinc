container_tag := btcinc
src_path := btcinc
dependencies_math_t := $(src_path)/bigrsa/math_t/*.c
dependencies_bigrsa := $(src_path)/bigrsa/*.c $(depndencies_math_t)
dependencies_sha256 := $(src_path)/sha256/*.c
dependencies := $(src_path)/*.c $(dependencies_bigrsa) $(dependencies_sha256)
gcc_flags := --std=c89 -Wall -Wextra -Werror -Wpedantic -O2 -lgmp

outie: podman_run

podman_run: podman_build
	podman run -it $(container_tag) make -f ./btcinc/Makefile innie

podman_build: style
	podman build -t $(container_tag) -f test/Containerfile.test .

style:
	astyle --style=java --suffix=none --recursive "*.c" "*.h"

innie: innie_run

innie_run: innie_compile

innie_compile:
	gcc $(gcc_flags) $(dependencies) -o btcinc.o
