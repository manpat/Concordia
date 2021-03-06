#ifndef COLLECTION3D
#define COLLECTION3D

#include <vector>
#include <memory>
#include <utility>
#include <functional>

namespace BlueBear {
  namespace Containers {

    template <typename T> class Collection3D {

      protected:
        std::vector< T > items;
        unsigned int getSingleIndex( unsigned int level, unsigned int x, unsigned int y ) {
          return ( dimensions.y * dimensions.x * level ) + ( dimensions.x * y ) + x;
        }

        std::vector< T >& getItems() {
          return items;
        }

      public:
        using Predicate = std::function< void( T& ) >;
        struct Dimensions {
          unsigned int x;
          unsigned int y;
          unsigned int levels;
        };

      protected:
        Dimensions dimensions;

      public:
        Collection3D( unsigned int levels, unsigned int x, unsigned int y ) : dimensions( Dimensions{ x, y, levels } ) {}

        unsigned int getX() {
          return getDimensions().x;
        }

        unsigned int getY() {
          return getDimensions().y;
        }

        unsigned int getLevels() {
          return getDimensions().levels;
        }

        /**
         * RVO might destroy thread safety here?
         * This is SUPPOSED to be a copy, explicitly
         */
        virtual Dimensions getDimensions() {
          return dimensions;
        }

        virtual T& getItemDirectByRef( unsigned int direct ) {
          return getItems()[ direct ];
        }
        T getItemDirect( unsigned int direct ) {
          return getItemDirectByRef( direct );
        }

        T& getItemByRef( unsigned int level, unsigned int x, unsigned int y ) {
          return getItemDirectByRef( getSingleIndex( level, x, y ) );
        }
        T getItem( unsigned int level, unsigned int x, unsigned int y ) {
          return getItemByRef( level, x, y );
        }

        virtual void clear() {
          items.clear();
        }

        virtual unsigned int getLength() {
          return getItems().size();
        }
        //void setItem( unsigned int level, unsigned int x, unsigned int y, T item );
        virtual void pushDirect( T item ) {
          getItems().push_back( item );
        }

        virtual void moveDirect( T item ) {
          getItems().push_back( std::move( item ) );
        }
    };

  }
}


#endif
