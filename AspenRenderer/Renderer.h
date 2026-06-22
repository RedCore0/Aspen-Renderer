#ifndef RENDERER_H
#define RENDERER_H

// --- System headers ---
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

// --- GL / windowing ---
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// --- Project headers ---
//#include "Shaders.h"

// --- STB ---
#define STB_IMAGE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "deps/stb_image.h"
#include "deps/stb_truetype.h"

// ============================================================================
// [SECTION] DEFINITIONS & CONFIGURATIONS
// ============================================================================

#define MAX_SPRITES 20000
#define MAX_VERTICES (MAX_SPRITES * 4)
#define MAX_INDICES (MAX_SPRITES * 6)
#define MAX_TRACKED_IMAGES 1024
#define MAX_FOLDER_IMAGES 256
#define IMAGE_NAME_LEN 64

typedef enum { RENDER_WORLD_SPACE, RENDER_SCREEN_SPACE } RenderSpace;

typedef enum {
    SCALE_ABSOLUTE,
    SCALE_RELATIVE_X,
    SCALE_RELATIVE_Y,
    SCALE_RELATIVE_BOTH,
    SCALE_RELATIVE_MIN,
    SCALE_RELATIVE_MAX
} RenderScaleMode;

typedef enum {
    REPEAT = GL_REPEAT,
    MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
    CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
    CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER
} WrapMode;

typedef enum {
    WINDOW_SCALABLE,
    WINDOW_LOCKED,
    WINDOW_FULLSCREEN
} WindowMode;

typedef struct {
    float x;
    float y;
} Vec2;

// ============================================================================
// [SECTION] COLOR
// ============================================================================

typedef struct {
    float r;
    float g;
    float b;
    float a;
} Color;

static const Color RED = {1.00f, 0.18f, 0.18f, 1.0f};
static const Color GREEN = {0.18f, 0.85f, 0.25f, 1.0f};
static const Color BLUE = {0.18f, 0.40f, 1.00f, 1.0f};
static const Color YELLOW = {1.00f, 0.88f, 0.10f, 1.0f};
static const Color ORANGE = {1.00f, 0.55f, 0.10f, 1.0f};
static const Color PURPLE = {0.65f, 0.18f, 1.00f, 1.0f};
static const Color CYAN = {0.10f, 0.95f, 0.90f, 1.0f};
static const Color PINK = {1.00f, 0.40f, 0.72f, 1.0f};
static const Color WHITE = {1.00f, 1.00f, 1.00f, 1.0f};
static const Color BLACK = {0.00f, 0.00f, 0.00f, 1.0f};
static const Color GRAY = {0.50f, 0.50f, 0.50f, 1.0f};
static const Color DARKGRAY = {0.20f, 0.20f, 0.20f, 1.0f};
static const Color BLANK = {0.00f, 0.00f, 0.00f, 0.0f};

static Color ColorRGBA(const float r, const float g, const float b, const float a) { return (Color){r, g, b, a}; }

static Color ColorRGB8(unsigned const char r, unsigned const char g, unsigned const char b, unsigned const char a) {
    return (Color){(float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f};
}

static Color ColorFade(const Color c, const float alpha) { return (Color){c.r, c.g, c.b, alpha}; }

// ============================================================================
// [SECTION] STRUCTS
// ============================================================================

typedef struct {
    float x;
    float y;
    float z;
    float u;
    float v;
    float r;
    float g;
    float b;
    float a;
} Vertex;

typedef struct {
    unsigned int id;
    int width;
    int height;
    int channels;
    Color tint;
    unsigned int customShaderId;
} Image;

typedef struct {
    Vec2 target;
    Vec2 offset;
    float rotation;
    float zoom;
} Camera2D;

typedef struct {
    Vec2 *frames;
    int frameCount;
    Vec2 frameSize;
    float speed;
    bool loops;
} Animation;

typedef struct {
    int currentFrame;
    double lastTime;
    bool isPlaying;

    void (*onComplete)(void *);

    void *onCompleteUserData;
} AnimationState;

typedef struct {
    char name[IMAGE_NAME_LEN];
    Image image;
} ImageEntry;

typedef struct {
    ImageEntry entries[MAX_FOLDER_IMAGES];
    int count;
} ImageMap;

#define MAX_FONTS 16

typedef struct {
    unsigned int textureId;
    stbtt_bakedchar chars[96];
    float size;
} Font;

// ============================================================================
// [SECTION] ENGINE STATE
// ============================================================================

static GLFWwindow *globalWindow = NULL;
unsigned int globalDefaultPostShader = 0;
static unsigned int globalShaderProgram = 0;
static unsigned int globalShapeProgram = 0;
static unsigned int globalCircleProgram = 0;
static unsigned int globalRingProgram = 0;
static unsigned int globalTextProgram = 0;

static unsigned int globalVAO = 0;
static unsigned int globalVBO = 0;
static unsigned int globalEBO = 0;
static unsigned int globalFBO = 0;
static unsigned int globalFBOTexture = 0;
static unsigned int globalPostVAO = 0;
static unsigned int globalPostVBO = 0;
static unsigned int globalPostFBO = 0;
static unsigned int globalPostFBOTexture = 0;

static Vertex globalVertexBatch[MAX_VERTICES];
static int globalVertexCount = 0;
static unsigned int globalActiveTexture = 0;
static unsigned int globalActiveShader = 0;

static const Camera2D *globalActiveCamera = NULL;

static int globalUniProj = -1;
static int globalUniTime = -1;
static unsigned int globalLastCachedShader = 0;

static double globalLastFrameTime = 0.0;
static float globalDeltaTime = 0.0f;

static unsigned int globalTrackedTextures[MAX_TRACKED_IMAGES];
static int globalTrackedTextureCount = 0;

static const char *postVertexShaderSource =
        "#version 330 core\n"
        "layout (location = 0) in vec2 aPos;\n"
        "layout (location = 2) in vec2 aTexCoords;\n"
        "out vec2 TexCoord;\n"
        "out vec4 FragColorOut;\n"
        "void main() {\n"
        "    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
        "    TexCoord = aTexCoords;\n"
        "    FragColorOut = vec4(1.0);\n"
        "}\0";

static const char *defaultPostFragmentShaderSource =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D screenTexture;\n"
        "void main() { FragColor = texture(screenTexture, TexCoord); }\n";

// ============================================================================
// [SECTION] INTERNAL GL UTILITIES
// ============================================================================

static char* ReadFileToString(const char* filePath) {
    FILE* file = fopen(filePath, "rb");
    if (!file) {
        printf("[ERROR] Failed to open source file: %s\n", filePath);
        return NULL;
    }

    // Seek to the end to determine exact file size allocation
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(length + 1);
    if (!buffer) {
        printf("[ERROR] Memory allocation failed for file buffer: %s\n", filePath);
        fclose(file);
        return NULL;
    }

    size_t readBytes = fread(buffer, 1, length, file);
    buffer[readBytes] = '\0'; // Ensure string null-termination

    fclose(file);
    return buffer;
}

static void framebuffer_size_callback(GLFWwindow *window, const int width, const int height) {
    glViewport(0, 0, width, height);
    unsigned const int fbos[2] = {globalFBO, globalPostFBO};
    unsigned const int textures[2] = {globalFBOTexture, globalPostFBOTexture};
    for (int i = 0; i < 2; i++) {
        if (textures[i] == 0) continue;
        glBindFramebuffer(GL_FRAMEBUFFER, fbos[i]);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[i], 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static unsigned int compileShader(unsigned const int type, const char *src) {
    unsigned const int s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
    int ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(s, 512, NULL, log);
        printf("Shader Error: %s\n", log);
    }
    return s;
}

static unsigned int createShaderProgram(const char *vs, const char *fs) {
    unsigned int v = compileShader(GL_VERTEX_SHADER, vs);
    unsigned int f = compileShader(GL_FRAGMENT_SHADER, fs);
    unsigned int p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    GLint ok;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetProgramInfoLog(p, sizeof(log), NULL, log);
        printf("[PROGRAM LINK ERROR]\n%s\n", log);
    }
    glDeleteShader(v);
    glDeleteShader(f);
    return p;
}

const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 2) in vec2 aTexCoord;\n"
    "layout (location = 3) in vec4 aColor;\n"
    "\n"
    "out vec2 TexCoord;\n"
    "out vec4 FragColorOut;\n"
    "\n"
    "uniform mat4 projection;\n"
    "\n"
    "void main() {\n"
    "    // The CPU has already calculated the rotation, scale, and translation.\n"
    "    // We just apply the camera projection matrix directly!\n"
    "    gl_Position = projection * vec4(aPos, 1.0);\n"
    "    TexCoord = aTexCoord;\n"
    "    FragColorOut = aColor;\n"
    "}\0";

