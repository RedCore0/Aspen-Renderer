# Aspen Renderer

# Renderer.h — API Reference & Quickstart

Renderer.h provides a single-file C-style rendering API for 2D applications built on GLFW + OpenGL. It wraps window creation, batching, simple shapes, sprite animation, font drawing (stb_truetype), image loading (stb_image) and a simple post-processing pipeline.

This document explains requirements, installation, and the user-facing types and functions in a documentation style so you can quickly integrate the renderer into your project.

---

## Requirements

- GLFW is required (window/context + input). Install GLFW for your platform and ensure the compiler/linker can find it.
- GLAD (or another GL loader) must be available to provide `<glad/glad.h>`.
- The file depends on the included STB headers in `AspenRenderer/deps/` (stb_image.h and stb_truetype.h are provided).
- Shaders and image/font assets are expected to be loaded from files at runtime (path relative to process working directory unless you provide absolute paths).

Important: To install the library into your project, download the `AspenRenderer` folder (the header and `deps` it references) and add it to your project include path. Example:
- Put the folder next to your `src` and compile with `-I./AspenRenderer`.

---

## Build / Link example

Linux (example):
gcc main.c -I./AspenRenderer -lglfw -ldl -lm -lGL -pthread -o app

Windows (MSVC / MinGW):
- Link with glfw3 and opengl32 (and include GLAD sources if not built-in).
- Example (MinGW): gcc main.c -I./AspenRenderer -lglfw3 -lopengl32 -lgdi32 -o app

Notes:
- Ensure GLAD is built/installed or include your GL loader before compiling.
- Place asset files (images, fonts, fragment shaders used with LoadFragmentShaderFile / LoadPostShaderFile) in working directory or use full paths.

---

## High-level overview

Renderer.h aims to be minimal and immediate-mode-friendly:
- OpenWindow / CloseWindow manage the GLFW window and OpenGL context.
- BeginFrame / EndFrame bracket each frame; BeginPostProcess allows applying a post-process shader.
- Batched drawing: images and shapes are batched to reduce draw calls.
- World vs screen space: many calls accept a RenderSpace / scale mode to choose how positions and sizes are interpreted.
- Simple sprite animation and bitmap font rendering are included.

---

## Public types (summary)

- Vec2: { float x, float y } — 2D vector.
- Color: { float r,g,b,a } — color components (0.0–1.0).
- Image: struct for a loaded texture (id, width, height, channels, tint, customShaderId).
- Camera2D: { Vec2 target, offset; float rotation, zoom } — simple 2D camera.
- Animation: frames, frameCount, frameSize, speed, loops.
- AnimationState: tracks playback (currentFrame, lastTime, isPlaying, onComplete).
- Font: { unsigned int textureId; stbtt_bakedchar chars[96]; float size }.
- ShapeStyle: controls outline/solid, thickness, rotation, anchor, shader, scaleMode.
- Anchor enum: e.g., ANCHOR_CENTER, ANCHOR_LEFT, etc.
- RenderScaleMode enum: SCALE_ABSOLUTE, SCALE_RELATIVE_X, etc.
- RenderSpace enum: RENDER_WORLD_SPACE, RENDER_SCREEN_SPACE.
- WrapMode enum: REPEAT, CLAMP_TO_EDGE, etc.
- WindowMode enum: WINDOW_SCALABLE, WINDOW_LOCKED, WINDOW_FULLSCREEN.

---

## User-facing functions (documentation)

All functions are declared in Renderer.h. Below are the commonly used functions, their signatures and behavior.

Window / lifecycle
- bool OpenWindow(int width, int height, const char *title, WindowMode mode)
  - Create an OpenGL window and initialize renderer state.
  - mode: WINDOW_SCALABLE / WINDOW_LOCKED / WINDOW_FULLSCREEN.
  - Returns true on success.

- bool WindowShouldClose()
  - Returns true when window should close (GLFW close requested or ESC pressed).

- void BeginFrame(Color clearColor)
  - Start drawing a new frame. Call before issuing draw commands for this frame.
  - clearColor clears the main FBO.

- void BeginPostProcess(unsigned int postShader)
  - Finish regular drawing and run post-processing pass to an intermediate FBO using postShader (0 uses default pass-through).

