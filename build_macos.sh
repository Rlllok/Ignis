
mkdir -p build
cp -r data/ build/data

if [ ! -f ./code/third_party/wayland/xdg_shell.h ] || [ ! -f ./code/third_party/wayland/xdg_shell.cpp ]; then
    echo "---- Create xdg_shell_protocol files"
    wayland-scanner client-header ./code/third_party/wayland/protocols/xdg-shell.xml ./code/third_party/wayland/xdg_shell.h
    wayland-scanner private-code ./code/third_party/wayland/protocols/xdg-shell.xml ./code/third_party/wayland/xdg_shell.cpp
fi
if [ ! -f ./code/third_party/wayland/relative_pointer_unstable_v1.h ] || [ ! -f ./code/third_party/wayland/relative_pointer_unstable_v1.cpp ]; then
    echo "---- Create relative_pointer_unstable_v1_protocol files"
    wayland-scanner client-header ./code/third_party/wayland/protocols/relative-pointer-unstable-v1.xml ./code/third_party/wayland/relative_pointer_unstable_v1.h
    wayland-scanner private-code ./code/third_party/wayland/protocols/relative-pointer-unstable-v1.xml ./code/third_party/wayland/relative_pointer_unstable_v1.cpp
fi
if [ ! -f ./code/third_party/wayland/pointer_constraints_unstable_v1.h ] || [ ! -f ./code/third_party/wayland/pointer_constraints_unstable_v1.cpp ]; then
    echo "---- Create pointer_constrains_unstable_v1_protocol files"
    wayland-scanner client-header ./code/third_party/wayland/protocols/pointer-constraints-unstable-v1.xml ./code/third_party/wayland/pointer_constraints_unstable_v1.h
    wayland-scanner private-code ./code/third_party/wayland/protocols/pointer-constraints-unstable-v1.xml ./code/third_party/wayland/pointer_constraints_unstable_v1.cpp
fi

#warnings="-Wconversion"
include_flags="-Icode/"
default_links="-framework Cocoa"
defines="-DIGNIS_DEBUG -DIGNIS_PLATFORM_MACOS"

echo "**************************************************"
echo "Compilation started."
echo "**************************************************"

if [ "$1" == "macos"  ]; then
  time (
    clang -x objective-c -g ./code/app/macos.c -o build/macos  $warnings $defines $default_links $include_flags;
  );
fi

echo "**************************************************"
echo "Compilation ended."
echo "**************************************************"
