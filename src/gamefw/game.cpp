#include "../common.h"

#include "game.h"

#include <SFML/Graphics.hpp>

using namespace gamefw;

enum Windows {
    MAIN
};

Game::Game() :
    m_main_window_context(25, 8, 0, 3, 3)
{
    m_main_window.Create(sf::VideoMode(800, 600, 24), "Test", sf::Style::Default,
                         m_main_window_context);
    m_main_window.SetActive();
    glClearColor(0.0, 0.0, 0.0, 1.0);
}

Game::~Game()
{

}

sf::Window* Game::getMainWindow()
{
    return &m_main_window;
}


void Game::update()
{
    if (!m_main_window.IsOpened()) {
        throw 1;
    }
    sf::Event event;
    while (m_main_window.GetEvent(event)) {
        if (event.Type == sf::Event::Closed) {
            m_main_window.Close();
        }
    }
}
