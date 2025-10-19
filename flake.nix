{
  description = "A small 2D ECS engine I built to better understand low-level game programming and engine design.";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = { self, nixpkgs }: 
    let
      pkgs = nixpkgs.legacyPackages.x86_64-linux;
      
      buildDeps = with pkgs; [
        # packages needed on runtime of the program
        libcxx
        SDL2
        SDL2_image
        SDL2_ttf
        SDL2_mixer
        lua5_4
      ];
      
      nativeDeps = with pkgs;[
        # packages needed to build the project
        zig_0_14
      ];
      
      sharedEnv = {
        # environment for nix shell and build time
        ZIG_GLOBAL_CACHE_DIR = "./.zig-cache";
        C_INCLUDE_PATH="${pkgs.SDL2}/include/SDL2:${pkgs.SDL2_image}/include/SDL2:${pkgs.SDL2_ttf}/include/SDL2:${pkgs.SDL2_mixer}/include/SDL2:$C_INCLUDE_PATH";
        LIBRARY_PATH="${pkgs.SDL2}/lib:${pkgs.SDL2_image}/lib:${pkgs.SDL2_ttf}/lib:${pkgs.SDL2_mixer}/lib:$LIBRARY_PATH";
      };
      
    in {
      packages.x86_64-linux.default = pkgs.stdenv.mkDerivation ({
        pname = "2d-ecs-engine";
        version = "1.0.0";
        src = ./.;
        buildInputs = buildDeps;
        nativeBuildInputs = nativeDeps;
        buildPhase = ''

          zig build
        '';
        installPhase = ''
          mkdir -p $out/bin
          ls -a
          cp zig-out/bin/* $out/bin/ 
          '';
      } // sharedEnv);

      devShells.x86_64-linux.default = pkgs.mkShell ({
        buildInputs = buildDeps ++ nativeDeps;
      } // sharedEnv);
    };
}
