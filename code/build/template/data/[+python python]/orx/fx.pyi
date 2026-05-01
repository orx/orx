from vector import Vector

class FX:
  """An orx special effect object.

  FX objects are rendering special effects that can affect the color, scale,
  rotation, and position of visuals over time.  Each FX is loaded from a
  config section that defines one or more curve slots describing how a
  particular property changes across the FX's duration.

  FX curves can also be sampled to get float or vector values at a time offset.

  Example::

      fx = FX("MyEffect")
      value = fx.get_float(0.0, 0.5)
  """

  def __new__(cls, name: str) -> FX:
    """Create an FX from a config section name.

    Loads the FX definition from the named config section.  Raises
    ``ValueError`` if the section does not exist or the FX cannot be created.

    Args:
      name: Config section name identifying the FX definition.

    Returns:
      An ``FX`` instance for the given config section.

    Raises:
      ValueError: If the FX cannot be created from the given config section.
    """
    ...

  def get_float(self, start_time: float, time: float) -> float:
    """Get the user float value of this FX between two timestamps.

    When ``start_time`` is ``<= 0``, the absolute value at ``time`` is
    returned.  Otherwise the delta between ``start_time`` and ``time`` is
    returned.

    Args:
      start_time: Previous timestamp in seconds.  Pass ``0.0`` to retrieve
        the absolute value at ``time`` rather than the delta. Ignored for
        Absolute slots.
      time: Current timestamp in seconds.

    Returns:
      The float value produced by the FX at the given time range.
    """
    ...

  def get_vector(self, start_time: float, time: float) -> Vector:
    """Get the user vector value of this FX between two timestamps.

    When ``start_time`` is ``<= 0``, the absolute value at ``time`` is
    returned.  Otherwise the delta between ``start_time`` and ``time`` is
    returned.

    Args:
      start_time: Previous timestamp in seconds.  Pass ``0.0`` to retrieve
        the absolute value at ``time`` rather than the delta. Ignored for
        Absolute slots.
      time: Current timestamp in seconds.

    Returns:
      The ``Vector`` value produced by the FX at the given time range.

    Raises:
      ValueError: If the FX does not contain valid vector-typed slots.
    """
    ...

def create_from_config(section_name: str) -> FX | None:
  """Create an FX from a config section name.

  Unlike the ``FX`` constructor, this function returns ``None`` instead of
  raising an exception when the FX cannot be created.

  Args:
    section_name: Config section name identifying the FX definition.

  Returns:
    An ``FX`` instance for the given config section, or ``None`` if the FX
    could not be created.
  """
  ...
