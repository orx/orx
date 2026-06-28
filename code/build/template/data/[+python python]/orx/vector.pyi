class Vector:
  """An orx Vector object.

  Vector objects represent coordinates or directions in 3D space with x, y,
  and z components. It supports operators for vector math and methods for
  in-place manipulation.

  Example::

      v1 = Vector(1.0, 2.0, 3.0)
      v2 = Vector(0.0, 1.0, 0.5)
      v_sum = v1 + v2
      v1.rotate(3.14)
  """
  x: float
  y: float
  z: float

  def __new__(cls, ix: float, iy: float, iz: float) -> Vector:
    """Create a new vector.

    Constructs a new vector with the given x, y, and z components.

    Args:
      ix: X coordinate of the vector.
      iy: Y coordinate of the vector.
      iz: Z coordinate of the vector.

    Returns:
      A new ``Vector`` object.
    """
    ...

  def __eq__(self, op: object) -> bool:
    """Check if two vectors are equal.

    Performs a component-wise equality comparison. Returns True if all three
    components are exactly equal.

    Args:
      op: The other vector to compare against.

    Returns:
      ``True`` if both vectors are equal, ``False`` otherwise.
    """
    ...

  def __neg__(self) -> Vector:
    """Get the negation of the vector.

    Returns:
      A new ``Vector`` containing the negated (-x, -y, -z) components.
    """
    ...

  def __add__(self, op: Vector) -> Vector:
    """Add two vectors.

    Adds the components of both vectors together and returns the result in a
    new vector.

    Args:
      op: The other vector to add.

    Returns:
      A new ``Vector`` containing the sum.
    """
    ...

  def __sub__(self, op: Vector) -> Vector:
    """Subtract two vectors.

    Subtracts the components of the other vector from this vector and returns
    the result in a new vector.

    Args:
      op: The other vector to subtract.

    Returns:
      A new ``Vector`` containing the difference.
    """
    ...

  def __mul__(self, op: Vector | float) -> Vector:
    """Multiply the vector by another vector or scalar.

    If ``op`` is a ``Vector``, performs a component-wise multiplication.
    If ``op`` is a ``float``, multiplies all components by the scalar.

    Args:
      op: A multiplier vector or scalar value.

    Returns:
      A new ``Vector`` containing the multiplied components.
    """
    ...

  def __truediv__(self, op: Vector | float) -> Vector:
    """Divide the vector by another vector or scalar.

    If ``op`` is a ``Vector``, performs a component-wise division.
    If ``op`` is a ``float``, divides all components by the scalar.

    Args:
      op: A divisor vector or scalar value.

    Returns:
      A new ``Vector`` containing the divided components.
    """
    ...
  def __str__(self) -> str:
    """Get the string representation of the vector.

    Returns:
      A string in the format "orx.vector.Vector(x, y, z)".
    """
    ...

  def copy(self) -> Vector:
    """Copy the vector.

    Returns:
      A new ``Vector`` object with the same components as this one.
    """
    ...

  def rotate(self, radians: float) -> None:
    """Rotate the vector in place.

    Rotates this 2D vector (along the Z-axis) in place by the given angle.

    Args:
      radians: Angle of rotation in radians.
    """
    ...

  def normalize(self) -> None:
    """Normalize the vector in place.

    Normalizes this vector's length to 1.0 without changing its direction.
    """
    ...

  def size(self) -> float:
    """Get the size of the vector.

    Returns:
      The size (magnitude) of this vector.
    """
    ...

  def round(self) -> None:
    """Round the vector components in place.

    Rounds all components of this vector to the nearest integer.
    """
    ...

def distance(v1: Vector, v2: Vector) -> float:
  """Get distance between two vectors.

  Args:
    v1: First position vector.
    v2: Second position vector.

  Returns:
    The distance between the two vectors.
  """
  ...

def lerp(v1: Vector, v2: Vector, f: float) -> Vector:
  """Lerp between two vectors.

  Linearly interpolates from one vector to another one using the given
  coefficient.

  Args:
    v1: The starting vector.
    v2: The ending vector.
    f: The interpolation coefficient (usually between 0.0 and 1.0).

  Returns:
    A new ``Vector`` representing the interpolated result.
  """
  ...

def dot(v1: Vector, v2: Vector) -> float:
  """Get 2D dot product of two vectors.

  Calculates the dot product using only the X and Y components.

  Args:
    v1: First vector.
    v2: Second vector.

  Returns:
    The 2D dot product of the two vectors.
  """
  ...

def rotate(v: Vector, radians: float) -> Vector:
  """Rotate a vector.

  Rotates the 2D vector (along the Z-axis) and returns a new vector.

  Args:
    v: The vector to rotate.
    radians: Angle of rotation in radians.

  Returns:
    A new rotated ``Vector``.
  """
  ...

def normalize(v: Vector) -> Vector:
  """Normalize a vector.

  Args:
    v: The vector to normalize.

  Returns:
    A new normalized ``Vector`` with length 1.0.
  """
  ...

def round(v: Vector) -> Vector:
  """Round a vector.

  Rounds the components of the vector to the nearest integer.

  Args:
    v: The vector to round.

  Returns:
    A new rounded ``Vector``.
  """
  ...
