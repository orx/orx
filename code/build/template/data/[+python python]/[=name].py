import orx
import pyscroll
[-scroll
from typing import override

class Logo(pyscroll.ScrollObject):
  @override
  def on_update(self, dt: float):
    # Per-frame update logic for an object would go here
    pass]

def init():
[-scroll
  # Register object config section names with pyscroll classes
  pyscroll.bind(Logo)
]
  # Push [Main] as the default config section
  orx.config.push_section("Main")

  # Create viewports defined in [Main]
  for i in range(0, orx.config.get_list_count("ViewportList")):
    _ = orx.viewport.create_from_config(orx.config.get_string("ViewportList", i))

  # Create the [-movie scene][+movie splash screen]
  _ = orx.object.create_from_config("[-movie Scene][+movie Splash]")

def update(dt: float):
  # Check to see if we should quit the game
  if orx.input.has_been_activated("Quit"):
    orx.close()

def camera_update(dt: float):
  # Get camera proxy control object
  with orx.config.Section("MainCamera"):
    id = orx.config.get_guid("ID")
  camera = orx.object.from_guid(id)
  if camera is not None:
    # Round camera position
    position = camera.get_position()
    position.round()
    camera.set_position(position)

# Register camera update callback
orx.on_camera_update = camera_update

# Setup engine callbacks for pyscroll
pyscroll.setup(init, update)
