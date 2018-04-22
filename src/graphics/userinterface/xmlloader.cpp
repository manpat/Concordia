#include "graphics/userinterface/xmlloader.hpp"
#include "graphics/userinterface/widgets/layout.hpp"
#include "graphics/userinterface/widgets/text.hpp"
#include "graphics/userinterface/widgets/window.hpp"
#include "graphics/userinterface/widgets/button.hpp"
#include "tools/utility.hpp"
#include "log.hpp"

namespace BlueBear::Graphics::UserInterface {

  XMLLoader::XMLLoader( const std::string& subject, bool file ) {
    if( file ) {
      document.LoadFile( subject.c_str() );
    } else {
      document.Parse( subject.c_str(), subject.size() );
    }

    if( document.ErrorID() ) {
      Log::getInstance().error( "XMLLoader::XMLLoader", "XMLLoader construction failed for input: " + subject );
      throw FailedToLoadXMLException();
    }
  }

  std::shared_ptr< Element > XMLLoader::getElementFromXML( const tinyxml2::XMLElement* element ) {
    std::shared_ptr< Element > result;

    switch( Tools::Utility::hash( element->Name() ) ) {
      case Tools::Utility::hash( "Window" ): {
        result = Widgets::Window::create(
          Tools::Utility::safeString( element->Attribute( "id" ) ),
          Tools::Utility::split( Tools::Utility::safeString( element->Attribute( "class" ) ), ' ' ),
          Tools::Utility::safeString( element->Attribute( "window-title" ) )
        );
        break;
      }
      case Tools::Utility::hash( "Layout" ): {
        result = Widgets::Layout::create(
          Tools::Utility::safeString( element->Attribute( "id" ) ),
          Tools::Utility::split( Tools::Utility::safeString( element->Attribute( "class" ) ), ' ' )
        );
        break;
      }
      case Tools::Utility::hash( "Text" ): {
        result = Widgets::Text::create(
          Tools::Utility::safeString( element->Attribute( "id" ) ),
          Tools::Utility::split( Tools::Utility::safeString( element->Attribute( "class" ) ), ' ' ),
          Tools::Utility::stringTrim( Tools::Utility::safeString( element->GetText() ) )
        );
        break;
      }
      case Tools::Utility::hash( "Button" ): {
        result = Widgets::Button::create(
          Tools::Utility::safeString( element->Attribute( "id" ) ),
          Tools::Utility::split( Tools::Utility::safeString( element->Attribute( "class" ) ), ' ' ),
          Tools::Utility::stringTrim( Tools::Utility::safeString( element->GetText() ) )
        );
        break;
      }
      default:
        Log::getInstance().error( "XMLLoader::getElementFromXML", "Unknown element: " + std::string( element->Name() ) );
        throw UnknownElementException();
    }

    for( const tinyxml2::XMLElement* child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement() ) {
      result->addChild( getElementFromXML( child ), false );
    }

    return result;
  }

  std::vector< std::shared_ptr< Element > > XMLLoader::getElements() {
    std::vector< std::shared_ptr< Element > > result;

    for( const tinyxml2::XMLElement* node = document.RootElement(); node != NULL; node = node->NextSiblingElement() ) {
      result.push_back( getElementFromXML( node ) );
    }

    return result;
  }

}
