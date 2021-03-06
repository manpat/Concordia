#ifndef WIDGETBUILDER
#define WIDGETBUILDER

#include <tinyxml2.h>
#include <SFGUI/Widget.hpp>
#include <SFGUI/Window.hpp>
#include <SFGUI/Label.hpp>
#include <SFGUI/Container.hpp>
#include <SFGUI/Box.hpp>
#include <SFGUI/Alignment.hpp>
#include <SFGUI/Misc.hpp>
#include <SFGUI/Button.hpp>
#include <SFGUI/Entry.hpp>
#include <SFGUI/Image.hpp>
#include <SFGUI/Frame.hpp>
#include <SFGUI/ProgressBar.hpp>
#include <SFGUI/Separator.hpp>
#include <SFGUI/Notebook.hpp>
#include <SFGUI/ScrolledWindow.hpp>
#include <SFGUI/Viewport.hpp>
#include <SFGUI/Adjustment.hpp>
#include <SFGUI/Table.hpp>
#include <SFGUI/Range.hpp>
#include <SFGUI/ToggleButton.hpp>
#include <SFGUI/CheckButton.hpp>
#include <SFGUI/RadioButton.hpp>
#include <SFGUI/SpinButton.hpp>
#include <SFGUI/ComboBox.hpp>
#include <SFGUI/Fixed.hpp>
#include <cparse/shunting-yard.h>
#include <string>
#include <memory>
#include <exception>
#include <vector>
#include <utility>
#include <array>
#include "log.hpp"
#include "tools/utility.hpp"

namespace BlueBear {
  namespace Graphics {
    class ImageCache;

    class WidgetBuilder {
      ImageCache& imageCache;
      TokenMap settingsTokens;
      // "This RadioButton needs to be associated with the group belonging to this item"
      std::vector< std::pair< std::shared_ptr< sfg::RadioButton >, std::string > > groups;
      static constexpr unsigned int hash(const char* str, int h = 0);
      bool includeCustom = true;

      void buildSettingsTokens();

      void addChildren( std::shared_ptr< sfg::ScrolledWindow > scrolledWindow, tinyxml2::XMLElement* element );
      void addChildren( std::shared_ptr< sfg::Container > widget, tinyxml2::XMLElement* element );
      void packChildren( std::shared_ptr< sfg::Box > widget, tinyxml2::XMLElement* element );
      void addNotebookTabs( std::shared_ptr< sfg::Notebook > notebook, tinyxml2::XMLElement* element );

      std::shared_ptr< sfg::Window > newWindowWidget( tinyxml2::XMLElement* element );
      std::shared_ptr< sfg::Label > newLabelWidget( tinyxml2::XMLElement* element );
      std::shared_ptr< sfg::Box > newBoxWidget( tinyxml2::XMLElement* element );
      std::shared_ptr< sfg::Alignment > newAlignmentWidget( tinyxml2::XMLElement* element );
      std::shared_ptr< sfg::Button > newButtonWidget( tinyxml2::XMLElement* element );
      std::shared_ptr< sfg::Entry > newEntryWidget( tinyxml2::XMLElement* element );
      std::shared_ptr< sfg::Image > newImageWidget( tinyxml2::XMLElement* element );
      std::shared_ptr< sfg::Frame > newFrameWidget( tinyxml2::XMLElement* element );
      std::shared_ptr< sfg::ProgressBar > newProgressBarWidget( tinyxml2::XMLElement* element );
      std::shared_ptr< sfg::Separator > newSeparatorWidget( tinyxml2::XMLElement* element );
      std::shared_ptr< sfg::Notebook > newNotebookWidget( tinyxml2::XMLElement* element );
      std::shared_ptr< sfg::ScrolledWindow > newScrolledWindowWidget( tinyxml2::XMLElement* element );
      std::shared_ptr< sfg::Viewport > newViewportWidget( tinyxml2::XMLElement* element );
      std::shared_ptr< sfg::Table > newTableWidget( tinyxml2::XMLElement* element );
      template< typename T > std::shared_ptr< T > newRangeWidget( tinyxml2::XMLElement* element ) {
        std::shared_ptr< T > range = T::Create( Orientation< sfg::Range >( element->Attribute( "orientation" ) ).get() );

        setBasicProperties( range, element );
        setAllocationAndRequisition( range, element );
        setDefaultEvents( range, element );

        setRangeAdjustment( range, element );

        return range;
      };
      template < typename T > std::shared_ptr< T > newToggleButtonDerivativeWidget( tinyxml2::XMLElement* element ) {
        std::shared_ptr< T > toggleButton = T::Create( Tools::Utility::sanitizeCString( element->GetText() ) );

        setBasicProperties( toggleButton, element );
        setAllocationAndRequisition( toggleButton, element );
        setDefaultEvents( toggleButton, element );

        bool enabled = false;
        element->QueryBoolAttribute( "enabled", &enabled );

        toggleButton->SetActive( enabled );

        return toggleButton;
      };
      std::shared_ptr< sfg::RadioButton > newRadioButtonWidget( tinyxml2::XMLElement* element );
      std::shared_ptr< sfg::SpinButton > newSpinButtonWidget( tinyxml2::XMLElement* element );
      std::shared_ptr< sfg::ComboBox > newComboBoxWidget( tinyxml2::XMLElement* element );
      std::shared_ptr< sfg::Fixed > newFixedWidget( tinyxml2::XMLElement* element );

