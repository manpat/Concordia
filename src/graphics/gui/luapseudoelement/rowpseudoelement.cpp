#include "graphics/gui/luapseudoelement/rowpseudoelement.hpp"
#include "graphics/gui/luaelement.hpp"
#include "graphics/widgetbuilder.hpp"
#include "tools/ctvalidators.hpp"
#include "tools/utility.hpp"
#include "log.hpp"
#include <algorithm>
#include <set>
#include <tinyxml2.h>

namespace BlueBear {
  namespace Graphics {
    namespace GUI {

      RowPseudoElement::RowPseudoElement( std::shared_ptr< sfg::Table > subject, int rowNumber, Display::MainGameState& displayState ) :
        subject( subject ), rowNumber( rowNumber ), displayState( displayState ) {}


      /**
       *
       * STACK ARGS: userdata
       * (Stack is unmodified after call)
       */
      void RowPseudoElement::setMetatable( lua_State* L ) {
        if( luaL_newmetatable( L, "bluebear_row_pseudo_element" ) ) { // metatable userdata
          luaL_Reg tableFuncs[] = {
            { "add", RowPseudoElement::lua_add },
            { "remove", RowPseudoElement::lua_removeWidget },
            { "get_name", RowPseudoElement::lua_getName },
            { "find_pseudo", RowPseudoElement::lua_findElement },
            { "find_by_id", RowPseudoElement::lua_findById },
            { "find_by_class", RowPseudoElement::lua_findByClass },
            { "get_property", RowPseudoElement::lua_getProperty },
            { "set_property", RowPseudoElement::lua_setProperty },
            { "get_content", RowPseudoElement::lua_content },
            { "set_content", RowPseudoElement::lua_content },
            { "get_children", RowPseudoElement::lua_getChildElements },
            { "__gc", RowPseudoElement::lua_gc },
            { NULL, NULL }
          };

          luaL_setfuncs( L, tableFuncs, 0 );

          lua_pushvalue( L, -1 ); // metatable metatable userdata

          lua_setfield( L, -2, "__index" ); // metatable userdata
        }

        lua_setmetatable( L, -2 ); // userdata
      }

      std::string RowPseudoElement::getName() {
        return "row";
      }

      void RowPseudoElement::removeFromTable( std::shared_ptr< sfg::Widget > comparison ) {
        if( !subject || ( comparison != subject ) ) {
          Log::getInstance().warn( "RowPseudoElement::removeFromTable", "This <row> is not attached to this Table widget!" );
          return;
        }

        // Get elements belonging to this row
        std::vector< std::shared_ptr< sfg::Widget > > insertedWidgets = getWidgetsForRow();
        for( std::shared_ptr< sfg::Widget > widget : insertedWidgets ) {
          widget->Show( false );
          subject->Remove( widget );
        }

        rowNumber = -1;
        subject = nullptr;
      }

      int RowPseudoElement::create( lua_State* L, Display::MainGameState& displayState, tinyxml2::XMLElement* element ) {
        RowPseudoElement** item = ( RowPseudoElement** ) lua_newuserdata( L, sizeof( RowPseudoElement* ) ); // userdata
        *item = new RowPseudoElement( nullptr, -1, displayState );
        ( *item )->setMetatable( L );

        // Process out everything
        float spacing = 0.0f;
        if( element->QueryFloatAttribute( "table_spacing", &spacing ) == tinyxml2::XML_SUCCESS ) {
          ( *item )->stagedRowSpacing = spacing;
        }

        ( *item )->processElements( element->FirstChildElement(), -1 );

        return 1;
      }

      /**
       * @static
       */
      int RowPseudoElement::getRowCount( std::shared_ptr< sfg::Table > table ) {
        std::set< sf::Uint32 > counts;

        std::list< sfg::priv::TableCell > tableCellList = table->m_cells;
        std::for_each( tableCellList.begin(), tableCellList.end(), [ & ]( sfg::priv::TableCell& cell ) {
          // Top should contain row number
          // Set should never add two of the same
          counts.insert( cell.rect.top );
        } );

        return counts.size();
      }

