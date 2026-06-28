class Viewport:
  """An orx Viewport object.

  Viewports are structures associated to cameras and used for rendering.
  They define a rectangular area on the screen and can be aligned, sized,
  and associated with background colors, shaders, and textures.

  Example::

      viewport = Viewport("MyViewport")
  """

  def __new__(cls, name: str) -> Viewport:
    """Create a Viewport from a config section name.

    Loads the Viewport definition from the named config section. Raises
    ``ValueError`` if the section does not exist or the Viewport cannot be created.

    Args:
      name: Config section name identifying the Viewport definition.

    Returns:
      A ``Viewport`` instance for the given config section.

    Raises:
      ValueError: If the Viewport cannot be created from the given config section.
    """
    ...

  def delete(self) -> None:
    """Deletes the viewport.

    Frees the underlying C object. Note that accessing the viewport after
    calling this method will result in undefined behavior.
    """
    ...

def create_from_config(name: str) -> Viewport | None:
  """Create a Viewport from a config section name.

  Unlike the ``Viewport`` constructor, this function returns ``None`` instead
  of raising an exception when the Viewport cannot be created.

  Args:
    name: Config section name identifying the Viewport definition.

  Returns:
    A ``Viewport`` instance for the given config section, or ``None`` if the
    Viewport could not be created.
  """
  ...
