#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <vector>
#include <iostream>
#include <cmath>

// Constants
const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 600;
const std::string WINDOW_TITLE = "SFML Simple Drawing App";
const sf::Color LINE_COLOR = sf::Color::Blue;
const float LINE_THICKNESS = 5.0f; // Represents the radius of the line points

// Structure to hold a single continuous line segment (a stroke)
struct Stroke
{
    // We store the vertices as individual points (sf::Points) for tracking
    // But we will render them as a custom geometry (quads) for thickness
    sf::VertexArray vertices;
    sf::Color color;
    float thickness;

    Stroke(sf::Color c, float t) : vertices(sf::Points), color(c), thickness(t) {}

    void addPoint(const sf::Vector2f &point)
    {
        // Add the point to the VertexArray (stored as points, will be drawn as thick lines)
        vertices.append(sf::Vertex(point, color));
    }

    void draw(sf::RenderWindow &window) const
    {
        // --- Custom Thick Line Rendering ---

        // This method draws rectangles and circles between points to create a continuous, solid line.

        if (vertices.getVertexCount() < 2)
        {
            // Nothing to draw yet
            return;
        }

        // Draw the initial point as a circle (end cap)
        sf::CircleShape start_cap(thickness / 2.0f);
        start_cap.setFillColor(color);
        start_cap.setPosition(vertices[0].position - sf::Vector2f(thickness / 2.0f, thickness / 2.0f));
        window.draw(start_cap);

        // Loop through all adjacent pairs of points
        for (size_t i = 0; i < vertices.getVertexCount() - 1; ++i)
        {
            sf::Vector2f p1 = vertices[i].position;
            sf::Vector2f p2 = vertices[i + 1].position;

            sf::Vector2f direction = p2 - p1;
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            // Skip if the points are the same
            if (length < 0.001f)
                continue;

            // Normalize direction vector
            direction /= length;

            // Calculate the perpendicular vector for thickness
            sf::Vector2f normal(-direction.y, direction.x);
            normal *= (thickness / 2.0f); // Half the thickness

            // Define the four corners of the rectangle (quad)
            sf::Vertex quad[4];
            quad[0] = sf::Vertex(p1 - normal, color);
            quad[1] = sf::Vertex(p2 - normal, color);
            quad[2] = sf::Vertex(p2 + normal, color);
            quad[3] = sf::Vertex(p1 + normal, color);

            // Draw the line segment as a quad
            window.draw(quad, 4, sf::Quads);

            // Draw a circle at the end point (joint/end cap) for roundness
            sf::CircleShape end_cap(thickness / 2.0f);
            end_cap.setFillColor(color);
            // Center the circle on p2
            end_cap.setPosition(p2 - sf::Vector2f(thickness / 2.0f, thickness / 2.0f));
            window.draw(end_cap);
        }
    }
};

int main()
{
    // 1. Setup the window
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE);
    window.setFramerateLimit(60);

    // 2. Data structures for drawing
    std::vector<Stroke> completed_strokes; // All finished strokes
    Stroke *current_stroke = nullptr;      // The stroke currently being drawn
    bool is_drawing = false;

    std::cout << WINDOW_TITLE << " Initialized." << std::endl;
    std::cout << "Drag the mouse to draw. Press 'C' to clear the canvas." << std::endl;

    // --- Main application loop ---
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {

            // Handle window closing
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            // Handle key press (e.g., 'C' for Clear)
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::C)
                {
                    completed_strokes.clear();
                    if (current_stroke)
                    {
                        current_stroke->vertices.clear();
                    }
                    std::cout << "Canvas cleared." << std::endl;
                }
            }

            // --- Mouse Event Handling ---

            // Start Drawing (Mouse Button Pressed)
            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    is_drawing = true;
                    // Start a new stroke
                    current_stroke = new Stroke(LINE_COLOR, LINE_THICKNESS);
                    sf::Vector2f start_pos((float)event.mouseButton.x, (float)event.mouseButton.y);
                    current_stroke->addPoint(start_pos);
                }
            }

            // Continue Drawing (Mouse Moved)
            if (event.type == sf::Event::MouseMoved && is_drawing && current_stroke)
            {
                sf::Vector2f current_pos((float)event.mouseMove.x, (float)event.mouseMove.y);

                // Optimization: only add a new point if it's far enough from the last one
                if (current_stroke->vertices.getVertexCount() > 0)
                {
                    // FIX: Use operator[] to get the last vertex
                    sf::Vector2f last_pos = current_stroke->vertices[current_stroke->vertices.getVertexCount() - 1].position;
                    // Calculate distance squared (faster than calculating distance)
                    float dx = current_pos.x - last_pos.x;
                    float dy = current_pos.y - last_pos.y;
                    if ((dx * dx + dy * dy) > (LINE_THICKNESS * LINE_THICKNESS / 4.0f))
                    {
                        current_stroke->addPoint(current_pos);
                    }
                }
                else
                {
                    current_stroke->addPoint(current_pos);
                }
            }

            // Stop Drawing (Mouse Button Released)
            if (event.type == sf::Event::MouseButtonReleased)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    is_drawing = false;

                    // Finalize the current stroke and save it
                    if (current_stroke && current_stroke->vertices.getVertexCount() > 1)
                    {
                        completed_strokes.push_back(*current_stroke);
                    }
                    // Clean up the temporary pointer
                    delete current_stroke;
                    current_stroke = nullptr;
                }
            }
        }

        // 3. Rendering
        window.clear(sf::Color::White); // Clear the window to white

        // Draw all completed strokes
        for (const auto &stroke : completed_strokes)
        {
            stroke.draw(window);
        }

        // Draw the current (in-progress) stroke
        if (current_stroke)
        {
            current_stroke->draw(window);
        }

        window.display(); // Swap buffers and display the content
    }

    // Clean up if the program is closed while drawing a stroke
    if (current_stroke)
    {
        delete current_stroke;
    }

    return 0;
}