- void EndFrame()
  - Finish the frame: composites FBO(s), swaps buffers and polls events. Call at the end of each frame.

- void CloseWindow()
  - Tear down GL resources and close the window. Call once when your app exits.

Delta / window utilities
- float GetDeltaTime()
  - Returns the time in seconds since the last frame.

- int GetWindowWidth(), GetWindowHeight()
  - Current window size (client area).

Camera
- Camera2D CreateCamera2D()
  - Returns a Camera2D initialized with sensible defaults (centered offset).

- void BeginCamera(const Camera2D *cam)
  - Sets an active camera; subsequent world-space draws are projected by it.

- void EndCamera()
  - Deactivates the active camera.

Images & textures
- Image LoadImage(const char *path, WrapMode wrapMode)
  - Loads image from disk using stb_image, creates an OpenGL texture.
  - wrapMode: REPEAT, CLAMP_TO_EDGE, etc.
  - Returns an Image struct (id==0 on failure).

- void UnloadImage(Image image)
  - Frees the GL texture and removes it from tracked textures.

- ImageMap LoadImageFolder(const char *directory, WrapMode wrapMode)
  - Loads all image files in a directory (png/jpg/jpeg/bmp/tga) into an ImageMap structure.
  - Useful for spritesheets separated into files.

- Image GetImage(const ImageMap *map, const char *name)
  - Lookup image by filename stem in an ImageMap.

- Image* GetImagePtr(ImageMap *map, const char *name)
  - Return pointer to image inside map for mutation.

- void FreeImageMap(ImageMap *map)
  - Unloads all textures and clears the map.

Image tint / shader
- void SetImageTint(Image *img, Color color)
  - Set the tint color multiplied with the sampled texture.
- void SetImageAlpha(Image *img, float alpha)
  - Set the tint alpha only.
- void ResetImageTint(Image *img)
  - Reset tint to WHITE (no tint).
- void ApplyShader(Image *img, unsigned int shaderId)
  - Use a specific shader program when drawing this image (custom shaders compiled by LoadFragmentShaderFile / LoadPostShaderFile).
- void ResetImageShader(Image *img)
  - Clear custom shader (back to default).

Image drawing
- void DrawImageEx(Image image, Vec2 pos, Vec2 scale, Vec2 srcPos, Vec2 srcSize, Vec2 texTiling, int layer, float rotDeg, Vec2 pivot, RenderSpace space, RenderScaleMode scaleMode)
  - Full-featured image draw. srcPos/srcSize select a sub-rectangle in pixels inside texture (for atlases).
  - pos: world/screen position (interpreted per space).
  - scale: size (in pixels or relative depending on scaleMode).
  - pivot: normalized [0..1] pivot inside the sprite (0.5,0.5 is center).
  - layer: z-layer for ordering (affects z coordinate).
  - texTiling: multiply UVs to tile texture.
  - rotDeg: rotation in degrees.

- inline helpers:
  - DrawImage(...) — standard world-space image with common defaults.
  - DrawBackground(Image img) — draws the image stretched to window size (screen space).

Shapes & primitives
- DrawRect(Vec2 pos, Vec2 size, Color color, int layer, ShapeStyle s)
- DrawTriangle(Vec2 pos, Vec2 size, Color color, int layer, ShapeStyle s)
- DrawCircle(Vec2 pos, float radius, Color color, int layer, ShapeStyle s)

ShapeStyle fields:
- bool outline — if true draws outline instead of filled shape.
- float thickness — outline thickness (pixels).
- float rotation — degrees rotation.
- Anchor anchor — e.g., ANCHOR_CENTER, ANCHOR_LEFT, etc.
- unsigned int shader — optional shader for shape rendering.
- RenderScaleMode scaleMode — how size scales with window (absolute or relative).

Lines / outlines
- DrawLine(Vec2 start, Vec2 end, float thickness, Color color, int layer)
- RectOutlineInSpace / CircleOutlineInSpace / TriangleOutlineInSpace — internal helpers available through public inline wrappers.

Sprite animation
- Vec2 *MakeFrameGrid(int sheetW, int sheetH, int spriteW, int spriteH, int startFrame, int count)
  - Builds an array of frame coordinates for a grid layout spritesheet. Returns heap-allocated Vec2*; free with FreeFrames.

