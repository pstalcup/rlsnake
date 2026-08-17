# rlsnake

A raylib project scaffold that builds for desktop and for the browser (WebAssembly),
with CI that publishes the web build to the `gh-pages` branch.

## Layout

```
src/main.c          entry point (stub — no game logic yet)
src/shell.html      emscripten HTML shell used for the web build
assets/             bundled into the web build via --preload-file
CMakeLists.txt      fetches raylib 5.5 and builds both targets
.github/workflows/deploy.yml
```

## Desktop build

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/rlsnake
```

Requires CMake ≥ 3.16 and a C compiler. On Debian/Ubuntu raylib also needs the X11
dev packages:

```sh
sudo apt install build-essential cmake libasound2-dev libx11-dev libxrandr-dev \
  libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
```

## Web build

With the [emsdk](https://emscripten.org/docs/getting_started/downloads.html)
activated in your shell:

```sh
emcmake cmake -B build-web -DCMAKE_BUILD_TYPE=Release
cmake --build build-web -j
python3 -m http.server -d build-web 8080   # then open http://localhost:8080/index.html
```

Output is `index.html` + `index.js` + `index.wasm` (+ `index.data` if `assets/`
is non-empty), so the directory can be served as a static site root.

## Deployment

`.github/workflows/deploy.yml` builds on every push and PR to `main`, and on
pushes to `main` force-pushes the `dist/` output to the `gh-pages` branch.

One-time setup after the first successful run:

1. **Settings → Pages → Source**: "Deploy from a branch", branch `gh-pages`, folder `/ (root)`.
2. **Settings → Actions → General → Workflow permissions**: "Read and write permissions"
   (the workflow already requests `contents: write`, but the repo default must allow it).

The site then lives at `https://<user>.github.io/rlsnake/`.
