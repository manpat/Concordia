#ifndef CONFIGMANAGER
#define CONFIGMANAGER

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <jsoncpp/json/json.h>
#include <string>
#include <map>
#include <functional>

namespace BlueBear {
  class ConfigManager {
    public:
      Json::Value configRoot;

      static ConfigManager& getInstance() {
        static ConfigManager instance;
        return instance;
      }

      std::string getValue( const std::string& key );
      int getIntValue( const std::string& key );
      bool getBoolValue( const std::string& key );
      static int lua_getValue( lua_State* L );

      void each( std::function< void( std::string, Json::Value& ) > func );

    private:
      static constexpr const char* SETTINGS_PATH = "settings.json";

      ConfigManager();
      ConfigManager( ConfigManager const& );
      void operator=( ConfigManager const& );
  };
}


#endif
