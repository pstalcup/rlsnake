#ifndef ASSETS_H
#define ASSETS_H

#include "cJSON.h"

// Anchors the working directory so paths relative to assets/ resolve the same
// on desktop and web. Call once at startup, before loading anything.
void InitAssets(void);

// Reads and parses a JSON file from assets/, e.g.
// LoadJsonAsset("assets/data/levels.json"). Returns NULL on a missing file or
// a parse error (both are logged). Free the result with UnloadJsonAsset().
cJSON *LoadJsonAsset(const char *path);
void UnloadJsonAsset(cJSON *json);

#endif // ASSETS_H
