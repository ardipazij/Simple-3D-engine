#include <iostream>
#include <SFML/Graphics.hpp>

using namespace sf;

class Game{
public:
    RenderWindow gameWindow{VideoMode(1000, 800), "Raycast Test"};
    CircleShape player{25};
    Game(){
        gameWindow.setFramerateLimit(60);
     }
    void draw(){
        gameWindow.clear(Color::White);
        gameWindow.display();
    }
    void update(){
        ;
    }
    void checkevents(){
        Event event;
        while (gameWindow.pollEvent(event))
        {
            if (event.type == Event::Closed)
                gameWindow.close();
        }
    }           
    void run(){
        while(gameWindow.isOpen()){
            checkevents();
            draw();
            update();
        }
    }


};

int main(){
    Game game;
    game.run();
    return 0;
}