- void FreeFrames(Vec2 *frames)
  - Free frames returned by MakeFrameGrid.

- Animation CreateAnimation(Vec2 *frames, int count, Vec2 frameSize, float speed, bool loops)
  - Create an Animation descriptor.

- AnimationState CreateAnimationState()
  - Create a state used for playback (current frame/time).

- void DrawSprite(Image sheet, Animation anim, AnimationState *state, Vec2 pos, Vec2 scale, int layer, RenderScaleMode scaleMode)
  - Draw an animation frame from a spritesheet (advances the state internally when playing).
  - state->onComplete can be set (via SetAnimationCallback) for a one-shot callback when animation finishes.

Animation helpers:
- PauseAnimation(AnimationState *s), ResumeAnimation(AnimationState *s), RewindAnimation(AnimationState *s)
- SetAnimationSpeed(Animation *anim, float secondsPerFrame)
- SetAnimationFrame(AnimationState *s, int frame)
- bool IsAnimationDone(const AnimationState *s)
- SetAnimationCallback(AnimationState *s, void (*cb)(void *), void *userData)

Fonts & text
- Font LoadFont(const char *path, float size)
  - Bakes a bitmap font using stb_truetype into a texture atlas used for drawing.
  - Returns a Font with textureId and baked glyphs.

- void DrawText(Font *font, const char *text, Vec2 pos, float rotation, float scale, Color color, int layer)
  - Draw text using baked font; supports rotation and camera/projected coordinates.

Shaders
- unsigned int LoadFragmentShaderFile(const char *fragmentPath)
  - Loads a fragment shader source file and compiles it together with the built-in vertex shader. Returns program id (0 on failure).

- unsigned int LoadPostShaderFile(const char *fragmentPath)
  - Loads a fragment shader used for the post-processing pass (postVertexShaderSource is used internally).

Notes:
- Use `ApplyShader(&img, shaderId)` to render a specific image with a custom shader program.
- For post-processing, call BeginPostProcess(shaderId) with the shader returned from LoadPostShaderFile.

Utilities
- void Cleanup()
  - Cleans up tracked textures (useful if you created textures outside the usual load API).
- Inline helpers: GetWindowWidth/Height, GetDeltaTime.

---

## Typical minimal usage pattern

1. Open window
2. Load assets (images, fonts, shaders)
3. Game loop:
   - BeginFrame(clearColor)
   - BeginCamera(camera) / draw world objects
   - EndCamera()
   - BeginPostProcess(optionalPostShader)
   - Draw UI (DrawImageUI, DrawText, shapes)
   - EndFrame()
4. On exit: CloseWindow()

Small snippet:

```c
OpenWindow(800, 600, "Demo", WINDOW_SCALABLE);
Camera2D cam = CreateCamera2D();
Image img = LoadImage("Textures/sprite.png", CLAMP_TO_EDGE);
Font font = LoadFont("MyFont.ttf", 32.0f);

while (!WindowShouldClose()) {
    BeginFrame(BLANK);
    BeginCamera(&cam);
    DrawImage(img, (Vec2){100,100}, (Vec2){64,64}, (Vec2){0,0}, (Vec2){64,64}, 0, (Vec2){.5,.5}, 1, SCALE_ABSOLUTE);
    EndCamera();
    BeginPostProcess(0); // no post shader
    DrawText(&font, "Hello!", (Vec2){50,50}, 0, 1.0f, WHITE, 2);
    EndFrame();
}
CloseWindow();
```

---

## Tips & gotchas

- Asset paths are interpreted relative to the current working directory — ensure you launch your app from a folder where assets exist or use absolute paths.
- The renderer uses an internal batching strategy: changing active texture or shader flushes the batch. Minimize state changes to get better performance.
- The font loader bakes to a 512x512 bitmap atlas; if you need many sizes or glyphs, consider modifying the bakery call or adding multiple fonts.
- When using custom shaders, ensure they define uniforms expected by the library (e.g., `projection`, `u_time`, `tex`, `screenTexture` depending on usage). The library sets some uniforms automatically when present.