      std::vector< std::shared_ptr< sfg::Widget > > RowPseudoElement::getWidgetsForRow() {
        std::vector< std::shared_ptr< sfg::Widget > > widgets;

        if( subject ) {
          // Use this magic using the exposed properties in ne0ndrag0n/SFGUI
          // Copy the table cell list
          std::list< sfg::priv::TableCell > tableCellList = subject->m_cells;

          std::for_each( tableCellList.begin(), tableCellList.end(), [ & ]( sfg::priv::TableCell& cell ) {
            // Top should contain the row number
            if( cell.rect.top == rowNumber ) {
              widgets.push_back( cell.child );
            }
          } );
        } else {
          // Extract all the stagedWidgets
          for( RowPseudoElement::WidgetStaging& staging : stagedWidgets ) {
            widgets.push_back( staging.widget );
          }
        }

        return widgets;
      }

      /**
       *
       * STACK ARGS: none
       * Returns: userdata or none
       */
      int RowPseudoElement::getItemById( lua_State* L, const std::string& id ) {
        std::vector< std::shared_ptr< sfg::Widget > > widgets = getWidgetsForRow();

        for( std::shared_ptr< sfg::Widget > widget : widgets ) {

          // Is this the widget we need?
          if( LuaElement::getId( widget ) == id ) {
            LuaElement::getUserdataFromWidget( L, widget ); // userdata
            return 1;
          }

          // Does this widget tree contain the widget we need?
          if( std::shared_ptr< sfg::Widget > found = LuaElement::getWidgetById( widget, id ) ) {
            LuaElement::getUserdataFromWidget( L, found ); // userdata
            return 1;
          }

        }

        Log::getInstance().warn( "RowPseudoElement::getElementById", "No elements of ID " + id + " found in this pseudo-element." );
        return 0;
      }

      /**
       *
       * STACK ARGS: none
       * Returns: array, single userdata, or none
       */
      int RowPseudoElement::getItemsByClass( lua_State* L, const std::string clss ) {
        std::vector< std::shared_ptr< sfg::Widget > > widgets = getWidgetsForRow();
        std::vector< std::shared_ptr< sfg::Widget > > results;

        // Pare this list down from widgets into results
        for( std::shared_ptr< sfg::Widget > widget : widgets ) {

          if( LuaElement::getClass( widget ) == clss ) {
            results.push_back( widget );
          }

          std::vector< std::shared_ptr< sfg::Widget > > subwidgets = LuaElement::getWidgetsByClass( widget, clss );
          results.insert( results.end(), subwidgets.begin(), subwidgets.end() );;
        }

        int resultSize = results.size();
        if( resultSize == 1 ) {
          // Single item, return direct
          LuaElement::getUserdataFromWidget( L, results[ 0 ] ); // userdata
          return 1;
        } else if( resultSize ) {
          // Multiple items, return array

          lua_createtable( L, resultSize, 0 ); // table

          for( int i = 0; i != resultSize; i++ ) {
            LuaElement::getUserdataFromWidget( L, results[ i ] ); // userdata table
            lua_rawseti( L, -2, i + 1 ); // table
          }

          return 1;
        }

        Log::getInstance().warn( "RowPseudoElement::getItemsByClass", "No elements of class " + clss + " found in this pseudo-element." );
        return 0;
      }

