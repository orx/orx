import orx

def orx_init():
  # Create the [-movie scene][+movie splash screen]
  orx.object.create_from_config("[-movie Scene][+movie Splash]");

def orx_update(_dt: float):
  pass

def orx_exit():
  return
