from clock import Clock
from guid import Guid
from vector import Vector

class Object:
  """Objects are structure containers that can be linked to many other
  structures such as graphics, animations, physics bodies, FX, spawners,
  sounds, shaders, clocks, etc.

  Objects are heavily data-driven. The most common way to create an object is
  to define its components in config then create objects based on the config
  definition.
  """

  def __new__(cls, name: str | Guid) -> Object:
    """Creates an Object from a config section name or retrieves an existing
    one by GUID.

    When called as a constructor (e.g. ``Object("MyObject")``), raises a
    ``ValueError`` if the object cannot be created or found.

    Args:
      name: Config section name to create an object from, or a ``Guid`` to
        look up an existing object.

    Returns:
      The created or retrieved ``Object``.

    Raises:
      ValueError: If called as a constructor and the object cannot be created
        or found.
    """
    ...
  def __eq__(self, other: object) -> bool: ...
  def __hash__(self) -> int: ...
  def __str__(self) -> str:
    """Returns the object's GUID as a hex string (e.g. ``"0x000000000000001A"``)."""
    ...

  def delete(self) -> None:
    """Deletes this object immediately.

    **Warning**:
      This is *unsafe* when called from an event handler. Use
      ``set_life_time(0)`` instead to schedule deletion at the end of the
      current frame.
    """
    ...

  def get_guid(self) -> Guid:
    """Returns the object's GUID."""
    ...

  def enable(self, state: bool, recursive: bool = False) -> None:
    """Enables or disables this object.

    When ``recursive`` is ``False`` the change only affects this object and
    not its children. Pass ``recursive=True`` to also enable/disable all
    owned children.

    Args:
      state: ``True`` to enable, ``False`` to disable.
      recursive: If ``True``, also enables/disables all owned children.
    """
    ...

  def is_enabled(self) -> bool:
    """Returns ``True`` if this object is currently enabled."""
    ...

  def pause(self, state: bool, recursive: bool = False) -> None:
    """Pauses or unpauses this object.

    When ``recursive`` is ``False`` the change only affects this object and
    not its children. Pass ``recursive=True`` to also pause/unpause all
    owned children.

    Args:
      state: ``True`` to pause, ``False`` to unpause.
      recursive: If ``True``, also pauses/unpauses all owned children.
    """
    ...

  def is_paused(self) -> bool:
    """Returns ``True`` if this object is currently paused."""
    ...

  def set_owner(self, owner: Object | None) -> None:
    """Sets the owner of this object.

    Ownership in orx is about *lifetime management*: when an owner object is
    deleted, it also deletes all objects it owns.  This is different from
    parenthood (see ``set_parent()``), which is about the transformation
    hierarchy.

    Args:
      owner: The new owner, or ``None`` to clear ownership.
    """
    ...

  def get_owner(self) -> Object | None:
    """Returns this object's owner, or ``None`` if it has no owner."""
    ...

  def find_owned_child(self, path: str) -> Object | None:
    """Finds a child in this object's owner hierarchy matching a path.

    Paths are dot-separated object names, e.g. ``"Parent.Child"``.  The
    following special forms are supported:

    - ``*`` matches any child at any depth (depth-first search).
    - A name prefixed with ``@`` is matched against the full config ancestry
      of each object, not just its own name.
    - ``[N]`` selects the N+1th (0-based) matching object.

    Examples::

      obj.find_owned_child("Higher.Lower")
      obj.find_owned_child("Higher.*.Deep")
      obj.find_owned_child("*.@Parent")
      obj.find_owned_child("*.Other[2]")
      obj.find_owned_child("Higher.[1]")

    Args:
      path: Path string describing the child to find.

    Returns:
      The matching ``Object``, or ``None`` if not found.
    """
    ...

  def set_clock(self, clock: Clock, recursive: bool = False) -> None:
    """Associates a clock with this object.

    Args:
      clock: The clock to associate.
      recursive: If ``True``, also sets the clock on all owned children.
    """
    ...

  def get_clock(self) -> Clock | None:
    """Returns the clock associated with this object, or ``None``."""
    ...

  def set_flip(self, flip_x: bool, flip_y: bool) -> None:
    """Sets horizontal and/or vertical flipping for this object.

    Args:
      flip_x: ``True`` to flip along the X axis.
      flip_y: ``True`` to flip along the Y axis.
    """
    ...

  def get_flip(self) -> tuple[bool, bool]:
    """Returns flip state of this object as a ``(flip_x, flip_y)``  tuple."""
    ...

  def set_position(self, position: Vector, world: bool = False) -> None:
    """Sets this object's position.

    Args:
      position: Target position.
      world: If ``False`` (default), the position is in the parent's reference
        frame. If ``True``, the position is in the global (world) reference
        frame.
    """
    ...

  def get_position(self, world: bool = False) -> Vector:
    """Returns this object's position.

    Args:
      world: If ``False`` (default), returns position in the parent's reference
        frame. If ``True``, returns the world-space position.

    Returns:
      The position as a ``Vector``.
    """
    ...

  def set_rotation(self, rotation: float, world: bool = False) -> None:
    """Sets this object's rotation (in radians).

    Args:
      rotation: Rotation in radians.
      world: If ``False`` (default), sets rotation in the parent's reference
        frame. If ``True``, sets the world-space rotation.
    """
    ...

  def get_rotation(self, world: bool = False) -> float:
    """Returns this object's rotation in radians.

    Args:
      world: If ``False`` (default), returns the rotation in the parent's
        reference frame. If ``True``, returns the world-space rotation.
    """
    ...

  def set_scale(self, scale: Vector, world: bool = False) -> None:
    """Sets this object's scale.

    Args:
      scale: Scale vector to set.
      world: If ``False`` (default), sets the scale in the parent's reference
        frame. If ``True``, sets the world-space scale.
    """
    ...

  def get_scale(self, world: bool = False) -> Vector:
    """Returns this object's scale vector.

    Args:
      world: If ``False`` (default), returns the scale in the parent's
        reference frame. If ``True``, returns the world-space scale.
    """
    ...

  def set_parent(self, parent: Object | None) -> None:
    """Sets the parent of this object in the frame (transformation) hierarchy.

    Parenthood in orx is about the transformation hierarchy: position,
    rotation, and scale of children are relative to their parent.  This is
    different from ownership (see ``set_owner()``), which is about lifetime
    management.

    Args:
      parent: The new parent ``Object``, or ``None`` to detach from the
        current parent.
    """
    ...

  def get_parent(self) -> Object | None:
    """Returns this object's parent in the frame hierarchy, or ``None``."""
    ...

  def find_child(self, path: str) -> Object | None:
    """Finds a child in this object's frame hierarchy matching a path.

    Paths are dot-separated object names, e.g. ``"Parent.Child"``.  The
    following special forms are supported:

    - ``*`` matches any child at any depth (depth-first search).
    - A name prefixed with ``@`` is matched against the full config ancestry
      of each object, not just its own name.
    - ``[N]`` selects the N+1th (0-based) matching object.

    Examples::

      obj.find_child("Higher.Lower")
      obj.find_child("Higher.*.Deep")
      obj.find_child("*.Other[2]")
      obj.find_child("Higher.[1]")

    Args:
      path: Path string describing the child to find.

    Returns:
      The matching ``Object``, or ``None`` if not found.
    """
    ...

  def attach(self, parent: Object) -> None:
    """Attaches this object to a parent while maintaining its current world position.

    Args:
      parent: The parent object to attach to.
    """
    ...

  def detach(self) -> None:
    """Detaches this object from its parent while maintaining its current world position."""
    ...

  def log_parents(self) -> None:
    """Logs all parents of this object (including frame data) to the debug output."""
    ...

  def set_anim_frequency(self, frequency: float, recursive: bool = False) -> None:
    """Sets this object's relative animation frequency.

    Args:
      frequency: Frequency multiplier (< 1.0 for slower, > 1.0 for faster
        than normal).
      recursive: If ``True``, also sets the frequency on all owned children.
    """
    ...

  def get_anim_frequency(self) -> float:
    """Returns this object's relative animation frequency."""
    ...

  def set_anim_time(self, time: float, recursive: bool = False) -> None:
    """Sets this object's animation time.

    Args:
      time: Animation time to set (in seconds).
      recursive: If ``True``, also sets the animation time on all owned
        children.
    """
    ...

  def get_anim_time(self) -> float:
    """Returns this object's current animation time in seconds."""
    ...

  def set_current_anim(self, name: str, recursive: bool = False) -> None:
    """Immediately switches the current animation of this object.

    Unlike ``set_target_anim()``, this bypasses the animation link graph and
    switches the displayed animation right away.

    Args:
      name: Config name of the animation to set.
      recursive: If ``True``, also sets the animation on all owned children.
    """
    ...

  def get_current_anim(self) -> str:
    """Returns the name of the currently playing animation, or an empty string."""
    ...

  def set_target_anim(self, name: str, recursive: bool = False) -> None:
    """Sets the target animation for this object.

    The engine will follow the animation link graph defined by the object's
    ``AnimationSet`` to reach the target animation.  Use
    ``set_current_anim()`` to switch immediately without using the graph.

    Args:
      name: Config name of the target animation.
      recursive: If ``True``, also sets the target animation on all owned
        children.
    """
    ...

  def get_target_anim(self) -> str:
    """Returns the name of the target animation, or an empty string."""
    ...

  def set_speed(self, speed: Vector, relative: bool = False) -> None:
    """Sets this object's speed.

    Args:
      speed: Speed vector to set.
      relative: If ``True``, the speed is relative to the object's own
        rotation and scale. If ``False`` (default), it is in world space.
    """
    ...

  def get_speed(self, relative: bool = False) -> Vector:
    """Returns this object's speed.

    Args:
      relative: If ``True``, returns the speed relative to the object's own
        rotation and scale. If ``False`` (default), returns the world-space
        speed.
    """
    ...

  def set_angular_velocity(self, velocity: float) -> None:
    """Sets this object's angular velocity.

    Args:
      velocity: Angular velocity in radians per second.
    """
    ...

  def get_angular_velocity(self) -> float:
    """Returns this object's angular velocity in radians per second."""
    ...

  def set_custom_gravity(self, dir: Vector | None) -> None:
    """Sets a custom gravity vector for this object, overriding the global gravity.

    Args:
      dir: Custom gravity direction and magnitude, or ``None`` to clear it.
    """
    ...

  def get_custom_gravity(self) -> Vector | None:
    """Returns this object's custom gravity vector, or ``None`` if none is set."""
    ...

  def get_mass(self) -> float:
    """Returns this object's mass as defined by its physics body."""
    ...

  def get_mass_center(self) -> Vector:
    """Returns this object's center of mass in object space."""
    ...

  def apply_torque(self, torque: float) -> None:
    """Applies a torque to this object's physics body.

    Args:
      torque: Torque value to apply.
    """
    ...

  def apply_force(self, force: Vector, point: Vector) -> None:
    """Applies a force to this object's physics body.

    Args:
      force: Force vector to apply.
      point: World-space point at which the force is applied. If the zero
        vector is used, the center of mass is used.
    """
    ...

  def apply_impulse(self, impulse: Vector, point: Vector) -> None:
    """Applies an impulse to this object's physics body.

    Args:
      impulse: Impulse vector to apply.
      point: World-space point at which the impulse is applied. If the zero
        vector is used, the center of mass is used.
    """
    ...

  def set_text_string(self, s: str) -> None:
    """Sets the text string displayed by this object, if it has a text graphic.

    Args:
      s: The string to display.
    """
    ...

  def get_text_string(self) -> str:
    """Returns the text string of this object, or an empty string if none."""
    ...

  def add_fx(self, name: str, recursive: bool = False, unique: bool = True, propagation_delay: float = 0) -> None:
    """Adds an FX to this object using its config ID.

    Args:
      name: Config ID of the FX to add.
      recursive: If ``True``, also adds the FX to all owned children.
      unique: If ``True`` (default), only one instance of the FX will be
        active at a time. If ``False``, multiple simultaneous instances are
        allowed.
      propagation_delay: When ``recursive`` is ``True``, delay (in seconds)
        applied between each child when propagating the FX.
    """
    ...

  def remove_fx(self, name: str, recursive: bool = False) -> None:
    """Removes an FX from this object using its config ID.

    Args:
      name: Config ID of the FX to remove.
      recursive: If ``True``, also removes the FX from all owned children.
    """
    ...

  def remove_all_fxs(self, recursive: bool = False) -> None:
    """Removes all FXs from this object.

    Args:
      recursive: If ``True``, also removes all FXs from all owned children.
    """
    ...

  def add_sound(self, name: str) -> None:
    """Adds a sound to this object using its config ID.

    Args:
      name: Config ID of the sound to add.
    """
    ...

  def remove_sound(self, name: str) -> None:
    """Removes a sound from this object using its config ID.

    Args:
      name: Config ID of the sound to remove.
    """
    ...

  def remove_all_sounds(self) -> None:
    """Removes all sounds from this object."""
    ...

  def set_volume(self, volume: float) -> None:
    """Sets the volume for all sounds on this object.

    Args:
      volume: Desired volume in the range ``0.0`` (silent) to ``1.0`` (full).
    """
    ...

  def set_pitch(self, pitch: float) -> None:
    """Sets the pitch for all sounds on this object.

    Args:
      pitch: Pitch multiplier. Values below ``1.0`` lower the pitch, ``1.0``
        is the original pitch, values above ``1.0`` raise it. ``0.0`` is
        ignored.
    """
    ...

  def set_panning(self, panning: float, mix: bool) -> None:
    """Sets the stereo panning for all sounds on this object.

    Args:
      panning: Panning value: ``-1.0`` for full left, ``0.0`` for center,
        ``1.0`` for full right.
      mix: If ``True``, left and right channels are mixed. If ``False``,
        panning acts as a balance between the two channels.
    """
    ...

  def play(self) -> None:
    """Plays all sounds attached to this object."""
    ...

  def stop(self) -> None:
    """Stops all sounds attached to this object."""
    ...

  def add_filter(self, name: str) -> None:
    """Adds a sound filter to all sounds of this object (cascading).

    Args:
      name: Config ID of the filter to add.
    """
    ...

  def remove_last_filter(self) -> None:
    """Removes the last added sound filter from all sounds of this object."""
    ...

  def remove_all_filters(self) -> None:
    """Removes all sound filters from all sounds of this object."""
    ...

  def set_shader_from_config(self, name: str | None, recursive: bool = False) -> None:
    """Sets the shader on this object using its config ID.

    Args:
      name: Config ID of the shader, or ``None`` to remove the current shader.
      recursive: If ``True``, also sets the shader on all owned children.
    """
    ...

  def enable_shader(self, enabled: bool = True) -> None:
    """Enables or disables the shader on this object.

    Args:
      enabled: ``True`` (default) to enable, ``False`` to disable.
    """
    ...

  def is_shader_enabled(self) -> bool:
    """Returns ``True`` if this object's shader is currently enabled."""
    ...

  def add_time_line_track(self, name: str, recusive: bool = False) -> None:
    """Adds a timeline track to this object using its config ID.

    Args:
      name: Config ID of the timeline track to add.
      recusive: If ``True``, also adds the track to all owned children.
    """
    ...

  def remove_time_line_track(self, name: str, recursive: bool = False) -> None:
    """Removes a timeline track from this object using its config ID.

    Args:
      name: Config ID of the timeline track to remove.
      recursive: If ``True``, also removes the track from all owned children.
    """
    ...

  def enable_time_line(self, enabled: bool = True) -> None:
    """Enables or disables the timeline on this object.

    Args:
      enabled: ``True`` (default) to enable, ``False`` to disable.
    """
    ...

  def is_time_line_enabled(self) -> bool:
    """Returns ``True`` if this object's timeline is currently enabled."""
    ...

  def add_trigger(self, name: str, recursive: bool = False) -> None:
    """Adds a trigger to this object using its config ID.

    Args:
      name: Config ID of the trigger to add.
      recursive: If ``True``, also adds the trigger to all owned children.
    """
    ...

  def remove_trigger(self, name: str, recursive: bool = False) -> None:
    """Removes a trigger from this object using its config ID.

    Args:
      name: Config ID of the trigger to remove.
      recursive: If ``True``, also removes the trigger from all owned children.
    """
    ...

  def fire_trigger(self, name: str, refinement: list[str] | None = None, recursive: bool = False) -> None:
    """Fires a trigger on this object.

    Args:
      name: Name of the trigger event to fire.
      refinement: Optional list of refinement strings for the event.
      recursive: If ``True``, also fires the trigger on all owned children.
    """
    ...

  def get_name(self) -> str:
    """Returns this object's config section name, or an empty string."""
    ...

  def set_rgb(self, rgb: Vector, recursive: bool = False) -> None:
    """Sets the RGB color values of this object.

    Args:
      rgb: Vector whose ``x``, ``y``, ``z`` components map to R, G, B.
      recursive: If ``True``, also sets the color on all owned children.
    """
    ...

  def get_rgb(self) -> Vector:
    """Returns this object's RGB color values as a ``Vector``."""
    ...

  def set_alpha(self, alpha: float, recursive: bool = False) -> None:
    """Sets the alpha (opacity) of this object.

    Args:
      alpha: Alpha value to set from ``0.0`` transparent to ``1.0`` opaque.
      recursive: If ``True``, also sets the alpha on all owned children.
    """
    ...

  def get_alpha(self) -> float:
    """Returns this object's current alpha value."""
    ...

  def set_life_time(self, life_time: float | str | None) -> None:
    """Sets the lifetime of this object.

    The object will be automatically deleted when its lifetime expires.

    Args:
      life_time: Lifetime in seconds. Pass ``None`` to disable the lifetime
        so the object lives indefinitely. Pass a ``str`` to use a *literal*
        lifetime composed of one or more space-separated tags (``anim``,
        ``child``, ``fx``, ``sound``, ``spawner``, ``track``), which will keep
        the object alive until the corresponding conditions are met.
    """
    ...

  def get_life_time(self) -> float | None:
    """Returns this object's remaining lifetime in seconds, or ``None`` if unlimited."""
    ...

  def get_active_time(self) -> float:
    """Returns the total time this object has been active and alive.

    This takes into account the object's clock multiplier and any periods
    during which it was paused.
    """
    ...

  def reset_active_time(self, recursive: bool = False) -> None:
    """Resets this object's active time counter to zero.

    Args:
      recursive: If ``True``, also resets the active time on all owned
        children.
    """
    ...

def create_from_config(name: str) -> Object | None:
  """Creates an object from a config section.

  Args:
    name: Config section name to create the object from.

  Returns:
    The created ``Object``, or ``None`` on failure.
  """
  ...

def from_guid(guid: Guid) -> Object | None:
  """Retrieves an existing object by GUID.

  Args:
    guid: The GUID of the object to look up.

  Returns:
    The matching ``Object``, or ``None`` if no object with that GUID exists.
  """
  ...

def raycast(begin: Vector, end: Vector, self_flags: int, check_mask: int, early_exit: bool = False) -> tuple[Object, Vector, Vector] | None:
  """Issues a physics raycast between two points.

  Args:
    begin: Starting point of the ray in world space.
    end: Ending point of the ray in world space.
    self_flags: Self-flags used for collision filtering (``0xFFFF`` for no
      filtering).
    check_mask: Check-mask used for collision filtering (``0xFFFF`` for no
      filtering).
    early_exit: If ``True``, the raycast stops as soon as *any* object is
      hit, which may not be the closest one.

  Returns:
    A ``(object, contact, normal)`` tuple if the ray hits an object, where
    ``contact`` is the world-space contact point and ``normal`` is the
    surface normal at that point.  Returns ``None`` if nothing was hit.
  """
  ...
