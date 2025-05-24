from vector import Vector
from viewport import Viewport

def get_world_position(screen_position: Vector, viewport: Viewport | None) -> Vector | None:
    """Gets the world position corresponding to a screen position (absolute picking).

    Args:
        screen_position: The screen-space position to convert.
        viewport: The viewport to use for the conversion. If None, uses either
            the last viewport that contains the screen position (if any), or
            the last viewport with a camera in the list if none contains it.

    Returns:
        The corresponding world-space position if the screen position is inside
        the display surface, or None otherwise.
    """
    ...

def get_screen_position(world_position: Vector, viewport: Viewport | None) -> Vector | None:
    """Gets the screen position corresponding to a world position (rendering position).

    Args:
        world_position: The world-space position to convert.
        viewport: The viewport to use for the conversion. If None, the last
            viewport with a camera will be used.

    Returns:
        The corresponding screen-space position if the conversion succeeds
        (can be off-screen), or None otherwise.
    """
    ...
