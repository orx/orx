def push_set(name: str) -> None:
  """Push an input set to make it active, storing the current one on the stack.

  Args:
    name: Name of the input set to push.
  """
  ...

def pop_set() -> None:
  """Pop the last input set from the stack, restoring the previous one."""
  ...

class Set:
  """Context manager for pushing/popping input sets

  The config section defining the input set should be passed to the context
  manager constructor.

  Use:
  ```python
  with Set("PlayerInput"):
    throttle = orx.input.get_value("Accelerator")
  # Input set will automatically be popped, even if an exception is raised
  ```
  """

  def __new__(cls, name: str) -> Set: ...

  def __enter__(self) -> None: ...

  def __exit__(self, *args: None) -> None: ...

def enable_set(name: str, enable: bool = True) -> None:
  """Enable or disable an input set without selecting it as the current working set.

  Args:
    name: Name of the input set to enable or disable.
    enable: ``True`` to enable the set, ``False`` to disable it.
  """
  ...

def is_set_enabled(name: str) -> bool:
  """Check whether an input set is enabled (includes the current working set).

  Args:
    name: Name of the input set to check.

  Returns:
    ``True`` if the set is enabled, ``False`` otherwise.
  """
  ...

def get_all() -> list[str]:
  """Return all input names defined in the current working set.

  Returns:
    A list of input names in the current set.
  """
  ...

def is_active(name: str) -> bool:
  """Check whether an input is currently active.

  Args:
    name: Name of the input to check.

  Returns:
    ``True`` if the input is active, ``False`` otherwise.
  """
  ...

def has_new_status(name: str) -> bool:
  """Check whether an input has a new status this frame.

  Args:
    name: Name of the input to check.

  Returns:
    ``True`` if the input status changed since the last frame, ``False`` otherwise.
  """
  ...

def has_been_activated(name: str) -> bool:
  """Check whether an input was newly activated this frame.

  Args:
    name: Name of the input to check.

  Returns:
    ``True`` if the input was activated since the last frame, ``False`` otherwise.
  """
  ...

def has_been_deactivated(name: str) -> bool:
  """Check whether an input was newly deactivated this frame.

  Args:
    name: Name of the input to check.

  Returns:
    ``True`` if the input was deactivated since the last frame, ``False`` otherwise.
  """
  ...

def get_value(name: str) -> float:
  """Return the current value of an input.

  Args:
    name: Name of the input to query.

  Returns:
    The current floating-point value of the input.
  """
  ...

def set_value(name: str, value: float, permanent: bool = False) -> None:
  """Set an input value from code, overriding peripheral input.

  When ``permanent`` is ``False`` the override applies for one frame only
  (i.e. peripheral input takes over again on the next frame).  When
  ``permanent`` is ``True`` the override persists until :func:`reset_value`
  is called.

  Args:
    name: Name of the input to set.
    value: Value to assign; use ``0.0`` to deactivate the input.
    permanent: If ``True``, the value persists until explicitly reset.
  """
  ...

def reset_value(name: str) -> None:
  """Reset an input value so that peripheral inputs are used again.

  Args:
    name: Name of the input to reset.
  """
  ...
