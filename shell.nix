{ pkgs ? import <nixpkgs> { config.allowUnfree = true; } }:

(pkgs.buildFHSEnv {
  name = "liquorchess-fhs";

  targetPkgs = pkgs: with pkgs; [
    cmake
    gcc
    gdb
    python314
    python314Packages.pip
    python314Packages.numpy
    python314Packages.torch
    cutechess
    lichess-bot
    cpm-cmake
    cargo
    en-croissant
    zstd
    ncurses5
    binutils
    gitRepo gnupg autoconf curl
    procps gnumake util-linux m4 gperf unzip
    libGLU libGL
    zlib
    glib

    glibc.dev
    stdenv.cc.cc
  ];

  profile = ''
    export NIX_LD_LIBRARY_PATH=${pkgs.lib.makeLibraryPath [
      pkgs.stdenv.cc.cc.lib
      pkgs.zlib
      pkgs.zstd
      pkgs.ncurses5
      pkgs.binutils
      pkgs.libGLU
      pkgs.libGL
      pkgs.glib
    ]}
    export NIX_LD=${builtins.readFile "${pkgs.stdenv.cc}/nix-support/dynamic-linker"}
    export LD_LIBRARY_PATH="$NIX_LD_LIBRARY_PATH:$LD_LIBRARY_PATH"
  '';
}).env