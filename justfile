set dotenv-load

builddir := env('BUILDDIR','build')
build_type := env('BUILD_TYPE', 'release')
num_threads := num_cpus()

build:
    ninja -C {{builddir}} -j{{num_threads}}

setup:
    meson setup {{builddir}} -Dbuildtype={{build_type}}

reconfigure:
    meson setup --reconfigure {{builddir}} -Dbuildtype={{build_type}}

run *args: build
    ./{{builddir}}/apa {{args}}

clean:
    rm -rf {{builddir}}

test:
    meson test -C {{builddir}}

format:
    find src -name '*.cpp' -or -name "*.h" | xargs clang-format -i
    nix fmt

check:
    cppcheck --enable=all --std=c++23 --check-level=exhaustive src \
        --suppress=missingIncludeSystem --suppress=checkersReport 

lint:
    find src -name '*.cpp' -or -name '*.h' | xargs clang-tidy -p {{builddir}} --fix 