const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "\n"
    "in vec2 TexCoord;\n"
    "in vec4 FragColorOut;\n"
    "\n"
    "uniform sampler2D texture1;\n"
    "\n"
    "void main() {\n"
    "    vec4 texElement = texture(texture1, TexCoord);\n"
    "    \n"
    "    // Standard Alpha Clipping for crisp pixel art transparency\n"
    "    if(texElement.a < 0.1) discard;\n"
    "    \n"
    "    // Apply the vertex color tint smoothly\n"
    "    FragColor = texElement * FragColorOut;\n"
    "}\0";

static unsigned int LoadFragmentShaderFile(const char *fragmentPath) {
    char *src = ReadFileToString(fragmentPath);
    if (!src) {
        printf("[ERROR] Failed to load shader: %s\n", fragmentPath);
        return 0;
    }
    unsigned const int id = createShaderProgram(vertexShaderSource, src);
    free(src);
    return id;
}

static unsigned int LoadPostShaderFile(const char *fragmentPath) {
    char *src = ReadFileToString(fragmentPath);
    if (!src) {
        printf("[ERROR] Failed to load post shader: %s\n", fragmentPath);
        return 0;
    }
    unsigned const int id = createShaderProgram(postVertexShaderSource, src);
    free(src);
    return id;
}

// ============================================================================
// [SECTION] SHADER UNIFORM HELPERS
// ============================================================================

// Set a float uniform on any shader program by name.
static void SetShaderFloat(unsigned const int shader, const char *name, const float value) {
    const int loc = glGetUniformLocation(shader, name);
    if (loc != -1) {
        glUseProgram(shader);
        glUniform1f(loc, value);
    }
}

static void SetShaderInt(unsigned const int shader, const char *name, int const value) {
    const int loc = glGetUniformLocation(shader, name);
    if (loc != -1) {
        glUseProgram(shader);
        glUniform1i(loc, value);
    }
}

static void SetShaderVec2(unsigned const int shader, const char *name, const Vec2 value) {
    int const loc = glGetUniformLocation(shader, name);
    if (loc != -1) {
        glUseProgram(shader);
        glUniform2f(loc, value.x, value.y);
    }
}

static void SetShaderColor(unsigned const int shader, const char *name, const Color value) {
    const int loc = glGetUniformLocation(shader, name);
    if (loc != -1) {
        glUseProgram(shader);
        glUniform4f(loc, value.r, value.g, value.b, value.a);
    }
}

