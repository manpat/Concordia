#ifndef UI_PROPERTY_LIST
#define UI_PROPERTY_LIST

#include "exceptions/genexc.hpp"
#include "log.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>
#include <string>
#include <typeinfo>
#include <vector>
#include <any>
#include <variant>

namespace BlueBear {
  namespace Graphics {
    namespace UserInterface {
      class Element;

      class PropertyList {
        std::unordered_map< std::string, std::any > values;

      public:
        static const PropertyList& rootPropertyList;

        EXCEPTION_TYPE( InvalidValueException, "Property is not a valid property" );

        PropertyList() = default;
        PropertyList( const std::unordered_map< std::string, std::any >& map ) : values( map ) {}

        std::vector< std::string > getProperties() const {
          std::vector< std::string > result;

          for( auto& pair : values ) {
            result.push_back( pair.first );
          }

          return result;
        }

        void clear() {
          values.clear();
        };

        bool keyExists( const std::string& key ) const {
          return values.find( key ) != values.end();
        };

        void removeProperty( const std::string& key ) {
          values.erase( key );
        };

        template < typename VariantType > void set( const std::string& key, VariantType value ) {
          values[ key ] = value;
        };

        template < typename VariantType > const VariantType get( const std::string& key ) const {
          if( keyExists( key ) ) {
            try {
              return std::any_cast< VariantType >( values.find( key )->second );
            } catch ( const std::bad_any_cast& e ) {
              Log::getInstance().error( "PropertyList::get", key + " could not be converted to the requested type." );
              throw e;
            }
          }

          throw InvalidValueException();
        };

      };

      enum class Gravity {
        LEFT,
        RIGHT,
        TOP,
        BOTTOM
      };

      enum class Requisition : int {
        AUTO = -1,
        NONE = -2,
        FILL_PARENT = -3
      };

      enum class Placement : int {
        FLOW,
        FREE
      };

      enum class Orientation : int {
        TOP,
        MIDDLE,
        BOTTOM,
        LEFT,
        RIGHT
      };

      using PropertyListType = std::variant<
        bool,
        int,
        double,
        Gravity,
        Requisition,
        Placement,
        Orientation,
        std::string,
        // Lua is the one that primarily uses PropertyListType; this will be constructed from a uvec4
        glm::vec4
      >;

    }
  }
}


#endif
