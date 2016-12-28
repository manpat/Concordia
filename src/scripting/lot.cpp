#include "bbtypes.hpp"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "containers/collection3d.hpp"
#include "containers/conccollection3d.hpp"
#include "scripting/lot.hpp"
#include "log.hpp"
#include "tools/utility.hpp"
#include "threading/lockable.hpp"
#include <jsoncpp/json/json.h>
#include "scripting/tile.hpp"
#include "scripting/wallcell.hpp"
#include "scripting/wallpaper.hpp"
#include <memory>
#include <vector>
#include <cstring>
#include <string>
#include <utility>

namespace BlueBear {
	namespace Scripting {

		Lot::Lot( lua_State* L, const Tick& currentTickReference, InfrastructureFactory& infrastructureFactory, Json::Value& rootObject ) :
			L( L ),
			currentTickReference( currentTickReference ),
			infrastructureFactory( infrastructureFactory ),
			floorX( rootObject[ "floorx" ].asInt() ),
			floorY( rootObject[ "floory" ].asInt() ),
			stories( rootObject[ "stories" ].asInt() ),
			undergroundStories( rootObject[ "subtr" ].asInt() ),
			terrainType( TerrainType( rootObject[ "terrain" ].asInt() ) ),
			currentRotation( rootObject[ "rot" ].asUInt() ) {
			buildFloorMap( rootObject[ "infr" ][ "floor" ] );
			buildWallMap( rootObject[ "infr" ][ "wall" ] );
		}

		/**
		 * Using the object lot.infr.wall, build the Collection3D containing all WallCells on the lot. Renderer (Display)
		 * will handle where they end up and what joints are used to draw the walls.
		 */
		void Lot::buildWallMap( Json::Value& wall ) {
			Json::Value dict = wall[ "dict" ];
			Json::Value levels = wall[ "levels" ];

			// Create the vector reference of shared_ptrs by iterating through dict
			std::vector< std::shared_ptr< Wallpaper > > lookup;
			for( const Json::Value& dictEntry : dict ) {
				auto wallpaper = infrastructureFactory.getWallpaper( dictEntry.asString() );
				lookup.push_back( wallpaper );
			}

			wallMap = std::make_unique< Containers::ConcCollection3D< Threading::Lockable< WallCell > > >( stories, floorX + 1, floorY + 1 );

			for( Json::Value& level : levels ) {
				for( Json::Value& object : level ) {
					if( Tools::Utility::isRLEObject( object ) ) {
						// De-RLE the object
						Threading::Lockable< WallCell > wallCell = getWallCell( object[ "value" ], lookup );

						unsigned int run = object[ "run" ].asUInt();
						for( unsigned int i = 0; i != run; i++ ) {
							wallMap->pushDirect( wallCell );
						}
					} else {
						// Push a wallcell or nothing
						wallMap->pushDirect( getWallCell( object, lookup ) );
					}
				}
			}
		}

		/**
		 * Build the wall cell in all four possible dimensions
		 */
		Threading::Lockable< WallCell > Lot::getWallCell( Json::Value& object, std::vector< std::shared_ptr< Wallpaper > >& lookup ) {
			Threading::Lockable< WallCell > wallCell;

			if( object.isObject() && !object.isNumeric() ) {
				// usable object
				wallCell = Threading::Lockable< WallCell >::create();

				// Check for dimensions x, y, d, and r
				Json::Value x = object.get( "x", Json::nullValue );
				Json::Value y = object.get( "y", Json::nullValue );
				Json::Value d = object.get( "d", Json::nullValue );
				Json::Value r = object.get( "r", Json::nullValue );

				// unsafe operations are okay here since it's a brand new Lockable
				if( !x.isNull() ) {
					wallCell.unsafe< void >( [ & ]( WallCell& wallCell ) {
						wallCell.x = std::make_unique< WallCell::Segment >( lookup.at( x[ "f" ].asUInt() ), lookup.at( x[ "b" ].asUInt() ) );
					} );
				}
				if( !y.isNull() ) {
					wallCell.unsafe< void >( [ & ]( WallCell& wallCell ) {
						wallCell.y = std::make_unique< WallCell::Segment >( lookup.at( y[ "f" ].asUInt() ), lookup.at( y[ "b" ].asUInt() ) );
					} );
				}
				if( !d.isNull() ) {
					wallCell.unsafe< void >( [ & ]( WallCell& wallCell ) {
						wallCell.d = std::make_unique< WallCell::Segment >( lookup.at( d[ "f" ].asUInt() ), lookup.at( d[ "b" ].asUInt() ) );
					} );
				}
				if( !r.isNull() ) {
					wallCell.unsafe< void >( [ & ]( WallCell& wallCell ) {
						wallCell.r = std::make_unique< WallCell::Segment >( lookup.at( r[ "f" ].asUInt() ), lookup.at( r[ "b" ].asUInt() ) );
					} );
				}
			}

			return wallCell;
		}

		void Lot::buildFloorMap( Json::Value& floor ) {
			Json::Value dict = floor[ "dict" ];
			Json::Value levels = floor[ "levels" ];

			// Create the vector reference of shared_ptrs by iterating through dict
			std::vector< Threading::Lockable< Tile > > lookup;
			for( const Json::Value& dictEntry : dict ) {
				auto tile = infrastructureFactory.getFloorTile( dictEntry.asString() );
				lookup.push_back( tile );
			}

			// Use the pointer lookup to create the floormap
			floorMap = std::make_unique< Containers::ConcCollection3D< Threading::Lockable< Tile > > >( stories, floorX, floorY );

			for( Json::Value& level : levels ) {
				for( Json::Value& object : level ) {
					if( Tools::Utility::isRLEObject( object ) ) {
						// De-RLE the object
						unsigned int run = object[ "run" ].asUInt();
						Threading::Lockable< Tile > entry = getTile( object[ "value" ].asInt(), lookup );

						for( unsigned int i = 0; i != run; i++ ) {
							floorMap->pushDirect( entry );
						}
					} else {
						floorMap->pushDirect( getTile( object.asInt(), lookup ) );
					}
				}
			}
		}

		/**
		 * Return a tile given a JSON object and a lookup of tiles. This is a simple inline function that just checks to see
		 * if the entry is blank or not; if it's not blank (index of -1), the lookup is performed.
		 */
		Threading::Lockable< Tile > Lot::getTile( int index, std::vector< Threading::Lockable< Tile > >& lookup ) {
			Threading::Lockable< Tile > entry;

			if( index >= 0 ) {
				entry = lookup.at( index );
			}

			return entry;
		}

	}
}
