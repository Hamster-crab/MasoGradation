

#include <raylib.h>

int main() {
    InitWindow(1280, 720, "Model Loading");
    
    Model model = LoadModel("resources/OBJ/RubberDuck_LOD0.obj");
    Texture2D tex = LoadTexture("resources/OBJ/RubberDuck_AlbedoTransparency.png");
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tex;
    
    Camera cam = {0};
    cam.position = (Vector3){50.0f,50.0f,50.0f};
    cam.target = (Vector3){0.0f,0.0f,0.0f};
    cam.up = (Vector3){0.0f,1.0f,0.0f};
    cam.fovy = 90.f;
    cam.projection = CAMERA_PERSPECTIVE;
    
    Vector3 pos = {0.0f,0.0f,0.0f};
    Vector3 pos2 = {200.0f,1.0f,0.0f};
    BoundingBox bounds = GetMeshBoundingBox(model.meshes[0]);
    
    SetTargetFPS(60);
    
    //SetCameraMode(cam, CAMERA_THIRD_PERSON);
    
    while(!WindowShouldClose()) {
        //UpdateCamera(&cam);
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(cam);
        DrawModel(model, pos, 1.0f, WHITE);
        DrawModel(model, pos2, 1.0f, WHITE);
        DrawGrid(20, 10.0f);
        DrawBoundingBox(bounds, GREEN);
        EndMode3D();
        DrawText("Loading obj file", 10, GetScreenHeight()-25, 25, DARKGRAY);
        DrawFPS(10,10);
        EndDrawing();
    }
    
    UnloadTexture(tex);
    UnloadModel(model);
    CloseWindow();
    return 0;
    
}