      void addTableRows( std::shared_ptr< sfg::Table > table, tinyxml2::XMLElement* element );

      void setDefaultEvents( std::shared_ptr< sfg::Widget > widget, tinyxml2::XMLElement* element );
      void setCustomAttributes( std::shared_ptr< sfg::Widget > widget, tinyxml2::XMLElement* element );
      void setAlignment( std::shared_ptr< sfg::Misc > widget, tinyxml2::XMLElement* element );
      void setBasicProperties( std::shared_ptr< sfg::Widget > widget, tinyxml2::XMLElement* element );
      void setAllocationAndRequisition( std::shared_ptr< sfg::Widget > widget, tinyxml2::XMLElement* element );
      void setRangeAdjustment( std::shared_ptr< sfg::Range > range, tinyxml2::XMLElement* element );
      void setEatEntryEvent( std::shared_ptr< sfg::Entry > entry );

      std::shared_ptr< sfg::Widget > nodeToWidget( tinyxml2::XMLElement* element );

      void correctXBoundary( float* input );
      void correctYBoundary( float* input );

      template< typename T > void setAdjustments( std::shared_ptr< T > widget, tinyxml2::XMLElement* element ) {
        // Set the adjustment positions based on viewport_x and viewport_y attributes
        std::shared_ptr< sfg::Adjustment > adjustmentX = widget->GetHorizontalAdjustment();
        std::shared_ptr< sfg::Adjustment > adjustmentY = widget->GetVerticalAdjustment();

        // Set default upper and lower bounds
        float lowerX = adjustmentX->GetLower();
        float upperX = adjustmentX->GetUpper();
        float lowerY = adjustmentY->GetLower();
        float upperY = adjustmentY->GetUpper();

        Tools::Utility::queryFloatExpression( element, "min_x", settingsTokens, &lowerX );
        Tools::Utility::queryFloatExpression( element, "max_x", settingsTokens, &upperX );
        Tools::Utility::queryFloatExpression( element, "min_y", settingsTokens, &lowerY );
        Tools::Utility::queryFloatExpression( element, "max_y", settingsTokens, &upperY );

        adjustmentX->SetLower( lowerX );
        adjustmentX->SetUpper( upperX );
        adjustmentY->SetLower( lowerY );
        adjustmentY->SetUpper( upperY );

        // Set the viewport values
        float viewportX = adjustmentX->GetValue();
        Tools::Utility::queryFloatExpression( element, "scroll_x", settingsTokens, &viewportX );
        float viewportY = adjustmentY->GetValue();
        Tools::Utility::queryFloatExpression( element, "scroll_y", settingsTokens, &viewportY );

        adjustmentX->SetValue( viewportX );
        adjustmentY->SetValue( viewportY );

        // Set the viewport minor and major step
        float minorStepX = adjustmentX->GetMinorStep();
        float minorStepY = adjustmentY->GetMinorStep();

        float majorStepX = adjustmentX->GetMajorStep();
        float majorStepY = adjustmentY->GetMajorStep();

        Tools::Utility::queryFloatExpression( element, "minor_step_x", settingsTokens, &minorStepX );
        Tools::Utility::queryFloatExpression( element, "minor_step_y", settingsTokens, &minorStepY );
        Tools::Utility::queryFloatExpression( element, "major_step_x", settingsTokens, &majorStepX );
        Tools::Utility::queryFloatExpression( element, "major_step_y", settingsTokens, &majorStepY );

        adjustmentX->SetMinorStep( minorStepX );
        adjustmentY->SetMinorStep( minorStepY );

        adjustmentX->SetMajorStep( majorStepX );
        adjustmentY->SetMajorStep( majorStepY );

        // Set the viewport page size
        float pageSizeX = adjustmentX->GetPageSize();
        float pageSizeY = adjustmentY->GetPageSize();
        Tools::Utility::queryFloatExpression( element, "page_size_x", settingsTokens, &pageSizeX );
        Tools::Utility::queryFloatExpression( element, "page_size_y", settingsTokens, &pageSizeY );

        adjustmentX->SetPageSize( pageSizeX );
        adjustmentY->SetPageSize( pageSizeY );
      };

