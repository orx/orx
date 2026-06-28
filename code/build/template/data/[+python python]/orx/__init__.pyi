# Expose each submodule as `orx.<module_name>`
from . import command, clock, config, fx, guid, input, mouse, object, physics, render, resource, vector, viewport

def log(message: str) -> None:
  """Log a message to the terminal and console using orx's logging"""
  ...

def close() -> None:
  """Send a close event to the engine. Can be used to exit the game loop."""
  ...

# Engine callbacks
def on_init() -> None:
  """Called once when the engine is initialized."""
  ...

def on_exit() -> None:
  """Called once when the engine is shutting down."""
  ...

# Event callbacks
def on_update(dt: float) -> None:
  """Called with every tick of the core clock."""
  ...

def on_camera_update(dt: float) -> None:
  """Called every tick of the core clock, after physics and objects have been updated."""
  ...

def on_create(o: object.Object) -> None:
  """Called when an object is created."""
  ...

def on_delete(o: object.Object) -> None:
  """Called when an object is deleted."""
  ...

def on_spawn(spawner: object.Object, spawned: object.Object) -> None:
  """Called when an object is spawned."""
  ...

def on_collide(
    o1: object.Object,
    body_name1: str,
    o2: object.Object,
    body_name2: str,
    position: vector.Vector,
    normal: vector.Vector
) -> None:
  """Called when two objects collide."""
  ...

def on_separate(
    o1: object.Object,
    body_name1: str,
    o2: object.Object,
    body_name2: str
) -> None:
  """Called when two objects separate."""
  ...

def on_shader_param(
    o: object.Object,
    shader_name: str,
    param_name: str,
    param_type: type[float] | type[vector.Vector]
) -> (float | vector.Vector | None):
  """Called when a shader parameter is requested."""
  ...
