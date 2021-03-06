#include "graphics/atlasbuilder.hpp"
#include "graphics/imagebuilder/imagesource.hpp"
#include "graphics/imagebuilder/pathimagesource.hpp"
#include "graphics/texture.hpp"
#include "tools/utility.hpp"
#include "log.hpp"
#include <fstream>
#include <string>
#include <jsoncpp/json/json.h>
#include <SFML/Graphics.hpp>
#include <exception>
#include <utility>
#include <memory>

namespace BlueBear {
  namespace Graphics {

    /**
     * Set an AtlaMapping by providing your own ImageSource.
     */
    void AtlasBuilder::setAtlasMapping( const std::string& key, std::unique_ptr< ImageSource > builder ) {
      AtlasMapping& mapping = mappings.at( key );

      mapping.imageBuilder = std::move( builder );
    }

    void AtlasBuilder::configure( const std::string& jsonPath ) {
      mappings.clear();

      std::ifstream schemaFile;
      // std::ifstream::failure
      schemaFile.exceptions( std::ios::failbit | std::ios::badbit );
      schemaFile.open( jsonPath );

      Json::Value schema;
      Json::Reader reader;

      if( !reader.parse( schemaFile, schema ) ) {
        throw AtlasBuilder::CannotLoadFileException();
      }

      // All paths are now relative to the path specified in jsonPath
      std::string basePath;

      auto tokens = Tools::Utility::split( jsonPath, '/' );
      if( tokens.size() > 0 ) {
        tokens.erase( tokens.end() );
        basePath = Tools::Utility::join( tokens, "/" ) + "/";
      }

      // Create a base image on which to overlay existing images
      Json::Value baseProps = schema[ "base" ];
      Json::Value components = schema[ "mappings" ];

      // Dispose of any old image
      base = sf::Image();
      if( baseProps[ "image" ].isString() ) {
        if( !base.loadFromFile( basePath + baseProps[ "image" ].asString() ) ) {
          throw AtlasBuilder::CannotLoadFileException();
        }
      } else {
        base.create( baseProps[ "width" ].asInt(), baseProps[ "height" ].asInt() );
      }

      // Load all components into mappings
      for( Json::Value::iterator jsonIterator = components.begin(); jsonIterator != components.end(); ++jsonIterator ) {
        std::string key = jsonIterator.key().asString();
        Json::Value value = *jsonIterator;

        mappings[ key ] = AtlasMapping{
          ( unsigned int ) value[ "x" ].asInt(),
          ( unsigned int ) value[ "y" ].asInt(),
          ( unsigned int ) value[ "width" ].asInt(),
          ( unsigned int ) value[ "height" ].asInt(),
          std::unique_ptr< ImageSource >()
        };
      }
    }

    std::shared_ptr< Texture > AtlasBuilder::getTextureAtlas() {
      sf::Image atlasBase = base;

      // Apply each overlay
      for( auto& pair : mappings ) {
        AtlasMapping& mapping = pair.second;

        if( mapping.imageBuilder ) {
          atlasBase.copy( mapping.imageBuilder->getImage(), mapping.x, mapping.y );
        }
      }

      // Overlay this sf::Image into an OpenGL texture
      return std::make_shared< Texture >( atlasBase );
    }
  }
}
