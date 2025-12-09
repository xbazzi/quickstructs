{
  description = "C and C++";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      ...
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
        llvm = pkgs.llvmPackages_latest;
        lib = nixpkgs.lib;
        pythonPackages = pkgs.python313Packages;
        pyPkgs = with pythonPackages; [
          pandas
          matplotlib
          numpy
          plotly
          seaborn
        ];

      in
      {
        # devShell = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } rec {
        devShell = pkgs.mkShell {
          nativeBuildInputs =
            pyPkgs
            ++ (with pkgs; [
              clang
              bear
              llvm.lldb
              llvm.clang
              gbenchmark
              gcc
              gtest
              binutils
              cmake
              gdb
              pkg-config
              boost
              toml11
              openssl
              valgrind
              nlohmann_json
              doxygen
              graphviz
              zsh
              grpc
              protobuf
              llvmPackages_21.clang-tools
            ]);

          buildInputs = with pkgs; [
            # pkgs.cassandra-cpp-driver
            llvmPackages_21.clang-tools
            nodejs_22
            gcc
            binutils
            cmake
            gdb
            pkg-config
            boost
            toml11
            openssl
            valgrind
            nlohmann_json
            doxygen
            graphviz
            zsh
            grpc
            protobuf
            # llvm.libcxx
          ];
          shell = pkgs.zsh;
          shellHook = ''
            echo "Welcome to the QuickStructs flake dev shell" 
            export CC=gcc
            export CXX=g++
          '';

          # CPATH = builtins.concatStringsSep ":" [
          #   (lib.makeSearchPathOutput "dev" "include" [ llvm.libcxx ])
          #   (lib.makeSearchPath "resource-root/include" [ llvm.clang ])
          # ];
        };
      }
    );
}