    public:
      WidgetBuilder( ImageCache& imageCache );
      std::vector< std::shared_ptr< sfg::Widget > > getWidgets( const std::string& path );
      std::shared_ptr< sfg::Widget > getWidgetFromXML( const std::string& xmlString );
      std::shared_ptr< sfg::Widget > getWidgetFromElementDirect( tinyxml2::XMLElement* element );


      template< typename T > static T getOrientation( const char* orientation ) {
        if( !orientation ) {
          orientation = "horizontal";
        }

        T orientationFlag;

        switch( Tools::Utility::hash( orientation ) ) {
          case Tools::Utility::hash( "vertical" ):
            orientationFlag = T::VERTICAL;
            break;
          default:
            Log::getInstance().warn( "WidgetBuilder::getOrientation", "Invalid value for \"orientation\" attribute: " + std::string( orientation ) + ", defaulting to \"horizontal\"" );
          case Tools::Utility::hash( "horizontal" ):
            orientationFlag = T::HORIZONTAL;
            break;
        }

        return orientationFlag;
      };

      template< typename T > static T getPosition( const char* tabs ) {
        if( !tabs ) {
          tabs = "top";
        }

        T tabPosition;
        switch( Tools::Utility::hash( tabs ) ) {
          default:
            Log::getInstance().warn( "WidgetBuilder::getPosition", "Invalid value for \"tab_position\" attribute: " + std::string( tabs ) + ", defaulting to \"top\"" );
          case Tools::Utility::hash( "top" ):
            tabPosition = T::TOP;
            break;
          case Tools::Utility::hash( "bottom" ):
            tabPosition = T::BOTTOM;
            break;
          case Tools::Utility::hash( "left" ):
            tabPosition = T::LEFT;
            break;
          case Tools::Utility::hash( "right" ):
            tabPosition = T::RIGHT;
        }

        return tabPosition;
      };

      template< typename T > static std::string getPosition( T position ) {
        switch( position ) {
          default:
          case T::TOP:
            return "top";
          case T::BOTTOM:
            return "bottom";
          case T::LEFT:
            return "left";
          case T::RIGHT:
            return "right";
        }
      };

      // Let's move onto helper classes instead of public static functions on WidgetBuilder
      // for better encapsulation and potential testability

      /**
       * Helper class to convert between various forms of ScrollbarPolicy
       */
      class ScrollbarPolicy {
      public:
        sfg::ScrolledWindow::ScrollbarPolicy x;
        sfg::ScrolledWindow::ScrollbarPolicy y;

        ScrollbarPolicy( const char* x, const char* y ) {
          setX( x );
          setY( y );
        }

        ScrollbarPolicy( std::shared_ptr< sfg::ScrolledWindow > window ) {
          char stat = window->GetScrollbarPolicy();

          x = ( sfg::ScrolledWindow::ScrollbarPolicy )( stat & 0x7 );
          y = ( sfg::ScrolledWindow::ScrollbarPolicy )( stat & 0x38 );
        }

        void setX( const char* x ) {
          switch( Tools::Utility::hash( x ) ) {
            case Tools::Utility::hash( "off" ):
              this->x = sfg::ScrolledWindow::ScrollbarPolicy::HORIZONTAL_NEVER;
              break;
            case Tools::Utility::hash( "on" ):
              this->x = sfg::ScrolledWindow::ScrollbarPolicy::HORIZONTAL_ALWAYS;
              break;
            default:
              if( x ) {
                Log::getInstance().warn( "ScrollbarPolicy::setX", "Invalid value for \"scrollbar_x\" attribute: " + std::string( x ) + ", defaulting to \"auto\"" );
              }
            case Tools::Utility::hash( "auto" ):
              this->x = sfg::ScrolledWindow::ScrollbarPolicy::HORIZONTAL_AUTOMATIC;
          }
        }

