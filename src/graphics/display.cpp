#include "graphics/display.hpp"
#include "threading/commandbus.hpp"
#include "localemanager.hpp"
#include "configmanager.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <memory>

namespace BlueBear {
  namespace Graphics {

    void Display::NewEntityCommand::execute( Display& instance ) {

    }

    Display::Display( Threading::CommandBus& commandBus ) : commandBus( commandBus ) {
      // Get our settings out of the config manager
      x = ConfigManager::getInstance().getIntValue( "viewport_x" );
      y = ConfigManager::getInstance().getIntValue( "viewport_y" );
    }

    void Display::openDisplay() {
      mainWindow.create( sf::VideoMode( x, y ), LocaleManager::getInstance().getString( "BLUEBEAR_WINDOW_TITLE" ), sf::Style::Close );
      mainWindow.setVerticalSyncEnabled( true );

      displayCommands = std::make_unique< Graphics::Display::CommandList >();
    }

    void Display::render() {
      processCommands();

      // Handle events
      sf::Event event;
      while( mainWindow.pollEvent( event ) ) {
        if( event.type == sf::Event::Closed ) {
          mainWindow.close();
        }
      }

      mainWindow.clear( sf::Color::Black );
      mainWindow.display();
    }

    bool Display::isOpen() {
      return mainWindow.isOpen();
    }

    /**
     * Swap the pointers, and if the resulting list contains any commands, process those commands.
     */
    void Display::processCommands() {

    }
  }
}
