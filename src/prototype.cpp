#include <SFML/Graphics.hpp>
#include <iostream>
#include <cassert>
#include <math.h>

using namespace sf;

const int MAP_W = 10;
const int MAP_H = 10;

const auto PI = acos(-1.0);

enum TileType
{
    EMPTY,
    BLOCKADE
};

//map data, 1 represents wall, 0 - no wall
int map[MAP_W][MAP_H] = 
{
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 0, 0, 0, 1, 1, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 1, 0, 0, 1, 0, 1 },
    { 1, 0, 0, 0, 1, 1, 1, 1, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 1, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 1, 0, 0, 1 },
    { 1, 0, 1, 0, 0, 1, 0, 1, 0, 1 },
    { 1, 0, 1, 0, 0, 1, 0, 0, 0, 1 },
    { 1, 1, 1, 1, 1, 1, 0, 1, 1, 1 },
};

sf::Vector2i playerMapPos = { 5, 5 };
sf::Vector2f playerWorldPos;

sf::Vector2f tileSize;

//check window events
void checkEvents(RenderWindow & gameWindow)
{
    Event event;
    while (gameWindow.pollEvent(event))
    {
        if (event.type == Event::Closed)
            gameWindow.close();
    }
}

//update player movement
void updateSteering()
{
    float speed = 5.0f;

    if (Keyboard::isKeyPressed(Keyboard::Key::W))
    {
        playerWorldPos -= sf::Vector2f{ 0.0f, 1.0f } *speed;
    }

    if (Keyboard::isKeyPressed(Keyboard::Key::S))
    {
        playerWorldPos += sf::Vector2f{ 0.0f, 1.0f } *speed;
    }

    if (Keyboard::isKeyPressed(Keyboard::Key::A))
    {
        playerWorldPos -= sf::Vector2f{ 1.0f, 0.0f } *speed;
    }

    if (Keyboard::isKeyPressed(Keyboard::Key::D))
    {
        playerWorldPos += sf::Vector2f{ 1.0f, 0.0f } *speed;
    }

    playerMapPos = { (int)(playerWorldPos.x / tileSize.x), (int)(playerWorldPos.y / tileSize.y) };
}

//get raycast closest hit point
sf::Vector2f getDistToClosestHitPoint(float angle, sf::Vector2i rayMapPos, sf::Vector2f rayWorldPos)
{
    sf::Vector2f rayDir = { cos(angle), sin(angle) };
    
    float dyh = 0; //dist y to next horizontal tile
    float dxh = 0; //dist x to next horizontal tile

    if (rayWorldPos.y == rayMapPos.y * tileSize.y)
    {
        dyh = tileSize.y;
    }
    else
    {
        if (rayDir.y < 0) dyh = rayWorldPos.y - (rayMapPos.y * tileSize.y);
        else           dyh = (rayMapPos.y + 1) * tileSize.y - rayWorldPos.y;
    }
     
    dxh = dyh / tan(angle);
    if (rayDir.y < 0) //invert distances values when pointing upwards
    {
        dxh = -dxh;
        dyh = -dyh;
    }

    float dyv = 0; //dist y to next vertical tile
    float dxv = 0; //dist x to next vertical tile

    if (rayWorldPos.x == rayMapPos.x * tileSize.x)
    {
        dxv = tileSize.x;
    }
    else
    {
        if (rayDir.x < 0) dxv = rayWorldPos.x - (rayMapPos.x * tileSize.x);
        else           dxv = (rayMapPos.x + 1) * tileSize.x - rayWorldPos.x;
    }

    dyv = dxv * tan(angle);
    if (rayDir.x < 0) //invert distances values when pointing upwards
    {
        dxv = -dxv;
        dyv = -dyv;
    }

    //calc squares and compare them
    float sqrLenHor = dxh * dxh + dyh * dyh;
    float sqrLenVer = dxv * dxv + dyv * dyv;

    //select distances which squares are lower
    float dx = sqrLenHor < sqrLenVer ? dxh : dxv;
    float dy = sqrLenHor < sqrLenVer ? dyh : dyv;

    return { dx, dy };
}

void drawMap(RenderWindow & gameWindow)
{
    for (int y = 0; y < MAP_H; y++)
    {
        for (int x = 0; x < MAP_W; x++)
        {
            RectangleShape tile(tileSize);
            tile.setPosition(x * tileSize.x, y * tileSize.y);
            tile.setOutlineThickness(1.0f);
            tile.setOutlineColor(sf::Color::Black);

            //we need to check by [y][x] to draw correctly because of array structure
            if (map[y][x] == TileType::BLOCKADE)
            {
                //if map[y][x] is blockade, make it black
                tile.setFillColor(sf::Color::Black);
                tile.setOutlineColor(sf::Color::White);
            }

            gameWindow.draw(tile);
        }
    }
}

