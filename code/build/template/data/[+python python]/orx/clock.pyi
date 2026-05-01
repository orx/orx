from typing import Callable

PRIORITY_LOWEST: int = ...
"""Lowest clock callback priority."""

PRIORITY_LOWER: int = ...
"""Lower clock callback priority."""

PRIORITY_LOW: int = ...
"""Low clock callback priority."""

PRIORITY_NORMAL: int = ...
"""Normal clock callback priority."""

PRIORITY_HIGH: int = ...
"""High clock callback priority."""

PRIORITY_HIGHER: int = ...
"""Higher clock callback priority."""

PRIORITY_HIGHEST: int = ...
"""Highest clock callback priority."""

class CallbackID:
  """Opaque handle representing a registered clock callback.

  Returned by `Clock.register` and passed to `Clock.unregister` to remove a
  previously registered callback.  A `CallbackID` becomes invalid after it has
  been passed to `Clock.unregister`.
  """

  def __eq__(self, other: object) -> bool: ...
  def __hash__(self) -> int: ...

  def is_valid(self) -> bool:
    """Return whether this `CallbackID` is still valid.

    A `CallbackID` is invalid once the corresponding callback has been
    unregistered via `Clock.unregister`.

    Returns:
      `True` if the callback ID is still valid, `False` otherwise.
    """
    ...

class Clock:
  """An orx clock object.

  Clocks are the low-level timing primitives used by orx to drive updates.
  Callbacks registered on a clock are called at every clock tick in priority
  order, each receiving the elapsed delta-time (DT) for that tick.

  Example::

      core_clock = orx.clock.create_from_config("orx:clock:core")
      if core_clock is not None:
          callback_id = core_clock.register(my_callback, orx.clock.PRIORITY_NORMAL)
  """

  def __new__(cls, name: str) -> Clock:
    """Look up or create a clock from a config section name.

    Equivalent to `create_from_config` when called as a constructor.  Raises
    `ValueError` if the named config section does not exist or the clock cannot
    be created.

    Args:
      name: Config section name identifying the clock.

    Returns:
      A `Clock` instance for the given config section.

    Raises:
      ValueError: If the clock cannot be created from the given config section.
    """
    ...

  def __eq__(self, other: object) -> bool: ...
  def __hash__(self) -> int: ...

  def compute_dt(self, dt: float) -> float:
    """Compute the modified DT for this clock given a raw DT value.

    Applies any modifiers (fixed, multiply, maxed, average) that are
    configured on this clock to the provided raw DT.

    Args:
      dt: Real (unmodified) delta-time in seconds.

    Returns:
      The modified delta-time in seconds after applying the clock's modifiers.
    """
    ...

  def register(self, callback: Callable[[float], None], priority: int) -> CallbackID:
    """Register a callback to be called on every tick of this clock.

    The callback will be invoked with the modified delta-time (DT) in seconds
    each time the clock ticks.  Callbacks are called in ascending priority order
    (lowest priority value first) on each tick.

    The returned `CallbackID` can be passed to `unregister` to stop the
    callback from being called.

    Args:
      callback: Callable that accepts a single `float` argument containing the
        DT in seconds.
      priority: One of the `PRIORITY_*` constants that controls the order in
        which callbacks are invoked within the same clock tick.

    Returns:
      A `CallbackID` that can be passed to `unregister` to remove this
      callback.

    Raises:
      ValueError: If `callback` is not callable or `priority` is out of range.
      RuntimeError: If the underlying orx registration fails.
    """
    ...

  def unregister(self, callback: CallbackID) -> None:
    """Unregister a previously registered clock callback.

    After this call the `CallbackID` becomes invalid and `is_valid` will
    return `False`.

    Args:
      callback: The `CallbackID` returned by a previous call to `register` on
        this clock.

    Raises:
      ValueError: If the `CallbackID` has already been unregistered.
      RuntimeError: If the underlying orx unregistration fails.
    """
    ...

def create_from_config(name: str) -> Clock | None:
  """Create or look up a clock from a config section name.

  Unlike the `Clock` constructor, this function returns `None` instead of
  raising an exception when the clock cannot be created.

  Args:
    name: Config section name identifying the clock.  Use `"orx:clock:core"`
      to obtain the main core clock.

  Returns:
    A `Clock` instance for the given config section, or `None` if the clock
    could not be created.
  """
  ...
