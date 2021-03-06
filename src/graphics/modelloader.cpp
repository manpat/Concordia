#include "graphics/modelloader.hpp"
#include "graphics/model.hpp"
#include "graphics/display.hpp"
#include "graphics/animplayer.hpp"
#include "tools/ctvalidators.hpp"
#include "exceptions/nullpointerexception.hpp"
#include "log.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace BlueBear {
  namespace Graphics {

    int ModelLoaderHelper::lua_createModelLoader( lua_State* L ) {
      ModelLoader** userData = ( ModelLoader** )lua_newuserdata( L, sizeof( ModelLoader* ) ); // userdata
      *userData = new ModelLoader();

      luaL_getmetatable( L, "bluebear_model_loader" ); // metatable userdata
      lua_setmetatable( L, -2 ); // userdata

      return 1;
    }

    int ModelLoaderHelper::lua_loadModel( lua_State* L ) {
      VERIFY_STRING_N( "ModelLoaderHelper::lua_loadModel", "load_model", 1 );
      VERIFY_STRING_N( "ModelLoaderHelper::lua_loadModel", "load_model", 2 );

      ModelLoader* self = *( ( ModelLoader** ) luaL_checkudata( L, 1, "bluebear_model_loader" ) );

      // "path" "id"
      self->emplace( lua_tostring( L, -2 ), std::make_shared< Model >( lua_tostring( L, -1 ) ) );

      return 0;
    }

    int ModelLoaderHelper::lua_getInstance( lua_State* L ) {
      VERIFY_TABLE_N( "ModelLoaderHelper::lua_getInstance", "get_instance", 1 );
      VERIFY_STRING_N( "ModelLoaderHelper::lua_getInstance", "get_instance", 2 );

      // table "id"
      ModelLoader* self = *( ( ModelLoader** ) luaL_checkudata( L, 1, "bluebear_model_loader" ) );
      Display::MainGameState* state = ( Display::MainGameState* )lua_touserdata( L, lua_upvalueindex( 1 ) );

      auto it = self->find( lua_tostring( L, -2 ) );
      if( it == self->end() ) {
        Log::getInstance().warn( "ModelLoaderHelper::lua_getInstance", "Model id " + std::string( lua_tostring( L, -2 ) ) + " not found in this ModelLoader." );
        return 0;
      }

      glm::vec3 initialPosition;

      lua_rawgeti( L, -1, 1 ); // x table "id"
      initialPosition.x = lua_tonumber( L, -1 );
      lua_pop( L, 1 ); // table "id"

      lua_rawgeti( L, -1, 2 ); // y table "id"
      initialPosition.y = lua_tonumber( L, -1 );
      lua_pop( L, 1 ); // table "id"

      lua_rawgeti( L, -1, 3 ); // z table "id"
      initialPosition.z = lua_tonumber( L, -1 );
      lua_pop( L, 3 ); // EMPTY

      LuaInstanceHelper** userData = ( LuaInstanceHelper** )lua_newuserdata( L, sizeof( LuaInstanceHelper* ) ); // userdata
      *userData = new LuaInstanceHelper();
      luaL_getmetatable( L, "bluebear_graphics_instance" ); // metatable userdata
      lua_setmetatable( L, -2 ); // userdata

      LuaInstanceHelper* instanceHelper = *userData;

      // Here's where we put custom shaders if we ever implement that for individual objects (this would -1 in the arugment list)
      instanceHelper->shader = state->getRegisteredShaders()[ "default" ];
      instanceHelper->instance = std::make_shared< Instance >( *( it->second ) );
      instanceHelper->instance->setPosition( initialPosition );

      return 1;
    }

    int LuaInstanceHelper::lua_getAnimList( lua_State* L ) {
      LuaInstanceHelper* self = *( ( LuaInstanceHelper** ) luaL_checkudata( L, 1, "bluebear_graphics_instance" ) );

      lua_newtable( L ); // table

      std::shared_ptr< std::map< std::string, Animation > > animation = self->instance->getAnimList();

      if( animation ) {
        unsigned int index = 0;
        for( auto& pair : *animation ) {
          lua_pushstring( L, pair.first.c_str() ); // "string" table
          lua_rawseti( L, -2, ++index ); // table
        }
      } else {
        Log::getInstance().warn( "LuaInstanceHelper::lua_getAnimList", "This model has no animations!" );
      }

      return 1;
    }

    int LuaInstanceHelper::lua_setAnimation( lua_State* L ) {
      unsigned int argumentCount = lua_gettop( L );
      bool playNow = true;
      std::string animationID;

      if( argumentCount == 2 ) {
        VERIFY_STRING_N( "LuaInstanceHelper::lua_setAnimation", "set_anim", 1 );

        animationID = lua_tostring( L, -1 );
      } else if( argumentCount == 3 ) {
        VERIFY_BOOLEAN_N( "LuaInstanceHelper::lua_setAnimation", "set_anim", 1 );
        VERIFY_STRING_N( "LuaInstanceHelper::lua_setAnimation", "set_anim", 2 );

        playNow = lua_toboolean( L, -1 ) ? true : false;
        animationID = lua_tostring( L, -2 );
      } else {
        Log::getInstance().warn( "LuaInstanceHelper::lua_setAnimation", "Pass either (self, string, bool) or (self, string)." );
        return 0;
      }

      LuaInstanceHelper* self = *( ( LuaInstanceHelper** ) luaL_checkudata( L, 1, "bluebear_graphics_instance" ) );
      self->instance->setAnimation( animationID, playNow );

      return 0;
    }

    int LuaInstanceHelper::lua_pauseAnimation( lua_State* L ) {
      LuaInstanceHelper* self = *( ( LuaInstanceHelper** ) luaL_checkudata( L, 1, "bluebear_graphics_instance" ) );

      if( std::shared_ptr< AnimPlayer > animPlayer = self->instance->getAnimPlayer() ) {
        animPlayer->pause();
      } else {
        Log::getInstance().warn( "LuaInstanceHelper::lua_pauseAnimation", "Instance has no animation playing!" );
      }

      return 0;
    }

    int LuaInstanceHelper::lua_isAnimationPaused( lua_State* L ) {
      LuaInstanceHelper* self = *( ( LuaInstanceHelper** ) luaL_checkudata( L, 1, "bluebear_graphics_instance" ) );

      if( std::shared_ptr< AnimPlayer > animPlayer = self->instance->getAnimPlayer() ) {
        lua_pushboolean( L, animPlayer->getPaused() ? 1 : 0 ); // true

        return 1;
      } else {
        Log::getInstance().warn( "LuaInstanceHelper::lua_isAnimationPaused", "Instance has no animation playing!" );
      }

      return 0;
    }

    int LuaInstanceHelper::lua_setAnimationFrame( lua_State* L ) {
      VERIFY_NUMBER_N( "LuaInstanceHelper::lua_setAnimationFrame", "set_anim_frame", 1 );

      LuaInstanceHelper* self = *( ( LuaInstanceHelper** ) luaL_checkudata( L, 1, "bluebear_graphics_instance" ) );
      self->instance->setAnimationFrame( lua_tonumber( L, -1 ) );

      return 0;
    }

    int LuaInstanceHelper::lua_getAnimationDuration( lua_State* L ) {
      LuaInstanceHelper* self = *( ( LuaInstanceHelper** ) luaL_checkudata( L, 1, "bluebear_graphics_instance" ) );

      if( std::shared_ptr< AnimPlayer > animPlayer = self->instance->getAnimPlayer() ) {
        lua_pushnumber( L, animPlayer->getAnimationDuration() ); // 42.0

        return 1;
      } else {
        Log::getInstance().warn( "LuaInstanceHelper::lua_getAnimationDuration", "Instance has no animation playing!" );
      }

      return 0;
    }

    int LuaInstanceHelper::lua_getAnimation( lua_State* L ) {
      LuaInstanceHelper* self = *( ( LuaInstanceHelper** ) luaL_checkudata( L, 1, "bluebear_graphics_instance" ) );

      lua_pushstring( L, self->instance->getAnimation().c_str() );

      return 1;
    }

    int ModelLoaderHelper::lua_gc( lua_State* L ) {
      ModelLoader* self = *( ( ModelLoader** ) luaL_checkudata( L, 1, "bluebear_model_loader" ) );

      delete self;

      return 0;
    }

    int LuaInstanceHelper::lua_gc( lua_State* L ) {
      LuaInstanceHelper* self = *( ( LuaInstanceHelper** ) luaL_checkudata( L, 1, "bluebear_graphics_instance" ) );

      delete self;

      return 0;
    }

  }
}
