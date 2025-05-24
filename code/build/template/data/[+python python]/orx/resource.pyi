def read(group: str, name: str) -> bytes:
    """Read the entire contents of a named resource and return the raw bytes
    of the resource.

    Locates the resource by name within the given group, opens it, reads all of
    its data synchronously, and closes it before returning.

    Args:
        group: Resource group to search (e.g. ``"Texture"``, ``"Sound"``).
        name:  Name of the resource within that group (e.g. a relative file
               path such as ``"sprites/player.png"``).

    Returns:
        The raw byte contents of the resource.

    Raises:
        ValueError: If the resource cannot be located or opened.
        RuntimeError: If the number of bytes read does not match the resource
            size reported by the engine.
    """
    ...
