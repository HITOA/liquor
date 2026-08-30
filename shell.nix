
with import <nixpkgs> { 
    config.allowUnfree = true;
};
pkgs.mkShell {
    buildInputs = with pkgs; [
        cmake
        gcc
        gdb
        python312
        python312Packages.pip
        cutechess
        lichess-bot
    ];

    NIX_LD_LIBRARY_PATH = lib.makeLibraryPath [
        stdenv.cc.cc.lib
        stdenv.cc
    ];

    NIX_LD = builtins.readFile "${stdenv.cc}/nix-support/dynamic-linker";
    shellHook = ''
        export "LD_LIBRARY_PATH=$NIX_LD_LIBRARY_PATH"
    '';
}
