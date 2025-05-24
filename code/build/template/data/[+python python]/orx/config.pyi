from guid import Guid
from vector import Vector

def set_parent(name: str, parent: str | None) -> None:
  """Set a section's parent.

  If the section does not exist, it will be created.

  Args:
    name: The name of the section whose parent is being set.
    parent: The name of the parent section to assign. Pass ``None`` to erase
      the current parent, or an empty string ``""`` to explicitly enforce
      "no default parent" (preventing the global default parent from being
      inherited).
  """
  ...

def get_parent(name: str) -> str | None:
  """Get a section's parent name.

  Args:
    name: The name of the section to query.

  Returns:
    The parent section name if one has been explicitly set, an empty string
    ``""`` if "no default parent" has been enforced, or ``None`` if no parent
    has been set.
  """
  ...

def push_section(name: str) -> None:
  """Push a section onto the section stack, making it the current section.

  The previous current section is saved on the stack and can be restored by
  calling :func:`pop_section`. Prefer the :class:`Section` context manager
  for automatic push/pop management.

  Args:
    name: The name of the section to push and select.
  """
  ...

def pop_section() -> None:
  """Pop the last section from the section stack.

  Restores the section that was current before the most recent
  :func:`push_section` call. Prefer the :class:`Section` context manager
  for automatic push/pop management.
  """
  ...

class Section:
  """Context manager for pushing/popping configuration sections

  Use:
  ```python
  with Section("Runtime"):
    orx.config.set_bool("IsInRuntimeSection", True)
  # Section will automatically be popped, even if an exception is raised
  ```
  """

  def __new__(cls, name: str) -> Section: ...

  def __enter__(self) -> None: ...

  def __exit__(self, *args: None) -> None: ...

def get_sections() -> list[str]:
  """Get a list of all config section names.

  Returns:
    A list containing the names of every section currently in config.
  """
  ...

def get_keys() -> list[str]:
  """Get a list of all key names in the current section.

  Returns:
    A list of key names defined in the currently selected config section.
  """
  ...

def get_list_count(key: str) -> int:
  """Get the number of items in a config list value.

  Args:
    key: The key name to query.

  Returns:
    The number of items in the list if the value is a valid list, or ``0``
    if the key is not a list.
  """
  ...

def set_bool(key: str, value: bool) -> None:
  """Write a boolean value to the current config section.

  Args:
    key: The key name to write to.
    value: The boolean value to store.
  """
  ...

def get_bool(key: str, index: int | None = None) -> bool:
  """Read a boolean value from the current config section.

  Args:
    key: The key name to read from.
    index: If ``None``, returns a single value (choosing randomly when the
      value is a list). If an integer, returns the item at that index from
      the list.

  Returns:
    The boolean value stored under ``key``.
  """
  ...

def set_int(key: str, value: int) -> None:
  """Write an integer value to the current config section.

  Args:
    key: The key name to write to.
    value: The integer value to store.
  """
  ...

def get_int(key: str, index: int | None = None) -> int:
  """Read an integer value from the current config section.

  Args:
    key: The key name to read from.
    index: If ``None``, returns a single value (choosing randomly when the
      value is a list). If an integer, returns the item at that index from
      the list.

  Returns:
    The integer value stored under ``key``.
  """
  ...

def set_guid(key: str, value: Guid) -> None:
  """Write a GUID value to the current config section.

  Args:
    key: The key name to write to.
    value: The GUID value to store.
  """
  ...

def get_guid(key: str, index: int | None = None) -> Guid:
  """Read a GUID value from the current config section.

  Args:
    key: The key name to read from.
    index: If ``None``, returns a single value (choosing randomly when the
      value is a list). If an integer, returns the item at that index from
      the list.

  Returns:
    The GUID value stored under ``key``.
  """
  ...

def set_float(key: str, value: float) -> None:
  """Write a float value to the current config section.

  Args:
    key: The key name to write to.
    value: The float value to store.
  """
  ...

def get_float(key: str, index: int | None = None) -> float:
  """Read a float value from the current config section.

  Args:
    key: The key name to read from.
    index: If ``None``, returns a single value (choosing randomly when the
      value is a list). If an integer, returns the item at that index from
      the list.

  Returns:
    The float value stored under ``key``.
  """
  ...

def set_string(key: str, value: str) -> None:
  """Write a string value to the current config section.

  Args:
    key: The key name to write to.
    value: The string value to store.
  """
  ...

def get_string(key: str, index: int | None = None) -> str:
  """Read a string value from the current config section.

  Args:
    key: The key name to read from.
    index: If ``None``, returns a single value (choosing randomly when the
      value is a list). If an integer, returns the item at that index from
      the list.

  Returns:
    The string value stored under ``key``.
  """
  ...

def set_vector(key: str, value: Vector) -> None:
  """Write a vector value to the current config section.

  Args:
    key: The key name to write to.
    value: The vector value to store.
  """
  ...

def get_vector(key: str, index: int | None = None) -> Vector:
  """Read a vector value from the current config section.

  Args:
    key: The key name to read from.
    index: If ``None``, returns a single value (choosing randomly when the
      value is a list). If an integer, returns the item at that index from
      the list.

  Returns:
    The vector value stored under ``key``.
  """
  ...

def has_section(name: str) -> bool:
  """Check whether a config section exists.

  Args:
    name: The section name to check for.

  Returns:
    ``True`` if the section exists, ``False`` otherwise.
  """
  ...

def has_value(key: str, check_spelling: bool = True) -> bool:
  """Check whether a value exists for the given key in the current section.

  Args:
    key: The key name to check for.
    check_spelling: When ``True`` (default), performs the standard check
      including spelling/typo detection. When ``False``, skips the spelling
      check which can be useful for performance-sensitive lookups.

  Returns:
    ``True`` if a value is defined for ``key``, ``False`` otherwise.
  """
  ...

def clear_section(name: str) -> None:
  """Clear all keys and values from a config section.

  The section itself will still exist after this call; only its contents are
  removed. Note that protected sections resist this operation.

  Args:
    name: The name of the section to clear.
  """
  ...

def clear_value(key: str) -> None:
  """Clear a value from the current config section.

  Removes the key/value pair identified by ``key`` from the currently
  selected section.

  Args:
    key: The key name to remove.
  """
  ...
