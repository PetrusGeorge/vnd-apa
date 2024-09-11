{
  description = "Nix shell APA";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    nixpkgs,
    flake-utils,
    ...
  } @ inputs:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs {inherit system;};
    in {
      devShell = pkgs.mkShell {
        nativeBuildInputs = with pkgs; [
          meson
          ninja

          (clang-tools.override {enableLibcxx = true;})
          lldb
          llvmPackages.libcxxClang
          llvmPackages.libcxx
          mold
        ];

        buildInputs = with pkgs; [
          cppcheck
          just
        ];

        LD_LIBRARY_PATH = nixpkgs.lib.strings.makeLibraryPath [pkgs.llvmPackages.libcxx];

        shellHook = ''
          export CXX=clang++
          export CXX_LD=mold
        '';
      };

      formatter = pkgs.alejandra;

      packages.default = pkgs.stdenv.mkDerivation {
        pname = "apa";
        version = "0.1.0";
        src = ./.;
        nativeBuildInputs = with pkgs; [
          gcc
          meson
          ninja
        ];
        mesonBuildType = "release";
      };
    });
}