      void RowPseudoElement::setSubject( std::shared_ptr< sfg::Table > table ) {
        if( !table ) {
          Log::getInstance().error( "RowPseudoElement::setSubject", "std::shared_ptr< sfg::Table > was nullptr" );
          return;
        }

        if( subject ) {
          Log::getInstance().warn( "RowPseudoElement::setSubject", "This <row> already belongs to a Table and cannot be added to another one." );
          return;
        }

        subject = table;

        rowNumber = RowPseudoElement::getRowCount( subject );

        std::map< unsigned int, float > requestedColumnSpacings;
        unsigned int size = stagedWidgets.size();
        for( int i = 0; i != size; i++ ) {
          // Add the WidgetStaging
          WidgetStaging staging = stagedWidgets.at( i );
          addFromStaging( staging, -1 );

          // If this widget has the table_spacing property set on it (WidgetBuilder should slap it on),
          // set it up so that we set the column spacing
          if( LuaElement::propertyIsSet( staging.widget, "table_spacing" ) ) {
            float spacing = 0.0f;
            LuaElement::queryFloatAttribute( staging.widget, "table_spacing", &spacing );
            requestedColumnSpacings[ i ] = spacing;
          }
        }

        for( auto& it : requestedColumnSpacings ) {
          subject->SetColumnSpacing( it.first, it.second );
        }

        if( stagedRowSpacing != -1.0f ) {
          subject->SetRowSpacing( rowNumber, stagedRowSpacing );
          stagedRowSpacing = -1.0f;
        }

        stagedWidgets.clear();
      }

      void RowPseudoElement::add( std::shared_ptr< sfg::Widget > widget, int index ) {
        // This is just a disgrace
        unsigned int colspan = 1;
        LuaElement::queryUnsignedAttribute( widget, "colspan", &colspan );
        unsigned int rowspan = 1;
        LuaElement::queryUnsignedAttribute( widget, "rowspan", &rowspan );
        float paddingX = 0.0f;
        LuaElement::queryFloatAttribute( widget, "padding_x", &paddingX );
        float paddingY = 0.0f;
        LuaElement::queryFloatAttribute( widget, "padding_y", &paddingY );
        bool expandX = true, expandY = true;
        bool fillX = true, fillY = true;

        LuaElement::queryBoolAttribute( widget, "expand_x", &expandX );
        LuaElement::queryBoolAttribute( widget, "expand_y", &expandX );
        LuaElement::queryBoolAttribute( widget, "fill_x", &fillX );
        LuaElement::queryBoolAttribute( widget, "fill_y", &fillY );

        int packX = 0, packY = 0;
        if( expandX ) { packX |= sfg::Table::EXPAND; }
        if( fillX ) { packX |= sfg::Table::FILL; }
        if( expandY ) { packY |= sfg::Table::EXPAND; }
        if( fillY ) { packY |= sfg::Table::FILL; }

        RowPseudoElement::WidgetStaging staging{
          colspan, rowspan, paddingX, paddingY, packX, packY, widget
        };

        if( subject ) {
          addFromStaging( staging, index );
        } else {
          std::vector< RowPseudoElement::WidgetStaging >::iterator iterator = stagedWidgets.end();

          if( index > -1 && index <= stagedWidgets.size() ) {
            iterator = stagedWidgets.begin();
            std::advance( iterator, index );
          }

          stagedWidgets.insert( iterator, staging );
        }
      }

      void RowPseudoElement::addFromStaging( WidgetStaging staging, int columnIndex ) {
        if( subject ) {
          int latestColumn = getLatestColumn();
          int columnNumber = columnIndex > -1 ? columnIndex : latestColumn + 1;

          subject->Attach(
            staging.widget,
            sf::Rect< sf::Uint32 >( columnNumber, rowNumber, staging.colspan, staging.rowspan ),
            staging.packX,
            staging.packY,
            sf::Vector2f( staging.paddingX, staging.paddingY )
          );
        } else {
          Log::getInstance().error( "RowPseudoElement::addFromStaging", "Tried to add <row> element staging to null subject; this is likely a bug." );
        }
      }

      void RowPseudoElement::add( lua_State* L, const std::string& xmlString, int index ) {
        // Manually parse this out - We'll be adding a widget but it has additional properties that need to be passed to the shared_ptr overload
        tinyxml2::XMLDocument document;

        processElements( Tools::Utility::getRootNode( document, xmlString ), index );
      }