        void setY( const char* y ) {
          switch( Tools::Utility::hash( y ) ) {
            case Tools::Utility::hash( "off" ):
              this->y = sfg::ScrolledWindow::ScrollbarPolicy::VERTICAL_NEVER;
              break;
            case Tools::Utility::hash( "on" ):
              this->y = sfg::ScrolledWindow::ScrollbarPolicy::VERTICAL_ALWAYS;
              break;
            default:
              if( y ) {
                Log::getInstance().warn( "ScrollbarPolicy::setY", "Invalid value for \"scrollbar_y\" attribute: " + std::string( y ) + ", defaulting to \"auto\"" );
              }
            case Tools::Utility::hash( "auto" ):
              this->y = sfg::ScrolledWindow::ScrollbarPolicy::VERTICAL_AUTOMATIC;
          }
        }

        std::string getXAsString() {
          if( x & sfg::ScrolledWindow::ScrollbarPolicy::HORIZONTAL_NEVER ) {
            return "off";
          } else if( x & sfg::ScrolledWindow::ScrollbarPolicy::HORIZONTAL_ALWAYS ) {
            return "on";
          } else {
            return "auto";
          }
        }

        std::string getYAsString() {
          if( y & sfg::ScrolledWindow::ScrollbarPolicy::VERTICAL_NEVER ) {
            return "off";
          } else if( y & sfg::ScrolledWindow::ScrollbarPolicy::VERTICAL_ALWAYS ) {
            return "on";
          } else {
            return "auto";
          }
        }

        char get() {
          return x | y;
        }
      };

      class Placement {
        sfg::ScrolledWindow::Placement placement;

      public:
        Placement( sfg::ScrolledWindow::Placement placement ) : placement( placement ) {}
        Placement( const char* mode ) {
          switch( Tools::Utility::hash( mode ) ) {
            default:
            case Tools::Utility::hash( "top_left" ):
              placement = sfg::ScrolledWindow::Placement::TOP_LEFT;
              break;
            case Tools::Utility::hash( "top_right" ):
              placement = sfg::ScrolledWindow::Placement::TOP_RIGHT;
              break;
            case Tools::Utility::hash( "bottom_left" ):
              placement = sfg::ScrolledWindow::Placement::BOTTOM_LEFT;
              break;
            case Tools::Utility::hash( "bottom_right" ):
              placement = sfg::ScrolledWindow::Placement::BOTTOM_RIGHT;
          }
        }
        sfg::ScrolledWindow::Placement get() {
          return placement;
        }
        std::string getAsString() {
          switch( placement ) {
            default:
            case sfg::ScrolledWindow::Placement::TOP_LEFT:
              return "top_left";
            case sfg::ScrolledWindow::Placement::TOP_RIGHT:
              return "top_right";
            case sfg::ScrolledWindow::Placement::BOTTOM_LEFT:
              return "bottom_left";
            case sfg::ScrolledWindow::Placement::BOTTOM_RIGHT:
              return "bottom_right";
          }
        }
      };

      template < typename T > class Orientation {
        // If it's not horizontal, it's vertical
        typename T::Orientation orientation;

      public:
        Orientation( const char* orientationString ) {
          if( !orientationString ) {
            orientationString = "horizontal";
          }

          switch( Tools::Utility::hash( orientationString ) ) {
            case Tools::Utility::hash( "vertical" ):
              orientation = T::Orientation::VERTICAL;
              break;
            default:
              Log::getInstance().warn( "Orientation::Orientation", "Invalid value for \"orientation\" attribute: " + std::string( orientationString ) + ", defaulting to \"horizontal\"" );
            case Tools::Utility::hash( "horizontal" ):
              orientation = T::Orientation::HORIZONTAL;
              break;
          }
        }

        Orientation( typename T::Orientation orientation ) : orientation( orientation ) {}

        typename T::Orientation get() {
          return orientation;
        }

        std::string getAsString() {
          switch( orientation ) {
            case T::VERTICAL:
              return "vertical";
            case T::HORIZONTAL:
            default:
              return "horizontal";
          }
        }
      };

    };

    struct InvalidCMEWidgetException : public std::exception {
      const char* what() const throw() { return "Invalid tag type specified!"; }
    };

    struct FailedToLoadXMLException : public std::exception {
      const char* what() const throw() { return "Error loading XML!"; }
    };

  }
}

#endif
