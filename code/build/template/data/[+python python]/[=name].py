import miniscroll, orx

class Logo(miniscroll.Mini):
  def on_update(self, dt: float):
    if orx.input.has_been_activated("Reverse"):
      self.o.set_angular_velocity(self.o.get_angular_velocity() * -1)

def init():
  miniscroll.classes = {
    "Logo": Logo
  }

  # Create the [-movie scene][+movie splash screen]
  orx.object.create_from_config("[-movie Scene][+movie Splash]");

def update(dt: float):
  # Check to see if we should quit the game
  if orx.input.has_been_activated("Quit"):
    orx.close()

# Setup engine callbacks for miniscroll
miniscroll.setup(init, update)
