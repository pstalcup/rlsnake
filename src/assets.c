#include "assets.h"

#include "raylib.h"

void InitAssets(void)
{
#if !defined(PLATFORM_WEB)
    // A desktop binary can be launched from any working directory, and CMake
    // copies assets/ next to the executable — so anchor there. On web the
    // preloaded files are mounted at /assets and the virtual cwd is already /,
    // which makes the same relative paths resolve correctly.
    ChangeDirectory(GetApplicationDirectory());
#endif
}

cJSON *LoadJsonAsset(const char *path)
{
    char *text = LoadFileText(path);
    if (text == NULL) return NULL; // raylib has already logged the failure

    cJSON *json = cJSON_Parse(text);
    if (json == NULL)
    {
        const char *error = cJSON_GetErrorPtr();
        TraceLog(LOG_WARNING, "JSON: [%s] Failed to parse near: %.32s",
                 path, (error != NULL) ? error : "(unknown)");
    }

    UnloadFileText(text);
    return json;
}

void UnloadJsonAsset(cJSON *json)
{
    cJSON_Delete(json);
}
