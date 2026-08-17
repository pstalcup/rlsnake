# rlsnake

A raylib project scaffold that builds for desktop and for the browser (WebAssembly),
with CI that publishes the web build to the `gh-pages` branch.

## Layout

```
src/main.c          entry point (stub — no game logic yet)
src/assets.c/.h     asset path anchoring + JSON loading
src/shell.html      emscripten HTML shell used for the web build
assets/images/      textures (PNG)
assets/data/        JSON data files
CMakeLists.txt      fetches raylib 5.5 + cJSON, builds both targets
.github/workflows/deploy.yml
```

## Assets

Drop files into `assets/` and they are packaged automatically — no CMake edits
needed (the glob uses `CONFIGURE_DEPENDS`, so adding a file is picked up on the
next build).

| | web | desktop |
|---|---|---|
| how | baked into `index.data`, mounted at `/assets` | copied next to the binary |
| by | emscripten `--preload-file` | CMake post-build step |

`InitAssets()` (called first in `main`) anchors the working directory to the
executable's location on desktop, so **the same relative path works on both
platforms**:

```c
Texture2D snake = LoadTexture("assets/images/snake.png");

cJSON *levels = LoadJsonAsset("assets/data/levels.json");
if (levels != NULL) { /* ... */ UnloadJsonAsset(levels); }
```

Notes:

- **Image formats**: raylib is built with PNG, BMP, TGA, GIF and QOI support.
  JPEG is off in raylib's default config — prefer PNG.
- **Editing an asset triggers a relink** on web (`LINK_DEPENDS`), so `index.data`
  can't go stale.
- **An empty `assets/` produces no `index.data`** at all, and the workflow's copy
  step skips it. `.gitkeep` files are filtered out of the glob, so they never get
  packaged.
- Assets are fetched over HTTP on web, so the page must be served (not opened as
  a `file://` URL).

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
