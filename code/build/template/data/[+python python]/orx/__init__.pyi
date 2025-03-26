# Expose each submodule as `orx.<module_name>`
from . import command, config, guid, input, object, vector

def log(message: str) -> None: ...
"""Log a message to the terminal and console using orx's logging"""

def close() -> None: ...
"""Send a close event to the engine. Can be used to exit the game loop."""

# Engine callbacks
def on_init() -> None: ...
def on_exit() -> None: ...
def on_update(dt: float) -> None: ...
def on_create(o: object.Object) -> None: ...
def on_delete(o: object.Object) -> None: ...
def on_collide(
    o1: object.Object,
    body_name1: str,
    o2: object.Object,
    body_name2: str,
    position: vector.Vector,
    normal: vector.Vector
) -> None: ...
def on_separate(
    o1: object.Object,
    body_name1: str,
    o2: object.Object,
    body_name2: str
) -> None: ...
def on_shader_param(
    o: object.Object,
    shader_name: str,
    param_name: str,
    param_type: type[float] | type[vector.Vector]
) -> (float | vector.Vector | None): ...