inline void ClearFBO(Color const color) {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

// ============================================================================
// [SECTION] BATCH MANAGEMENT
// ============================================================================

static void FlushBatch() {
    if (globalVertexCount == 0) return;

    glUseProgram(globalActiveShader);

    // Only re-query uniform locations when the shader changes
    if (globalActiveShader != globalLastCachedShader) {
        globalUniProj = glGetUniformLocation(globalActiveShader, "projection");
        globalUniTime = glGetUniformLocation(globalActiveShader, "u_time");
        globalLastCachedShader = globalActiveShader;
    }

    if (globalUniProj != -1) {
        int w, h;
        glfwGetWindowSize(globalWindow, &w, &h);
        const float proj[16] = {
            2.0f / (float)w, 0, 0, 0,
            0, -2.0f / (float)h, 0, 0,
            0, 0, -1.0f / 1000.f, 0,
            -1.0f, 1.0f, 0, 1.0f
        };
        glUniformMatrix4fv(globalUniProj, 1, GL_FALSE, proj);
    }
    if (globalUniTime != -1)
        glUniform1f(globalUniTime, (float) glfwGetTime());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, globalActiveTexture);
    const int texLoc = glGetUniformLocation(globalActiveShader, "tex");
    if (texLoc != -1)
        glUniform1i(texLoc, 0);
    const int hasTexLoc = glGetUniformLocation(globalActiveShader, "u_hasTexture");
    if (hasTexLoc != -1)
        glUniform1i(hasTexLoc, globalActiveTexture != 0 ? 1 : 0);
    glBindBuffer(GL_ARRAY_BUFFER, globalVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, globalVertexCount * sizeof(Vertex), globalVertexBatch);
    glBindVertexArray(globalVAO);
    glDrawElements(GL_TRIANGLES, (globalVertexCount / 4) * 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    globalVertexCount = 0;
}

static void SetBatchState(unsigned const  int shader, unsigned const int texture) {
    if (globalActiveShader != shader || globalActiveTexture != texture || globalVertexCount >= MAX_VERTICES) {
        FlushBatch();
        globalActiveShader = shader;
        globalActiveTexture = texture;
    }
}

// ============================================================================
// [SECTION] WINDOW LIFECYCLE
// ============================================================================

bool OpenWindow(int width, int height, const char *title, WindowMode mode) {
    if (!glfwInit()) return false;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor *monitor = NULL;
    if (mode == WINDOW_FULLSCREEN) {
        monitor = glfwGetPrimaryMonitor();
        if (monitor) {
            const GLFWvidmode *v = glfwGetVideoMode(monitor);
            width = v->width;
            height = v->height;
        }
    } else if (mode == WINDOW_LOCKED) {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    } else {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    }

    globalWindow = glfwCreateWindow(width, height, title, monitor, NULL);
    if (!globalWindow) {
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(globalWindow);
    glfwSetFramebufferSizeCallback(globalWindow, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) return false;

    globalShaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    const char *shapeVS =
            "#version 330 core\n"
            "layout (location = 0) in vec3 aPos;\n"
            "layout (location = 2) in vec2 aTexCoords;\n"
            "layout (location = 3) in vec4 aColor;\n"
            "uniform mat4 projection;\n"
            "out vec2 TexCoord;\n"
            "out vec4 FragColorOut;\n"
            "void main() {\n"
            "    gl_Position = projection * vec4(aPos, 1.0);\n"
            "    TexCoord = aTexCoords;\n"
            "    FragColorOut = aColor;\n"
            "}\0";

    const char *flatFS =
            "#version 330 core\n"
            "out vec4 FragColor;\n"
            "in vec4 FragColorOut;\n"
            "void main() { FragColor = FragColorOut; }\0";

    const char *circleFS =
            "#version 330 core\n"
            "out vec4 FragColor;\n"
            "in vec2 TexCoord;\n"
            "in vec4 FragColorOut;\n"
            "void main() {\n"
            "    float dist = length(TexCoord - vec2(0.5));\n"
            "    if (dist > 0.5) discard;\n"
            "    FragColor = FragColorOut;\n"
            "}\0";

    globalShapeProgram = createShaderProgram(shapeVS, flatFS);
    globalCircleProgram = createShaderProgram(shapeVS, circleFS);

    const char *ringFS =
            "#version 330 core\n"
            "out vec4 FragColor;\n"
            "in vec2 TexCoord;\n"
            "in vec4 FragColorOut;\n"
            "uniform float u_innerRadius;\n"
            "void main() {\n"
            "    float dist = length(TexCoord - vec2(0.5));\n"
            "    if (dist > 0.5 || dist < u_innerRadius) discard;\n"
            "    FragColor = FragColorOut;\n"
            "}\0";
    globalRingProgram = createShaderProgram(shapeVS, ringFS);
    globalActiveShader = globalShaderProgram;
    globalActiveTexture = 0;

    const char *textFS =
            "#version 330 core\n"
            "out vec4 FragColor;\n"
            "in vec2 TexCoord;\n"
            "in vec4 FragColorOut;\n"
            "uniform sampler2D tex;\n"
            "void main() {\n"
            "    float alpha = texture(tex, TexCoord).r;\n"
            "    FragColor = vec4(FragColorOut.rgb, FragColorOut.a * alpha);\n"
            "}\0";
    globalTextProgram = createShaderProgram(shapeVS, textFS);


    // Build static EBO index buffer
    unsigned int *idx = malloc(MAX_INDICES * sizeof(unsigned int));
    unsigned int offset = 0;
    for (int i = 0; i < MAX_INDICES; i += 6) {
        idx[i + 0] = offset + 0;
        idx[i + 1] = offset + 1;
        idx[i + 2] = offset + 3;
        idx[i + 3] = offset + 1;
        idx[i + 4] = offset + 2;
        idx[i + 5] = offset + 3;
        offset += 4;
    }

    glGenVertexArrays(1, &globalVAO);
    glGenBuffers(1, &globalVBO);
    glGenBuffers(1, &globalEBO);
    glBindVertexArray(globalVAO);
    glBindBuffer(GL_ARRAY_BUFFER, globalVBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, globalEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(unsigned int), idx, GL_STATIC_DRAW);
    free(idx);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, x));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, u));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, r));
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);

    // FBO
    glGenFramebuffers(1, &globalFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, globalFBO);
    glGenTextures(1, &globalFBOTexture);
    glBindTexture(GL_TEXTURE_2D, globalFBOTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, globalFBOTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("[ERROR] Framebuffer incomplete!\n");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Post FBO
    glGenFramebuffers(1, &globalPostFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, globalPostFBO);
    glGenTextures(1, &globalPostFBOTexture);
    glBindTexture(GL_TEXTURE_2D, globalPostFBOTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, globalPostFBOTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("[ERROR] Post framebuffer incomplete!\n");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Post-process
    float postVerts[] = {-1, 1, 0, 1, 1, 1, 1, 1, 1, -1, 1, 0, -1, -1, 0, 0};
    glGenVertexArrays(1, &globalPostVAO);
    glGenBuffers(1, &globalPostVBO);
    glBindVertexArray(globalPostVAO);
    glBindBuffer(GL_ARRAY_BUFFER, globalPostVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(postVerts), postVerts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, globalEBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    globalDefaultPostShader = createShaderProgram(postVertexShaderSource, defaultPostFragmentShaderSource);
    globalLastFrameTime = glfwGetTime();
    return true;
}

bool WindowShouldClose() { return !globalWindow || glfwWindowShouldClose(globalWindow); }

static void CompositeTexture(unsigned int texture, unsigned int shader) {
    glDisable(GL_DEPTH_TEST);
    unsigned int ps = shader ? shader : globalDefaultPostShader;
    glUseProgram(ps);
    int tLoc = glGetUniformLocation(ps, "u_time");
    if (tLoc != -1)
        glUniform1f(tLoc, (float) glfwGetTime());
    int sLoc = glGetUniformLocation(ps, "screenTexture");
    if (sLoc != -1)
        glUniform1i(sLoc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(globalPostVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

void BeginFrame(Color clearColor) {
    // Update delta time
    double now = glfwGetTime();
    globalDeltaTime = (float) (now - globalLastFrameTime);
    globalLastFrameTime = now;

    // Draw into main FBO
    glBindFramebuffer(GL_FRAMEBUFFER, globalFBO);
    glEnable(GL_DEPTH_TEST);
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    globalVertexCount = 0;
}

void BeginPostProcess(unsigned int postShader) {
    FlushBatch();

    glBindFramebuffer(GL_FRAMEBUFFER, globalPostFBO);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    CompositeTexture(globalFBOTexture, postShader);

    glBindFramebuffer(GL_FRAMEBUFFER, globalPostFBO);
    glEnable(GL_DEPTH_TEST);
    globalActiveCamera = NULL;
}

void EndFrame() {
    if (!globalWindow) return;
    FlushBatch();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    unsigned int srcTexture = (globalPostFBOTexture && glIsTexture(globalPostFBOTexture))
                                  ? globalPostFBOTexture
                                  : globalFBOTexture;
    CompositeTexture(srcTexture, 0);

    if (glfwGetKey(globalWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(globalWindow, GLFW_TRUE);

    glfwSwapBuffers(globalWindow);
    glfwPollEvents();

    glBindFramebuffer(GL_FRAMEBUFFER, globalPostFBO);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, globalFBO);
}

static inline float GetDeltaTime() { return globalDeltaTime; }

static inline int GetWindowWidth() {
    int w, h;
    glfwGetWindowSize(globalWindow, &w, &h);
    return w;
}

static inline int GetWindowHeight() {
    int w, h;
    glfwGetWindowSize(globalWindow, &w, &h);
    return h;
}

void CloseWindow() {
    glDeleteProgram(globalShaderProgram);
    glDeleteProgram(globalShapeProgram);
    glDeleteProgram(globalCircleProgram);
    glDeleteProgram(globalRingProgram);
    glDeleteProgram(globalDefaultPostShader);
    glDeleteProgram(globalTextProgram);
    glDeleteVertexArrays(1, &globalVAO);
    glDeleteBuffers(1, &globalVBO);
    glDeleteBuffers(1, &globalEBO);
    glDeleteFramebuffers(1, &globalFBO);
    glDeleteTextures(1, &globalFBOTexture);
    glDeleteFramebuffers(1, &globalPostFBO);
    glDeleteTextures(1, &globalPostFBOTexture);
    glDeleteVertexArrays(1, &globalPostVAO);
    glDeleteBuffers(1, &globalPostVBO);
    glfwDestroyWindow(globalWindow);
    glfwTerminate();
}

// ============================================================================
// [SECTION] CAMERA
// ============================================================================

Camera2D CreateCamera2D() {
    Camera2D cam;
    cam.target = (Vec2){0, 0};
    cam.offset = (Vec2){(float) GetWindowWidth() * 0.5f, (float) GetWindowHeight() * 0.5f};
    cam.rotation = 0.0f;
    cam.zoom = 1.0f;
    return cam;
}

void BeginCamera(const Camera2D *cam) {
    FlushBatch();
    globalActiveCamera = cam;
}

void EndCamera() {
    FlushBatch();
    globalActiveCamera = NULL;
    globalActiveShader = globalShaderProgram;
}

// ============================================================================
// [SECTION] IMAGE LOADING & UTILITIES
// ============================================================================

Image LoadImage(const char *path, WrapMode wrapMode) {
    Image img = {0};
    img.tint = WHITE;

    stbi_set_flip_vertically_on_load(1);
    int w, h, ch;
    unsigned char *data = stbi_load(path, &w, &h, &ch, 0);
    if (data) {
        unsigned int tid;
        glGenTextures(1, &tid);
        glBindTexture(GL_TEXTURE_2D, tid);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        img.width = w;
        img.height = h;
        img.channels = ch;
        GLenum fmt = (ch == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt,GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        img.id = tid;
        if (globalTrackedTextureCount < MAX_TRACKED_IMAGES)
            globalTrackedTextures[globalTrackedTextureCount++] = tid;
    } else {
        printf("[ERROR] Failed to load image: %s\n", path);
    }
    stbi_image_free(data);
    return img;
}

void UnloadImage(Image image) {
    if (image.id == 0) return;
    glDeleteTextures(1, &image.id);
    for (int i = 0; i < globalTrackedTextureCount; i++) {
        if (globalTrackedTextures[i] == image.id) {
            globalTrackedTextures[i] = globalTrackedTextures[--globalTrackedTextureCount];
            break;
        }
    }
}

void Cleanup() {
    if (globalTrackedTextureCount > 0) {
        glDeleteTextures(globalTrackedTextureCount, globalTrackedTextures);
        globalTrackedTextureCount = 0;
    }
}

// ============================================================================
// [SECTION] IMAGE FOLDER LOADING
// ============================================================================

static void ExtractStem(const char *filename, char *out) {
    // find last slash
    const char *base = filename;
    for (const char *p = filename; *p; p++)
        if (*p == '/' || *p == '\\') base = p + 1;
    int i = 0;
    while (base[i] && base[i] != '.' && i < IMAGE_NAME_LEN - 1) {
        out[i] = base[i];
        i++;
    }
    out[i] = '\0';
}

static bool IsImageExtension(const char *name) {
    const char *ext = strrchr(name, '.');
    if (!ext) return false;
    return strcmp(ext, ".png") == 0 || strcmp(ext, ".jpg") == 0 ||
           strcmp(ext, ".jpeg") == 0 || strcmp(ext, ".bmp") == 0 ||
           strcmp(ext, ".tga") == 0;
}

ImageMap LoadImageFolder(const char *directory, WrapMode wrapMode) {
    ImageMap map = {0};
    char path[512];

#ifdef _WIN32
    WIN32_FIND_DATAA fd;
    snprintf(path, sizeof(path), "%s\\*", directory);
    HANDLE h = FindFirstFileA(path, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        printf("[ERROR] Could not open folder: %s\n", directory);
        return map;
    }
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        if (!IsImageExtension(fd.cFileName)) continue;
        if (map.count >= MAX_FOLDER_IMAGES) {
            printf("[WARN] ImageMap full\n");
            break;
        }
        snprintf(path, sizeof(path), "%s\\%s", directory, fd.cFileName);
        map.entries[map.count].image = LoadImage(path, wrapMode);
        ExtractStem(fd.cFileName, map.entries[map.count].name);
        map.count++;
    } while (FindNextFileA(h, &fd));
    FindClose(h);
#else
    DIR *dir = opendir(directory);
    if (!dir) {
        printf("[ERROR] Could not open folder: %s\n", directory);
        return map;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; // skip . and ..
        if (!IsImageExtension(entry->d_name)) continue;
        if (map.count >= MAX_FOLDER_IMAGES) {
            printf("[WARN] ImageMap full\n");
            break;
        }
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);
        map.entries[map.count].image = LoadImage(path, wrapMode);
        ExtractStem(entry->d_name, map.entries[map.count].name);
        map.count++;
    }
    closedir(dir);
#endif

    printf("[INFO] Loaded %d image(s) from '%s'\n", map.count, directory);
    return map;
}

Image GetImage(const ImageMap *map, const char *name) {
    if (!map) return (Image){0};
    for (int i = 0; i < map->count; i++)
        if (strcmp(map->entries[i].name, name) == 0)
            return map->entries[i].image;
    printf("[WARN] Image '%s' not found in map\n", name);
    return (Image){0};
}

Image *GetImagePtr(ImageMap *map, const char *name) {
    if (!map) return NULL;
    for (int i = 0; i < map->count; i++)
        if (strcmp(map->entries[i].name, name) == 0)
            return &map->entries[i].image;
    return NULL;
}

void FreeImageMap(ImageMap *map) {
    if (!map) return;
    for (int i = 0; i < map->count; i++)
        UnloadImage(map->entries[i].image);
    map->count = 0;
}

void SetImageTint(Image *img, Color color) { if (img) img->tint = color; }
void SetImageAlpha(Image *img, float alpha) { if (img) img->tint.a = alpha; }
void ResetImageTint(Image *img) { if (img) img->tint = WHITE; }
void ApplyShader(Image *img, unsigned int shaderId) { if (img) img->customShaderId = shaderId; }
void ResetImageShader(Image *img) { if (img) img->customShaderId = 0; }

// ============================================================================
// [SECTION] SCALE HELPER
// ============================================================================

static inline Vec2 ApplyScaleMode(Vec2 size, RenderScaleMode mode) {
    float w = (float) GetWindowWidth();
    float h = (float) GetWindowHeight();
    switch (mode) {
        case SCALE_RELATIVE_X: return (Vec2){size.x * w, size.y * w};
        case SCALE_RELATIVE_Y: return (Vec2){size.x * h, size.y * h};
        case SCALE_RELATIVE_BOTH: return (Vec2){size.x * w, size.y * h};
        case SCALE_RELATIVE_MIN: {
            float s = w < h ? w : h;
            return (Vec2){size.x * s, size.y * s};
        }
        case SCALE_RELATIVE_MAX: {
            float s = w > h ? w : h;
            return (Vec2){size.x * s, size.y * s};
        }
        default: return size;
    }
}

// ============================================================================
// [SECTION] INTERNAL VERTEX TRANSFORM HELPER
// ============================================================================

static inline void ApplyCameraToVertex(Vertex *v, float wx, float wy, float layer,
                                       float camCos, float camSin) {
    if (globalActiveCamera) {
        float rx = (wx - globalActiveCamera->target.x) * globalActiveCamera->zoom;
        float ry = (wy - globalActiveCamera->target.y) * globalActiveCamera->zoom;
        v->x = rx * camCos - ry * camSin + globalActiveCamera->offset.x;
        v->y = rx * camSin + ry * camCos + globalActiveCamera->offset.y;
    } else {
        v->x = wx;
        v->y = wy;
    }
    v->z = layer;
}

// ============================================================================
// [SECTION] IMAGE DRAWING
// ============================================================================

void DrawImageEx(Image image, Vec2 pos, Vec2 scale, Vec2 srcPos, Vec2 srcSize, Vec2 texTiling,
                 int layer, float rotDeg, Vec2 pivot, RenderSpace space, RenderScaleMode scaleMode) {
    if (image.id == 0 || image.width == 0 || image.height == 0) return;

    unsigned int shader = image.customShaderId ? image.customShaderId : globalShaderProgram;
    SetBatchState(shader, image.id);

    Vec2 fs = ApplyScaleMode(scale, scaleMode);
    float rad = rotDeg * (3.14159265f / 180.0f);
    float cosA = cosf(rad), sinA = sinf(rad);
    float px = (pivot.x - 0.5f) * fs.x, py = (pivot.y - 0.5f) * fs.y;
    float tx = (px * cosA - py * sinA) - px;
    float ty = (px * sinA + py * cosA) - py;
    float ox = pos.x - tx, oy = pos.y - ty;

    float nX = srcPos.x / (float) image.width, nW = srcSize.x / (float) image.width;
    float nY = srcPos.y / (float) image.height, nH = srcSize.y / (float) image.height;
    float uvs[4][2] = {{nX, nY + nH}, {nX + nW, nY + nH}, {nX + nW, nY}, {nX, nY}};

    float camCos = 1, camSin = 0;
    if (globalActiveCamera && space == RENDER_WORLD_SPACE) {
        float cr = globalActiveCamera->rotation * (3.14159265f / 180.0f);
        camCos = cosf(cr);
        camSin = sinf(cr);
    }

    int base = globalVertexCount;
    for (int i = 0; i < 4; i++) {
        float quad[4][2] = {{-.5f, -.5f}, {.5f, -.5f}, {.5f, .5f}, {-.5f, .5f}};
        float sx = quad[i][0] * fs.x, sy = quad[i][1] * fs.y;
        float wx = (sx * cosA) + (sy * -sinA) + ox;
        float wy = (sx * sinA) + (sy * cosA) + oy;
        if (globalActiveCamera && space == RENDER_WORLD_SPACE)
            ApplyCameraToVertex(&globalVertexBatch[base + i], wx, wy, (float) layer, camCos, camSin);
        else {
            globalVertexBatch[base + i].x = wx;
            globalVertexBatch[base + i].y = wy;
            globalVertexBatch[base + i].z = (float) layer;
        }
        globalVertexBatch[base + i].u = uvs[i][0] * texTiling.x;
        globalVertexBatch[base + i].v = uvs[i][1] * texTiling.y;
        globalVertexBatch[base + i].r = image.tint.r;
        globalVertexBatch[base + i].g = image.tint.g;
        globalVertexBatch[base + i].b = image.tint.b;
        globalVertexBatch[base + i].a = image.tint.a;
    }
    globalVertexCount += 4;
}

static inline void DrawImage(Image img, Vec2 pos, Vec2 scale,
                             Vec2 srcPos, Vec2 srcSize,
                             float rot, Vec2 pivot, int layer, RenderScaleMode sm) {
    DrawImageEx(img, pos, scale, srcPos, srcSize, (Vec2){1, 1}, layer, rot, pivot, RENDER_WORLD_SPACE, sm);
}

static inline void DrawBackground(Image img) {
    float w = (float) GetWindowWidth(), h = (float) GetWindowHeight();
    DrawImageEx(img,
                (Vec2){w * 0.5f, h * 0.5f},
                (Vec2){w, h},
                (Vec2){0, 0},
                (Vec2){(float) img.width, (float) img.height},
                (Vec2){1, 1},
                0, 0.0f, (Vec2){0.5f, 0.5f},
                RENDER_SCREEN_SPACE, SCALE_ABSOLUTE);
}

// ============================================================================
// [SECTION] SHAPE STYLE & ANCHOR
// ============================================================================

typedef enum {
    ANCHOR_CENTER,
    ANCHOR_LEFT,
    ANCHOR_RIGHT,
    ANCHOR_TOP,
    ANCHOR_BOTTOM,
    ANCHOR_TOP_LEFT,
    ANCHOR_TOP_RIGHT,
    ANCHOR_BOTTOM_LEFT,
    ANCHOR_BOTTOM_RIGHT,
} Anchor;

static inline Vec2 AnchorToPivot(const Anchor a) {
    switch (a) {
        case ANCHOR_LEFT: return (Vec2){0.0f, 0.5f};
        case ANCHOR_RIGHT: return (Vec2){1.0f, 0.5f};
        case ANCHOR_TOP: return (Vec2){0.5f, 0.0f};
        case ANCHOR_BOTTOM: return (Vec2){0.5f, 1.0f};
        case ANCHOR_TOP_LEFT: return (Vec2){0.0f, 0.0f};
        case ANCHOR_TOP_RIGHT: return (Vec2){1.0f, 0.0f};
        case ANCHOR_BOTTOM_LEFT: return (Vec2){0.0f, 1.0f};
        case ANCHOR_BOTTOM_RIGHT: return (Vec2){1.0f, 1.0f};
        default: return (Vec2){0.5f, 0.5f};
    }
}

typedef struct {
    bool outline;
    float thickness;
    float rotation;
    Anchor anchor;
    unsigned int shader;
    RenderScaleMode scaleMode;
} ShapeStyle;

// ============================================================================
// [SECTION] SHAPE DRAWING
// ============================================================================

static void DrawRectCore(Vec2 pos, Vec2 size, Color color, float rotDeg, Vec2 pivot,
                         int layer, RenderSpace space, RenderScaleMode sm, unsigned int shaderId) {
    unsigned int prog = shaderId ? shaderId : globalShapeProgram;
    SetBatchState(prog, 0);

    Vec2 fs = ApplyScaleMode(size, sm);
    float rad = rotDeg * (3.14159265f / 180.0f);
    float cosA = cosf(rad), sinA = sinf(rad);
    float px = (pivot.x - 0.5f) * fs.x, py = (pivot.y - 0.5f) * fs.y;
    float tx = (px * cosA - py * sinA) - px, ty = (px * sinA + py * cosA) - py;
    float ox = pos.x - tx - px, oy = pos.y - ty - py;

    float camCos = 1, camSin = 0;
    if (globalActiveCamera && space == RENDER_WORLD_SPACE) {
        float cr = globalActiveCamera->rotation * (3.14159265f / 180.0f);
        camCos = cosf(cr);
        camSin = sinf(cr);
    }

    int base = globalVertexCount;
    for (int i = 0; i < 4; i++) {
        float q[4][2] = {{.5f, .5f}, {.5f, -.5f}, {-.5f, -.5f}, {-.5f, .5f}};
        float sx = q[i][0] * fs.x, sy = q[i][1] * fs.y;
        float wx = (sx * cosA) + (sy * -sinA) + ox;
        float wy = (sx * sinA) + (sy * cosA) + oy;
        if (globalActiveCamera && space == RENDER_WORLD_SPACE)
            ApplyCameraToVertex(&globalVertexBatch[base + i], wx, wy, (float) layer, camCos, camSin);
        else {
            globalVertexBatch[base + i].x = wx;
            globalVertexBatch[base + i].y = wy;
            globalVertexBatch[base + i].z = (float) layer;
        }
        globalVertexBatch[base + i].u = q[i][0] + 0.5f;
        globalVertexBatch[base + i].v = q[i][1] + 0.5f;
        globalVertexBatch[base + i].r = color.r;
        globalVertexBatch[base + i].g = color.g;
        globalVertexBatch[base + i].b = color.b;
        globalVertexBatch[base + i].a = color.a;
    }
    globalVertexCount += 4;
}

static void DrawTriangleCore(Vec2 pos, Vec2 size, Color color, float rotDeg, Vec2 pivot,
                             int layer, RenderSpace space, RenderScaleMode sm, unsigned int shaderId) {
    unsigned int prog = shaderId ? shaderId : globalShapeProgram;
    SetBatchState(prog, 0);

    Vec2 fs = ApplyScaleMode(size, sm);
    float rad = rotDeg * (3.14159265f / 180.0f);
    float cosA = cosf(rad), sinA = sinf(rad);
    float px = (pivot.x - 0.5f) * fs.x, py = (pivot.y - 0.5f) * fs.y;
    float tx = (px * cosA - py * sinA) - px, ty = (px * sinA + py * cosA) - py;
    float ox = pos.x - tx, oy = pos.y - ty;

    float camCos = 1, camSin = 0;
    if (globalActiveCamera && space == RENDER_WORLD_SPACE) {
        float cr = globalActiveCamera->rotation * (3.14159265f / 180.0f);
        camCos = cosf(cr);
        camSin = sinf(cr);
    }

    int base = globalVertexCount;
    for (int i = 0; i < 4; i++) {
        float tri[4][2] = {{0, .5f}, {.5f, -.5f}, {-.5f, -.5f}, {0, .5f}};
        float sx = tri[i][0] * fs.x, sy = tri[i][1] * fs.y;
        float wx = (sx * cosA) + (sy * -sinA) + ox;
        float wy = (sx * sinA) + (sy * cosA) + oy;
        if (globalActiveCamera && space == RENDER_WORLD_SPACE)
            ApplyCameraToVertex(&globalVertexBatch[base + i], wx, wy, (float) layer, camCos, camSin);
        else {
            globalVertexBatch[base + i].x = wx;
            globalVertexBatch[base + i].y = wy;
            globalVertexBatch[base + i].z = (float) layer;
        }
        globalVertexBatch[base + i].u = tri[i][0] + 0.5f;
        globalVertexBatch[base + i].v = tri[i][1] + 0.5f;
        globalVertexBatch[base + i].r = color.r;
        globalVertexBatch[base + i].g = color.g;
        globalVertexBatch[base + i].b = color.b;
        globalVertexBatch[base + i].a = color.a;
    }
    globalVertexCount += 4;
}

static void DrawCircleCore(Vec2 pos, float radius, Color color, int layer,
                           RenderSpace space, RenderScaleMode sm, unsigned int shaderId) {
    float d = radius * 2.0f;
    Vec2 fs = ApplyScaleMode((Vec2){d, d}, sm);
    unsigned int prog = shaderId ? shaderId : globalCircleProgram;
    SetBatchState(prog, 0);

    float camCos = 1, camSin = 0;
    if (globalActiveCamera && space == RENDER_WORLD_SPACE) {
        float cr = globalActiveCamera->rotation * (3.14159265f / 180.0f);
        camCos = cosf(cr);
        camSin = sinf(cr);
    }

    int base = globalVertexCount;
    for (int i = 0; i < 4; i++) {
        float q[4][2] = {{.5f, .5f}, {.5f, -.5f}, {-.5f, -.5f}, {-.5f, .5f}};
        float wx = q[i][0] * fs.x + pos.x;
        float wy = q[i][1] * fs.y + pos.y;
        if (globalActiveCamera && space == RENDER_WORLD_SPACE)
            ApplyCameraToVertex(&globalVertexBatch[base + i], wx, wy, (float) layer, camCos, camSin);
        else {
            globalVertexBatch[base + i].x = wx;
            globalVertexBatch[base + i].y = wy;
            globalVertexBatch[base + i].z = (float) layer;
        }
        globalVertexBatch[base + i].u = q[i][0] + 0.5f;
        globalVertexBatch[base + i].v = q[i][1] + 0.5f;
        globalVertexBatch[base + i].r = color.r;
        globalVertexBatch[base + i].g = color.g;
        globalVertexBatch[base + i].b = color.b;
        globalVertexBatch[base + i].a = color.a;
    }
    globalVertexCount += 4;
}

// ============================================================================
// [SECTION] LINE DRAWING
// ============================================================================

static inline void DrawLineInSpace(Vec2 start, Vec2 end, float t, Color color,
                                   int layer, RenderSpace space) {
    float dx = end.x - start.x, dy = end.y - start.y, len = sqrtf(dx * dx + dy * dy);
    if (len < 0.0001f) return;
    Vec2 c = {(start.x + end.x) * 0.5f, (start.y + end.y) * 0.5f};
    DrawRectCore(c, (Vec2){len, t}, color,
                 atan2f(dy, dx) * (180.0f / 3.14159265f),
                 (Vec2){0.5f, 0.5f}, layer, space, SCALE_ABSOLUTE, 0);
}

static inline void DrawLine(Vec2 start, Vec2 end, float t, Color color, int layer) {
    DrawLineInSpace(start, end, t, color, layer, RENDER_WORLD_SPACE);
}

static inline void RectOutlineInSpace(Vec2 pos, Vec2 size, float t, Color color,
                                      int layer, RenderSpace space) {
    float hw = size.x * 0.5f, hh = size.y * 0.5f;
    Vec2 tl = {pos.x - hw, pos.y - hh};
    Vec2 tr = {pos.x + hw, pos.y - hh};
    Vec2 bl = {pos.x - hw, pos.y + hh};
    Vec2 br = {pos.x + hw, pos.y + hh};
    DrawLineInSpace(tl, tr, t, color, layer, space);
    DrawLineInSpace(tr, br, t, color, layer, space);
    DrawLineInSpace(br, bl, t, color, layer, space);
    DrawLineInSpace(bl, tl, t, color, layer, space);
}

static inline void CircleOutlineInSpace(Vec2 pos, float radius, float thickness,
                                        Color color, int layer, RenderSpace space) {
    FlushBatch();
    float inner = (radius - thickness) / radius * 0.5f;
    glUseProgram(globalRingProgram);
    glUniform1f(glGetUniformLocation(globalRingProgram, "u_innerRadius"), inner);
    DrawCircleCore(pos, radius, color, layer, space, SCALE_ABSOLUTE, globalRingProgram);
    FlushBatch();
}

static inline void TriangleOutlineInSpace(Vec2 pos, Vec2 size, float t, Color color,
                                          int layer, RenderSpace space) {
    float hw = size.x * 0.5f, hh = size.y * 0.5f;
    Vec2 top = {pos.x, pos.y - hh};
    Vec2 left = {pos.x - hw, pos.y + hh};
    Vec2 right = {pos.x + hw, pos.y + hh};
    DrawLineInSpace(top, right, t, color, layer, space);
    DrawLineInSpace(right, left, t, color, layer, space);
    DrawLineInSpace(left, top, t, color, layer, space);
}

// ============================================================================
// [SECTION] PUBLIC SHAPE API  (two functions per shape: world + UI)
// ============================================================================

static inline void DrawRect(Vec2 pos, Vec2 size, Color color, int layer, ShapeStyle s) {
    Vec2 pivot = AnchorToPivot(s.anchor);
    if (s.outline) {
        RectOutlineInSpace(pos, ApplyScaleMode(size, s.scaleMode),
                           s.thickness > 0 ? s.thickness : 1.0f, color, layer, RENDER_WORLD_SPACE);
    } else {
        DrawRectCore(pos, size, color, s.rotation, pivot, layer, RENDER_WORLD_SPACE, s.scaleMode, s.shader);
    }
}

static inline void DrawTriangle(Vec2 pos, Vec2 size, Color color, int layer, ShapeStyle s) {
    Vec2 pivot = AnchorToPivot(s.anchor);
    if (s.outline) {
        TriangleOutlineInSpace(pos, ApplyScaleMode(size, s.scaleMode),
                               s.thickness > 0 ? s.thickness : 1.0f, color, layer, RENDER_WORLD_SPACE);
    } else {
        DrawTriangleCore(pos, size, color, s.rotation, pivot, layer, RENDER_WORLD_SPACE, s.scaleMode, s.shader);
    }
}

static inline void DrawCircle(Vec2 pos, float radius, Color color, int layer, ShapeStyle s) {
    if (s.outline) {
        CircleOutlineInSpace(pos, ApplyScaleMode((Vec2){radius, radius}, s.scaleMode).x,
                             s.thickness > 0 ? s.thickness : 1.0f, color, layer, RENDER_WORLD_SPACE);
    } else {
        DrawCircleCore(pos, radius, color, layer, RENDER_WORLD_SPACE, s.scaleMode, s.shader);
    }
}

// ============================================================================
// [SECTION] ANIMATION
// ============================================================================

Vec2 *MakeFrameGrid(int sheetW, int sheetH, int spriteW, int spriteH,
                    int startFrame, int count) {
    Vec2 *frames = malloc((size_t) count * sizeof(Vec2));
    if (!frames) return NULL;
    int cols = sheetW / spriteW;
    for (int i = 0; i < count; i++) {
        int fi = startFrame + i;
        frames[i].x = (float) ((fi % cols) * spriteW);
        frames[i].y = (float) ((fi / cols) * spriteH);
    }
    return frames;
}

static inline void FreeFrames(Vec2 *frames) { free(frames); }

Animation CreateAnimation(Vec2 *frames, int count, Vec2 frameSize, float speed, bool loops) {
    return (Animation){frames, count, frameSize, speed, loops};
}

AnimationState CreateAnimationState(void) {
    return (AnimationState){0, glfwGetTime(), true, NULL, NULL};
}

static inline void PauseAnimation(AnimationState *s) {
    if (s) s->isPlaying = false;
}

static inline void ResumeAnimation(AnimationState *s) {
    if (s) {
        s->isPlaying = true;
        s->lastTime = glfwGetTime();
    }
}

static inline void RewindAnimation(AnimationState *s) {
    if (s) {
        s->currentFrame = 0;
        s->lastTime = glfwGetTime();
        s->isPlaying = true;
    }
}

static inline void SetAnimationSpeed(Animation *anim, float secondsPerFrame) {
    if (anim) anim->speed = secondsPerFrame;
}

static inline void SetAnimationFrame(AnimationState *s, int frame) {
    if (s) {
        s->currentFrame = frame;
        s->lastTime = glfwGetTime();
    }
}

static inline bool IsAnimationDone(const AnimationState *s) {
    return s && !s->isPlaying;
}

static inline void SetAnimationCallback(AnimationState *s, void (*cb)(void *), void *userData) {
    if (s) {
        s->onComplete = cb;
        s->onCompleteUserData = userData;
    }
}

void DrawSprite(Image sheet, Animation anim, AnimationState *state,
                Vec2 pos, Vec2 scale, int layer, RenderScaleMode scaleMode) {
    if (!state || anim.frameCount <= 0) return;

    if (state->isPlaying) {
        double now = glfwGetTime();
        double elapsed = now - state->lastTime;
        if (elapsed >= (double) anim.speed) {
            int steps = (int) (elapsed / (double) anim.speed);
            state->currentFrame += steps;
            state->lastTime += steps * (double) anim.speed;
            if (state->currentFrame >= anim.frameCount) {
                if (anim.loops) {
                    state->currentFrame %= anim.frameCount;
                } else {
                    state->currentFrame = anim.frameCount - 1;
                    state->isPlaying = false;
                    if (state->onComplete) state->onComplete(state->onCompleteUserData);
                }
            }
        }
    }

    Vec2 srcPos = anim.frames[state->currentFrame];
    Vec2 srcSize = anim.frameSize;
    DrawImage(sheet, pos, scale, srcPos, srcSize, 0.0f, (Vec2){.5f, .5f}, layer, scaleMode);
}

// ============================================================================
// [SECTION] FONT LOADING & TEXT
// ============================================================================

Font LoadFont(const char *path, float size) {
    Font font = {0};
    font.size = size;

    unsigned char *ttfData = (unsigned char *) ReadFileToString(path);
    if (!ttfData) {
        printf("[ERROR] Failed to load font: %s\n", path);
        return font;
    }

    unsigned char bitmap[512 * 512];
    stbtt_BakeFontBitmap(ttfData, 0, size, bitmap, 512, 512, 32, 96, font.chars);
    free(ttfData);

    glGenTextures(1, &font.textureId);
    glBindTexture(GL_TEXTURE_2D, font.textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (globalTrackedTextureCount < MAX_TRACKED_IMAGES)
        globalTrackedTextures[globalTrackedTextureCount++] = font.textureId;

    return font;
}

static inline void DrawTextCore(Font *font, const char *text, Vec2 pos,
                                float rotation, float scale, Color color, int layer) {
    if (!font || !text || font->textureId == 0) return;
    SetBatchState(globalTextProgram, font->textureId);

    float rad = rotation * (3.14159265f / 180.0f);
    float cosA = cosf(rad), sinA = sinf(rad);
    float rawCx = pos.x / scale, rawCy = pos.y / scale;

    float camCos = 1, camSin = 0;
    if (globalActiveCamera) {
        float cr = globalActiveCamera->rotation * (3.14159265f / 180.0f);
        camCos = cosf(cr);
        camSin = sinf(cr);
    }

    while (*text) {
        if (*text < 32 || *text > 126) {
            text++;
            continue;
        }
        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(font->chars, 512, 512, *text - 32, &rawCx, &rawCy, &q, 1);

        float vx[4] = {q.x0, q.x1, q.x1, q.x0};
        float vy[4] = {q.y0, q.y0, q.y1, q.y1};
        float vu[4] = {q.s0, q.s1, q.s1, q.s0};
        float vv[4] = {q.t0, q.t0, q.t1, q.t1};

        int base = globalVertexCount;
        for (int i = 0; i < 4; i++) {
            float lx = vx[i] * scale - pos.x;
            float ly = vy[i] * scale - pos.y;
            float wx = lx * cosA - ly * sinA + pos.x;
            float wy = lx * sinA + ly * cosA + pos.y;

            if (globalActiveCamera)
                ApplyCameraToVertex(&globalVertexBatch[base + i], wx, wy, (float) layer, camCos, camSin);
            else {
                globalVertexBatch[base + i].x = wx;
                globalVertexBatch[base + i].y = wy;
                globalVertexBatch[base + i].z = (float) layer;
            }
            globalVertexBatch[base + i].u = vu[i];
            globalVertexBatch[base + i].v = vv[i];
            globalVertexBatch[base + i].r = color.r;
            globalVertexBatch[base + i].g = color.g;
            globalVertexBatch[base + i].b = color.b;
            globalVertexBatch[base + i].a = color.a;
        }
        globalVertexCount += 4;
        text++;
    }
}

static inline void DrawText(Font *font, const char *text, Vec2 pos,
                            float rotation, float scale, Color color, int layer) {
    DrawTextCore(font, text, pos, rotation, scale, color, layer);
}

#endif // RENDERER_H
