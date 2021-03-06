#include "graphics/texturecache.hpp"
#include "graphics/texture.hpp"
#include "graphics/atlasbuilder.hpp"
#include "graphics/imagebuilder/imagesource.hpp"
#include "configmanager.hpp"
#include "log.hpp"
#include <string>
#include <map>
#include <memory>

namespace BlueBear {
  namespace Graphics {

    std::shared_ptr< Texture > TextureCache::get( const std::string& path ) {
      auto textureIterator = textureCache.find( path );

      if( textureIterator != textureCache.end() ) {
        return textureIterator->second;
      }

      // Value needs to be created
      // This should throw an exception if it fails
      textureCache[ path ] = std::make_shared< Texture >( path );
      return textureCache[ path ];
    }

    /**
     * Prune all shared_ptrs in the map that only have one count
     */
    void TextureCache::prune() {
      auto iterator = std::begin( textureCache );

      while( iterator != std::end( textureCache ) ) {
        if( iterator->second.unique() ) {
          iterator = textureCache.erase( iterator );
        } else {
          iterator++;
        }
      }
    }

    std::string TextureCache::getKey( AtlasSettings& mappings ) {
      std::string key;

      for( auto& iterator : mappings ) {
        key = key + iterator.second->getKey() + " ";
      }

      return key;
    }

    /**
     * Shared logic to work around C++'s inability to just define a damn non-pointer reference and assign it separately
     */
    std::shared_ptr< Texture > TextureCache::generateForAtlasBuilderEntry( AtlasBuilderEntry& entry, AtlasSettings& mappings ) {
      // This atlas builder already exists
      AtlasBuilder& builder = entry.builder;
      SharedPointerTextureCache& texCache = entry.generatedTextures;

      // Generate the key for the desired set of textures
      std::string key = getKey( mappings );

      auto textureIterator = texCache.find( key );
      if( textureIterator != texCache.end() ) {
        // The texture was found
        return textureIterator->second;
      } else {
        Log::getInstance().warn( "TextureCache::generateForAtlasBuilderEntry", "Key (" + key + ") not found; generating texture atlas." );

        // The texture needs to be generated using "builder"
        for( auto& kvPair : mappings ) {
          builder.setAtlasMapping( kvPair.first, std::move( kvPair.second ) );
        }

        return texCache[ key ] = builder.getTextureAtlas();
      }
    }

    /**
     * Get a texture according to an atlas and cache both the AtlasBuilder used to make it
     * and the resulting atlas (if you provided the same order in the map).
     */
    std::shared_ptr< Texture > TextureCache::getUsingAtlas( const std::string& atlasBasePath, AtlasSettings& mappings ) {
      // Test for atlas at the current path
      auto atlasBuilderEntryIterator = atlasTextureCache.find( atlasBasePath );

      if( atlasBuilderEntryIterator != atlasTextureCache.end() ) {
        return generateForAtlasBuilderEntry( atlasBuilderEntryIterator->second, mappings );
      } else {
        // This atlas builder needs to be created
        if( ConfigManager::getInstance().getBoolValue( "disable_texture_cache" ) == true ) {
          AtlasBuilder builder;
          builder.configure( atlasBasePath );

          for( auto& kvPair : mappings ) {
            builder.setAtlasMapping( kvPair.first, std::move( kvPair.second ) );
          }

          return builder.getTextureAtlas();
        } else {
          AtlasBuilderEntry& wrapper = atlasTextureCache[ atlasBasePath ] = AtlasBuilderEntry();
          wrapper.builder.configure( atlasBasePath );

          return generateForAtlasBuilderEntry( wrapper, mappings );
        }
      }
    }


  }
}
