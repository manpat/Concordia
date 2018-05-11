#ifndef ENTITY_REGISTRANT
#define ENTITY_REGISTRANT

#include "exceptions/genexc.hpp"
#include "scripting/entitykit/entity.hpp"
#include "scripting/entitykit/component.hpp"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <sol.hpp>
#include <string>
#include <optional>
#include <map>

namespace BlueBear::Scripting::EntityKit {

  class Registry {
    std::map< std::string, sol::table > components;
    std::map< std::string, std::vector< std::string > > entities;

    std::map< std::string, sol::object > tableToMap( sol::table table );
    void submitLuaContributions( sol::state& lua );
    void registerComponent( const std::string& id, sol::table table );
    void registerEntity( const std::string& id, const std::vector< std::string >& componentlist );

    Component newComponent( const std::string& id );
    Entity newEntity( const std::string& id, std::map< std::string, sol::table > constructors );

  public:
    EXCEPTION_TYPE( InvalidIDException, "Invalid ID!" );

    Registry();
    ~Registry();
  };

}

#endif
