class Guid:
  """An opaque 64-bit globally unique identifier (GUID) for an orx structure.

  GUIDs are produced by functions such as `Object.get_guid` and can be used to
  retrieve an `Object` at a later point via its constructor.

  Because GUIDs are value types, two `Guid` objects that contain the same
  underlying 64-bit value compare equal and hash to the same value, making them
  safe to use as dictionary keys or set members.
  """

  def __eq__(self, other: object) -> bool:
    """Return whether this GUID equals *other*.

    Two `Guid` values are equal when their underlying 64-bit identifiers are
    identical.

    Args:
      other: The object to compare against.  Returns `False` for any object
        that is not a `Guid`.

    Returns:
      `True` if both GUIDs represent the same identifier, `False` otherwise.
    """
    ...

  def __hash__(self) -> int:
    """Return an integer hash of this GUID.

    The hash is derived directly from the underlying 64-bit identifier, so
    `Guid` objects can be used as dictionary keys or stored in sets.

    Returns:
      An integer hash value for this GUID.
    """
    ...

  def __str__(self) -> str:
    """Return a hexadecimal string representation of this GUID.

    The string is formatted as a zero-padded 16-digit hexadecimal number
    prefixed with ``0x`` (e.g. ``\"0x0000000100000001\"``). This format can
    be used in commands where a GUID value is required.

    Returns:
      A string of the form ``\"0x<16 hex digits>\"`` representing the GUID.
    """
    ...
