#include <raylib.h>
#include <cmath>  // sinfおよびcosfを使用するために必要

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Maso Gradation");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 2.0f, 4.0f };
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    float cameraSpeed = 0.2f;
    float cameraRotationY = 0.0f; // カメラの水平回転角度
    float cameraRotationX = 0.0f; // カメラの垂直回転角度
    const float mouseSensitivity = 0.003f; // マウス感度

    bool sampleZimen = false;
    bool title = true;

    bool setting = false;

    Rectangle startButton = { 300, 360, 230, 80 };
    Rectangle startSetting = { 350, 450, 123, 65 };
    Rectangle startEnd = { 350, 528, 123, 65 };

    Image BackImageSample = LoadImage("resources/Title/BackImage/BackImage.png");
    ImageResize(&BackImageSample, 800, 600);
    Texture2D BackTextureSample = LoadTextureFromImage(BackImageSample);
    UnloadImage(BackImageSample);


    SetTargetFPS(60);

    while (!WindowShouldClose())        // ESCで強制終了
    {
        if (title)
        {
            Vector2 mousePoint = GetMousePosition();
            // マウス座標を小さな矩形に変換
            Rectangle mouseRect = { mousePoint.x, mousePoint.y, 1, 1 };
            // 当たり判定
            bool callStart = CheckCollisionRecs(startButton, mouseRect);
            bool callSetting = CheckCollisionRecs(startSetting, mouseRect);
            bool callEnd = CheckCollisionRecs(startEnd, mouseRect);

            BeginDrawing();
            ClearBackground(RAYWHITE);

            //DrawTexture(BackTexture, 0, 0, WHITE);
            //DrawTexture(BackTextureLogo, 0, 0, WHITE);
            //DrawTexture(BackTextureButton, 0, 0, WHITE);
            DrawTexture(BackTextureSample, 0, 0, WHITE);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                if (callStart) title = false;
                if (callSetting) setting = true;
                if (callEnd) break;
            }

            if (setting)
            {
                DrawRectangle(0, 0 ,screenWidth, screenHeight, GRAY);
                
                DrawText("volume", 5, 50, 50, WHITE);
                DrawRectangle (200, 50, 50, 50, BLACK);
                DrawRectangle (205, 56, 40, 38, WHITE);
            }

            EndDrawing();
        }
        else if (!title)
        {
            DisableCursor();  // マウスカーソルを非表示にする
            Vector2 mouseDelta = GetMouseDelta();

            cameraRotationY -= mouseDelta.x * mouseSensitivity; // 水平回転の符号を反転
            cameraRotationX -= mouseDelta.y * mouseSensitivity; // 垂直回転の符号を反転


            // 垂直回転の制限
            if (cameraRotationX > PI/2.0f - 0.01f) cameraRotationX = PI/2.0f - 0.01f;
            if (cameraRotationX < -PI/2.0f + 0.01f) cameraRotationX = -PI/2.0f + 0.01f;

            // カメラの移動
            Vector3 forward = { sinf(cameraRotationY), 0.0f, cosf(cameraRotationY) };
            Vector3 right = { cosf(cameraRotationY), 0.0f, -sinf(cameraRotationY) };

            if (IsKeyDown(KEY_W)) {
                camera.position.x += forward.x * cameraSpeed;
                camera.position.z += forward.z * cameraSpeed;
            }
            if (IsKeyDown(KEY_S)) {
                camera.position.x -= forward.x * cameraSpeed;
                camera.position.z -= forward.z * cameraSpeed;
            }
            if (IsKeyDown(KEY_A)) {
                camera.position.x += right.x * cameraSpeed;
                camera.position.z += right.z * cameraSpeed;
            }
            if (IsKeyDown(KEY_D)) {
                camera.position.x -= right.x * cameraSpeed;
                camera.position.z -= right.z * cameraSpeed;
            }
            if (IsKeyDown(KEY_SPACE)) camera.position.y += cameraSpeed;
            if (IsKeyDown(KEY_LEFT_SHIFT)) camera.position.y -= cameraSpeed;

            if (IsKeyDown(KEY_F3)) if (IsKeyPressed(KEY_B))
            {
                if (sampleZimen)
                {
                    sampleZimen = false;
                }
                else if (!sampleZimen)
                {
                    sampleZimen = true;
                }
            }

            // カメラのターゲット位置を更新
            camera.target = (Vector3){
                camera.position.x + cosf(cameraRotationX) * sinf(cameraRotationY),
                camera.position.y + sinf(cameraRotationX),
                camera.position.z + cosf(cameraRotationX) * cosf(cameraRotationY)
            };


            BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

            DrawCube((Vector3){ 1.0f, 0.5f, 0.0f }, 1.0f, 1.0f, 1.0f, RED);
            DrawCubeWires((Vector3){ 1.0f, 0.5f, 0.0f }, 1.0f, 1.0f, 1.0f, MAROON);

            if (sampleZimen)
            {
                //グリッドの描画
                for (int x = -10; x <= 10; x++) {
                    for (int z = -10; z <= 10; z++) {
                        Color color = ((x + z) % 2 == 0) ? LIGHTGRAY : WHITE;
                        DrawCube((Vector3){ static_cast<float>(x), 0.0f, static_cast<float>(z) }, 1.0f, 0.1f, 1.0f, color);
                    }
                }
            }

                EndMode3D();

                // カメラ位置の表示
                DrawText(TextFormat("Camera Position: [%.2f, %.2f, %.2f]", camera.position.x, camera.position.y, camera.position.z), 10, 10, 20, DARKGRAY);

            EndDrawing();
            }
    }

    CloseWindow();

    return 0;
}
