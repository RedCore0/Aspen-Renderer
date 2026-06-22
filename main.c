#include "AspenRenderer/Renderer.h"
#include <math.h>

int main(void) {
    OpenWindow(800, 600, "Renderer Showcase", WINDOW_SCALABLE);

    Camera2D camera = CreateCamera2D();
    camera.offset = (Vec2){0, 100};
    unsigned int CamShader = LoadPostShaderFile("grayscale.frag");

    Image sprite = LoadImage("Textures/Character.png", CLAMP_TO_EDGE);
    Vec2* walkFrames  = MakeFrameGrid(64, 64, 32, 32, 0, 4);
    Animation anim = CreateAnimation(walkFrames, 4, (Vec2){32,32}, 0.2f, true);
    AnimationState state = CreateAnimationState();

    Image container = LoadImage("container.jpg", CLAMP_TO_EDGE);
    Image containerRepeat = LoadImage("container.jpg", REPEAT);

    Image BackGround = LoadImage("uniformclouds.jpg", REPEAT);
    unsigned int backgroundShader = LoadFragmentShaderFile("nebula.frag");
    ApplyShader(&BackGround, backgroundShader);

    Font Spring = LoadFont("SpringMinimalist.ttf", 64.0f);

    float rot = 0;
    float total_time = 0.0f;

    float transparency = 1.0f;
    float r, g, b;
    float barFill = 750;

    while (!WindowShouldClose()) {
        float dt = GetDeltaTime();
        total_time += dt;
        BeginFrame(BLANK);
        DrawBackground(BackGround);
        BeginCamera(&camera);
        if (barFill <= 0 ) {
        }

        DrawSprite(sprite, anim, &state, (Vec2){100,0}, (Vec2){64,64}, 1, SCALE_ABSOLUTE);

        Vec2 ContainerSrc = {0,0};
        Vec2 ContainerPivot = {0.5f, 0.5f};
        Vec2 ContainerDst = {512, 512};

        Vec2 ContainerPos = {200,0};
        Vec2 ContainerScale = {100, 100};
        DrawImage(container, ContainerPos, ContainerScale, ContainerSrc, ContainerDst, 0, ContainerPivot, 1, SCALE_ABSOLUTE);

        rot += dt * 20.0f;
        ContainerPos.x = 400;
        SetImageTint(&container, GREEN);
        DrawImage(container, ContainerPos, ContainerScale, ContainerSrc, ContainerDst, rot, ContainerPivot, 1, SCALE_ABSOLUTE);
        ResetImageTint(&container);

        ContainerPos.x = 600;
        ContainerDst.x = 1024;
        ContainerScale.x = 200;
        DrawImage(containerRepeat, ContainerPos, ContainerScale, ContainerSrc, ContainerDst, 0, ContainerPivot, 1, SCALE_ABSOLUTE);

        Vec2 ShapePos = {100,200};
        ShapeStyle DefaultS = {};
        DrawCircle(ShapePos, 32, RED, 1, DefaultS);

        ShapePos.x = 200;
        DrawRect(ShapePos, (Vec2){64, 64}, GREEN, 1, DefaultS);

        ShapePos.x = 300;
        DrawTriangle(ShapePos, (Vec2){64, 64}, BLUE, 1, DefaultS);

        ShapeStyle OutlineS = {.outline =  true};
        ShapePos.x = 400;
        DrawCircle(ShapePos, 32, ORANGE, 1, OutlineS);

        ShapePos.x = 500;
        DrawRect(ShapePos, (Vec2){64, 64}, CYAN, 1, OutlineS);

        ShapePos.x = 600;
        DrawTriangle(ShapePos, (Vec2){64, 64}, PURPLE, 1, OutlineS);

        ShapePos.x = 100;
        ShapePos.y = 300;
        transparency = (sin(total_time*1.5f)+1)/ 2;
        Color TransparencyShowcase = {1,1,0,transparency};
        DrawRect(ShapePos, (Vec2){64, 64}, TransparencyShowcase, 1, DefaultS);

        ShapePos.x = 200;
        r = (sin(total_time * 1.5f + 0.0f) + 1.0f) / 2.0f;
        g = (sin(total_time * 1.5f + 2.0f) + 1.0f) / 2.0f;
        b = (sin(total_time * 1.5f + 4.0f) + 1.0f) / 2.0f;
        Color rainbow = {r,g,b,1};
        DrawCircle(ShapePos, 32, rainbow, 1, DefaultS);

        ShapePos.x = 300;
        ShapePos.y = 320;
        DrawText(&Spring, "Hello World", ShapePos, 0, 1, WHITE, 1);

        EndCamera();
        if (barFill <= 0) {
            BeginPostProcess(CamShader);
        }else {
            BeginPostProcess(0);
        }

        double wh = GetWindowHeight();
        double ww = GetWindowWidth();
        ShapeStyle BarStyle = {};
        BarStyle.anchor = ANCHOR_LEFT;
        Vec2 BarPos = {20, wh-20};
        Vec2 BarScale = {barFill, 20};
        DrawRect(BarPos, BarScale, GREEN, 1, BarStyle);
        barFill -=  dt * 100.0f;
        if (barFill < 0) {barFill = 0;}

        char array[10];
        sprintf(array, "%.0f", barFill);
        BarPos.y = wh - 40;
        DrawText(&Spring, array, BarPos, 0, 1, WHITE, 1);

        ShapePos.x = ww - 40;
        ShapePos.y = wh - 40;
        DrawCircle(ShapePos, 32, ORANGE, 2, DefaultS);

        EndFrame();
    }
    CloseWindow();
    return 0;
}