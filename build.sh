set echo on

mkdir -p build

include_flags="-Icode/"
linker_flags="-lvulkan -lwayland-client -lglslang -lglslang-default-resource-limits"
defines="-DIGNIS_DEBUG -DIGNIS_PLATFORM_LINUX"

echo "Building Main"
clang++ -g code/app/main.cpp -o build/main $defines $include_flags $linker_flags
