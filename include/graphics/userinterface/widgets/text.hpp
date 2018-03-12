#ifndef NEW_GUI_ELEMENT_TEXT
#define NEW_GUI_ELEMENT_TEXT

#include "graphics/userinterface/element.hpp"
#include <string>
#include <functional>
#include <glm/glm.hpp>

namespace BlueBear {
  namespace Graphics {
    namespace UserInterface {
      namespace Widgets {

        class Text : public Element {
          std::string innerText;
          double textSpan = 0;

        protected:
          Text( const std::string& id, const std::vector< std::string >& classes, const std::string& innerText );

        public:
          static std::function< glm::vec4( const std::string&, const std::string&, float ) > getTextSizeParams;

          virtual void render( Device::Display::Adapter::Component::GuiComponent& manager ) override;
          virtual void calculate() override;

          void setText( const std::string& text );
          std::string getText();

          static std::shared_ptr< Text > create( const std::string& id, const std::vector< std::string >& classes, const std::string& innerText );
        };

      }
    }
  }
}

#endif
