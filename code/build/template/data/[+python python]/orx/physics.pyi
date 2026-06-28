def get_collision_flag_value(name: str) -> int:
    """Gets the numerical value of a collision flag by its literal name.

    Args:
        name: Literal name of the collision flag as defined in config.

    Returns:
        The numerical value of the collision flag.
    """
    ...

def get_collision_flag_name(value: int) -> str:
    """Gets the literal name of a collision flag by its numerical value.

    Args:
        value: Numerical value of the collision flag.

    Returns:
        The literal name of the collision flag as defined in config.
    """
    ...