      void RowPseudoElement::processElements( tinyxml2::XMLElement* element, int startingIndex ) {
        int index = 0;

        for( tinyxml2::XMLElement* child = element; child != NULL; child = child->NextSiblingElement() ) {
          try {
            WidgetBuilder widgetBuilder( displayState.getImageCache() );
            add( widgetBuilder.getWidgetFromElementDirect( child ), startingIndex == -1 ? -1 : index + startingIndex );

            index++;
          } catch( std::exception& e ) {
            Log::getInstance().error( "RowPseudoElement::add", "Failed to add widget XML: " + std::string( e.what() ) );
          }
        }
      }

      int RowPseudoElement::getLatestColumn() {
        if( subject ) {
          int largestColumn = -1;

          std::list< sfg::priv::TableCell > tableCellList = subject->m_cells;
          std::for_each( tableCellList.begin(), tableCellList.end(), [ & ]( sfg::priv::TableCell& cell ) {
            if( cell.rect.top == rowNumber && (int) cell.rect.left > largestColumn ) {
              largestColumn = cell.rect.left;
            }
          } );

          return largestColumn;
        } else {
          return stagedWidgets.size();
        }
      }

      int RowPseudoElement::lua_add( lua_State* L ) {
        RowPseudoElement* self = *( ( RowPseudoElement** ) luaL_checkudata( L, 1, "bluebear_row_pseudo_element" ) );

        bool numericArgument = lua_gettop( L ) == 3 && lua_isnumber( L, -1 );
        int position = numericArgument ? lua_tonumber( L, -1 ) : -1;
        int target = numericArgument ? -2 : -1;

        if( lua_isstring( L, target ) ) {
          self->add( L, lua_tostring( L, target ), position );
        } else {
          LuaElement* element = *( ( LuaElement** ) luaL_checkudata( L, 2, "bluebear_widget" ) );
          self->add( element->widget, position );
        }
      }

      int RowPseudoElement::lua_findById( lua_State* L ) {
        VERIFY_STRING( "RowPseudoElement::lua_findById", "find_by_id" );
        RowPseudoElement* self = *( ( RowPseudoElement** ) luaL_checkudata( L, 1, "bluebear_row_pseudo_element" ) );

        return self->getItemById( L, lua_tostring( L, -1 ) ); // userdata/none
      }

      int RowPseudoElement::lua_findByClass( lua_State* L ) {
        VERIFY_STRING( "RowPseudoElement::lua_findByClass", "find_by_class" );
        RowPseudoElement* self = *( ( RowPseudoElement** ) luaL_checkudata( L, 1, "bluebear_row_pseudo_element" ) );

        return self->getItemsByClass( L, lua_tostring( L, -1 ) ); // userdata/none
      }

      int RowPseudoElement::lua_getName( lua_State* L ) {
        RowPseudoElement* self = *( ( RowPseudoElement** ) luaL_checkudata( L, 1, "bluebear_row_pseudo_element" ) );

        lua_pushstring( L, self->getName().c_str() );

        return 1;
      }

      float RowPseudoElement::getSpacing() {
        if( subject ) {
          return subject->m_rows.at( rowNumber ).spacing;
        } else {
          return stagedRowSpacing;
        }
      }

      void RowPseudoElement::setSpacing( float spacing ) {
        if( subject ) {
          subject->SetRowSpacing( rowNumber, spacing );
        } else {
          stagedRowSpacing = spacing;
        }
      }

      /**
       *
       * STACK ARGS: none
       * Returns: number or none
       */
      int RowPseudoElement::getProperty( lua_State* L, const std::string& property ) {
        switch( Tools::Utility::hash( property.c_str() ) ) {
          case Tools::Utility::hash( "table_spacing" ): {
            lua_pushnumber( L, getSpacing() ); // 42.0
            return 1;
          }
          default:
            Log::getInstance().warn( "RowPseudoElement::getProperty", "Property \"" + std::string( property ) + "\" cannot be queried from this pseudo-element." );
        }

        return 0;
      }

