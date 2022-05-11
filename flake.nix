{
  description = "An implementation of the Double Ratchet cryptographic ratchet";

  inputs.nixpkgs.url = "nixpkgs/c777cdf5c564015d5f63b09cc93bef4178b19b01";
  # c777cdf5c564015d5f63b09cc93bef4178b19b01 is current unstable.  We can't use
  # the current stable release because of
  # https://github.com/emscripten-core/emscripten/issues/14995
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = { self, nixpkgs, flake-utils }:
   (
      flake-utils.lib.eachDefaultSystem (system:
        let
          pkgs = import nixpkgs {
            inherit system;
          };
        in
          rec {
            packages.javascript = pkgs.buildEmscriptenPackage {
              name = "olm";

              buildInputs = [ pkgs.gnumake pkgs.python3 pkgs.yarn ];

              src = ./.;

              configurePhase = "";

              buildPhase = ''
                export EM_CACHE=$TMPDIR
                make javascript/exported_functions.json
                make js
              '';

              output = [ "out" ];

              installPhase = ''
                mkdir -p $out/javascript
                cd javascript
                echo sha256: > checksums.txt
	              sha256sum olm.js olm_legacy.js olm.wasm >> checksums.txt
	              echo sha512: >> checksums.txt
	              sha512sum olm.js olm_legacy.js olm.wasm >> checksums.txt
                cp package.json olm.js olm.wasm olm_legacy.js index.d.ts README.md checksums.txt $out/javascript
                cd ..
              '';

              checkPhase = ''
                cd javascript
                export HOME=$TMPDIR
                yarn install
                yarn test
                cd ..
              '';
            };

            defaultPackage = packages.javascript;
          }
      )
   );
}