void visualizePlayerRaycast(RenderWindow & gameWindow)
{
    //draw line going from player position to next hit positions
    VertexArray hitLines(LinesStrip);
    hitLines.append({ playerWorldPos, Color::Red });

    //get mouse pos
    auto mousePos = Mouse::getPosition(gameWindow);

    //get player rotation angle and direction vector
    float angle = atan2(mousePos.y - playerWorldPos.y, mousePos.x - playerWorldPos.x);
    sf::Vector2f dir = { cos(angle), sin(angle) };

    //get distance to first hit point
    sf::Vector2f dist = getDistToClosestHitPoint(angle, playerMapPos, playerWorldPos);

    //first ray hit position coordinates
    sf::Vector2f rayWorldPos = { playerWorldPos.x + dist.x, playerWorldPos.y + dist.y };
    sf::Vector2i rayPosMap = { int(rayWorldPos.x / tileSize.x), int(rayWorldPos.y / tileSize.y) }; //just divide world coordinates by tile size

    bool hit = false;

    //raycast loop
    while (!hit)
    {
        //drawing ray hit lines
        hitLines.append({ { rayWorldPos.x, rayWorldPos.y }, sf::Color::Red });

        //drawing hit point circles
        CircleShape hitPoint(5);
        hitPoint.setOrigin({ 5, 5 });
        hitPoint.setPosition({ rayWorldPos.x, rayWorldPos.y });
        hitPoint.setFillColor(sf::Color::Red);
        gameWindow.draw(hitPoint);

        //out of array range exceptions handling
        if (rayPosMap.x < 0 || rayPosMap.x >= MAP_W || rayPosMap.y < 0 || rayPosMap.y >= MAP_H) break;

        //checking that actually hit side is wall side
        int hitTileX = rayPosMap.x;
        int hitTileY = rayPosMap.y;

        //fix checking walls when hit them on their right or bottom side, check walls earlier them
        if (rayWorldPos.x == rayPosMap.x * tileSize.x && dir.x < 0) //hit wall left side
        {
            hitTileX--;
        }

        if (rayWorldPos.y == rayPosMap.y * tileSize.y && dir.y < 0) //hit wall up side
        {
            hitTileY--;
        }

        if (map[hitTileY][hitTileX] == BLOCKADE)
        {
            hit = true; //end raycasting loop
        }
        else
        {
            //move ray to next closest horizontal or vertical side
            sf::Vector2f dist = getDistToClosestHitPoint(angle, { rayPosMap.x, rayPosMap.y }, { rayWorldPos.x, rayWorldPos.y });

            //draw triangle for better visualization of distance
            sf::VertexArray triangleVisual(LinesStrip);
            triangleVisual.append({ { rayWorldPos.x, rayWorldPos.y }, Color::Magenta });
            triangleVisual.append({ { rayWorldPos.x + dist.x, rayWorldPos.y }, Color::Magenta });
            triangleVisual.append({ { rayWorldPos.x + dist.x, rayWorldPos.y + dist.y }, Color::Magenta });
            gameWindow.draw(triangleVisual);

            //apply new move 
            rayWorldPos.x += dist.x;
            rayWorldPos.y += dist.y;

            //update map positions
            rayPosMap.x = rayWorldPos.x / tileSize.x;
            rayPosMap.y = rayWorldPos.y / tileSize.y;
        }
    }


    gameWindow.draw(hitLines);
}

void render(RenderWindow & gameWindow)
{
    gameWindow.clear(Color::White);
    drawMap(gameWindow);

    //draw player
    CircleShape player(25);
    player.setOrigin({ 25, 25 });
    player.setPosition(playerWorldPos);
    player.setFillColor(sf::Color::Black);
    gameWindow.draw(player);

    visualizePlayerRaycast(gameWindow);
    gameWindow.display();
}

int main()
{
    RenderWindow gameWindow(VideoMode(1000, 800), "Raycast Test");
    gameWindow.setFramerateLimit(60);

    //initialization
    tileSize = { (float)gameWindow.getView().getSize().x / MAP_W, (float)gameWindow.getView().getSize().y / MAP_H };
    playerWorldPos = { playerMapPos.x * tileSize.x, playerMapPos.y * tileSize.y, };

    while (gameWindow.isOpen())
    {
        checkEvents(gameWindow);
        updateSteering();
        render(gameWindow);
    }
}
