#include "raylib.h"
#include <vector>

// Structure to represent a line segment
struct Line
{
    Vector2 start;
    Vector2 end;
    Color color;
};

int main(void)
{
    // Initialization
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Raylib Drawing App - Continuous Lines");

    std::vector<Line> drawingLines;     // Stores all drawn line segments
    Color currentColor = BLACK;         // Current drawing color
    Vector2 lastMousePosition = {0, 0}; // Stores the mouse position from the previous frame

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Vector2 currentMousePosition = GetMousePosition();

            // If this is the first time the mouse button is pressed in a new line,
            // we don't have a 'last' position to draw from. We just set the last position.
            if (lastMousePosition.x != 0 || lastMousePosition.y != 0)
            {
                // Create a new line segment from the last position to the current one
                Line newLine;
                newLine.start = lastMousePosition;
                newLine.end = currentMousePosition;
                newLine.color = currentColor;
                drawingLines.push_back(newLine);
            }

            // Update the last position for the next frame
            lastMousePosition = currentMousePosition;
        }
        else // If the mouse button is not down, reset the last position
        {
            lastMousePosition = {0, 0};
        }

        // Change color with number keys
        if (IsKeyPressed(KEY_ONE))
            currentColor = BLACK;
        if (IsKeyPressed(KEY_TWO))
            currentColor = RED;
        if (IsKeyPressed(KEY_THREE))
            currentColor = BLUE;
        if (IsKeyPressed(KEY_FOUR))
            currentColor = GREEN;

        // Drawing
        BeginDrawing();
        ClearBackground(BLACK);

        // Draw all stored line segments
        for (const auto &line : drawingLines)
        {
            DrawLineEx(line.start, line.end, 5, line.color); // Use DrawLineEx for a thicker line
        }

        EndDrawing();
    }

    // De-Initialization
    CloseWindow(); // Close window and unload OpenGL context

    return 0;
}