      int RowPseudoElement::lua_getProperty( lua_State* L ) {
        VERIFY_STRING_N( "RowPseudoElement::lua_getProperty", "get_property", 1 );

        RowPseudoElement* self = *( ( RowPseudoElement** ) luaL_checkudata( L, 1, "bluebear_row_pseudo_element" ) );

        return self->getProperty( L, lua_tostring( L, -1 ) );
      }

      /**
       *
       * STACK ARGS: various
       * (Stack is unmodified after call)
       */
      void RowPseudoElement::setProperty( lua_State* L, const std::string& property ) {
        switch( Tools::Utility::hash( property.c_str() ) ) {
          case Tools::Utility::hash( "table_spacing" ): {
            if( lua_isnumber( L, -1 ) ) {
              setSpacing( lua_tonumber( L, -1 ) );
            } else {
              Log::getInstance().warn( "RowPseudoElement::setProperty", "Argument #2 must be a number." );
            }

            return;
          }
          default:
            Log::getInstance().warn( "RowPseudoElement::setProperty", "Property \"" + std::string( property ) + "\" cannot be set on this pseudo-element." );
        }
      }

      int RowPseudoElement::lua_setProperty( lua_State* L ) {
        VERIFY_STRING_N( "RowPseudoElement::lua_getProperty", "get_property", 2 );

        RowPseudoElement* self = *( ( RowPseudoElement** ) luaL_checkudata( L, 1, "bluebear_row_pseudo_element" ) );

        self->setProperty( L, lua_tostring( L, -2 ) );
        return 0;
      }

      void RowPseudoElement::remove( lua_State* L, LuaElement* element ) {
        if( subject ) {
          subject->Remove( element->widget );
        } else {

          auto it = std::find_if( stagedWidgets.begin(), stagedWidgets.end(), [ & ]( WidgetStaging staging ) {
            return staging.widget == element->widget;
          } );

          if( it != stagedWidgets.end() ) {
            stagedWidgets.erase( it );
          }

        }
      }

      int RowPseudoElement::lua_removeWidget( lua_State* L ) {
        VERIFY_USER_DATA( "RowPseudoElement::lua_removeWidget", "remove" );

        RowPseudoElement* self = *( ( RowPseudoElement** ) luaL_checkudata( L, 1, "bluebear_row_pseudo_element" ) );

        LuaElement* element = *( ( LuaElement** ) luaL_checkudata( L, 2, "bluebear_widget" ) );
        self->remove( L, element );

        return 0;
      }

      int RowPseudoElement::lua_findElement( lua_State* L ) {
        Log::getInstance().warn( "RowPseudoElement::lua_findElement", "<row> pseudo-element has no pseudo-element children." );
        return 0;
      }

      int RowPseudoElement::lua_findPseudo( lua_State* L ) {
        Log::getInstance().warn( "RowPseudoElement::lua_findPseudo", "<row> pseudo-element has no pseudo-element children." );
        return 0;
      }

      int RowPseudoElement::lua_content( lua_State* L ) {
        Log::getInstance().warn( "RowPseudoElement::lua_content", "<row> pseudo-element has no direct content." );
        return 0;
      }

      int RowPseudoElement::lua_getChildElements( lua_State* L ) {
        RowPseudoElement* self = *( ( RowPseudoElement** ) luaL_checkudata( L, 1, "bluebear_row_pseudo_element" ) );

        LuaElement::elementsToTable( L, self->getWidgetsForRow() ); // table

        return 1;
      }

      int RowPseudoElement::lua_gc( lua_State* L ) {
        // TODO cleanup of masterAttrMap

        RowPseudoElement* self = *( ( RowPseudoElement** ) luaL_checkudata( L, 1, "bluebear_row_pseudo_element" ) );

        delete self;

        return 0;
      }

    }
  }
}
