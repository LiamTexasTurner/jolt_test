#!/usr/bin/env bash

cd "$(dirname "$0")" || exit 1

name="SceneRenderer"
build_dir="$PWD/cmake-build-debug"

cmake --build "$build_dir" --target "$name" -j 22 || exit 1

cd "$build_dir" || exit 1
./"$name"
