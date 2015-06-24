-- Base Types

-- Register default traits

bluebear.register_motive( "hunger", {
	name = "Hunger",
	decay_rate = 10,
	decay_period = 6000,
	events = {
		{
			trigger = "<= 0",
			callback = function( self, lot ) 
				self.die()
			end
		}
	}
} )

bluebear.register_motive( "hygiene", {
	name = "Hygiene",
	decay_rate = 10,
	decay_period = 6000,
	events = {
		{
			trigger = "<= 15",
			recur = false,
			callback = function( self, lot )
				lot.modal_message( bluebear.Dialog.DIALOG_OK, "What is that...smell?" )
			end
		}
	}
} )
bluebear.register_motive( "bladder", {
	name = "Bladder",
	decay_rate = 5,
	decay_period = 6000,
	increase = true,
	events = {
		{
			trigger = "= 100",
			callback = function( self, lot ) 
				lot.spawn_object( "mipani.puddle", bluebear.Location.BELOW_PLAYER )
				self.animate( self.Animations.EMBARASSED_1 )
				self.set_motive( "hygiene", 15 )
			end
		}
	}
} )
bluebear.register_motive( "energy", {
	name = "Energy",
	decay_rate = 5,
	decay_period = 6000,
	events = {
		{
			trigger = "<= 5",
			callback = "sleep"
		}
	}
} )

local base_player = {

	motives = {
		hunger = bluebear.get_motive_table( "hunger" ),
		hygiene = bluebear.get_motive_table( "hygiene" ),
		bladder = bluebear.get_motive_table( "bladder" ),
		energy = bluebear.get_motive_table( "energy" )
	}

}

_obj_templates[ "base_player" ] = base_player
