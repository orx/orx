import orx

def init():
  # Create the [-movie scene][+movie splash screen]
  orx.object.create_from_config("[-movie Scene][+movie Splash]");

# Setup engine callbacks
orx.on_init = init
