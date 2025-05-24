from typing import Callable
from guid import Guid

# Command parameter types

STRING: int = ...
"""Command parameter/result type constant for string values (`str`)."""

FLOAT: int = ...
"""Command parameter/result type constant for floating-point values (`float`)."""

INT: int = ...
"""Command parameter/result type constant for integer values (`int`)."""

GUID: int = ...
"""Command parameter/result type constant for GUID values (`Guid`)."""

BOOL: int = ...
"""Command parameter/result type constant for boolean values (`bool`)."""

VECTOR: int = ...
"""Command parameter/result type constant for vector values (`Vector`)."""


def evaluate(command: str, guid: Guid | None = None) -> None:
  """Evaluate a command string, optionally substituting a GUID.

  Parses and executes the given command line.  If *guid* is provided it is
  substituted wherever the GUID marker character (``^``) appears in the command
  string.

  Args:
    command: Full command line string, including the command name and any
      arguments (e.g. ``"Object.Enable ^ true"``).
    guid: Optional `Guid` whose value replaces GUID markers in *command*.  Pass
      ``None`` (the default) to evaluate the command without a GUID substitution.
  """
  ...


def register(
    name: str,
    callback: Callable,
    required: list[tuple[str, int]],
    optional: list[tuple[str, int]],
    result_type: int,
) -> None:
  """Register a Python function as an orx command.

  Registers a new command that can be evaluated by name from anywhere that orx
  commands are accepted (config, the in-game console, ``evaluate``, etc.).
  Up to 32 Python commands may be registered simultaneously.

  Each parameter is described as a ``(name, type)`` tuple where *type* is one
  of the module-level type constants (`STRING`, `FLOAT`, `INT`, `GUID`, `BOOL`,
  `VECTOR`).

  When the command is invoked the *callback* is called with positional arguments
  whose Python types match the declared parameter types:

  * `STRING` → `str`
  * `FLOAT`  → `float`
  * `INT`    → `int`
  * `GUID`   → `Guid`
  * `BOOL`   → `bool`
  * `VECTOR` → `Vector`

  The callback must return a value of the Python type corresponding to
  *result_type*.

  Example::

      def greet(name: str) -> str:
          return f"Hello, {name}!"

      orx.command.register(
          "MyGame.Greet",
          greet,
          required=[("Name", orx.command.STRING)],
          optional=[],
          result_type=orx.command.STRING,
      )

  Args:
    name: Unique command name (e.g. ``"MyGame.Greet"``).  Must not already be
      registered.
    callback: Python callable that implements the command.  It receives one
      positional argument per entry in *required* and *optional* (of the
      corresponding Python type) and must return a value matching *result_type*.
    required: List of ``(name, type)`` tuples describing mandatory parameters.
      Each *name* is a human-readable label and *type* is one of the type
      constants defined in this module.
    optional: List of ``(name, type)`` tuples describing optional parameters,
      using the same format as *required*.
    result_type: Type constant for the value returned by *callback*.  Must be
      one of `STRING`, `FLOAT`, `INT`, `GUID`, `BOOL`, or `VECTOR`.

  Raises:
    ValueError: If *callback* is not callable, a parameter list entry is not a
      valid ``(name, type)`` tuple, or a type value is not a recognised type
      constant.
    RuntimeError: If the 32-command limit has been reached or the underlying
      orx registration fails.
  """
  ...


def unregister(name: str) -> None:
  """Unregister a previously registered Python command.

  Removes the command with the given *name* from the orx command system and
  frees the internal wrapper slot so it can be reused.

  Args:
    name: The command name that was passed to `register`.

  Raises:
    ValueError: If no Python command with the given *name* is currently
      registered.
    RuntimeError: If the underlying orx unregistration fails.
  """
  ...
