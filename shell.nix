{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } rec
{
  packages = with pkgs;
  [
    clang-tools # clang-tools should go before clang. If not lsp (clangd) is not working properly
    clang
    gdb
    renderdoc
  ];

  buildInputs = with pkgs;
  [
    wayland
    wayland-scanner
    vulkan-headers
    vulkan-loader
    vulkan-validation-layers
    vulkan-tools # vulkaninfo
    glslang
    vulkan-tools-lunarg # vkconfig
  ];

  VK_LAYER_PATH = "${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d";
}
