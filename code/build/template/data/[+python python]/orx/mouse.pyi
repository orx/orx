from vector import Vector

def get_position() -> Vector:
    """Get the current mouse position.

    Returns:
        The current mouse cursor position as a Vector.
    """
    ...

def get_wheel_delta() -> float:
    """Get the mouse wheel delta since the last call.

    Returns:
        The mouse wheel scroll delta accumulated since the previous call.
    """
    ...
