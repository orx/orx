"""pyscroll - Python bindings for orx game object behaviors.

Maps orx config sections to Python classes via the ``bind`` or ``bind_all``
functions.  When an orx object is created whose config hierarchy matches a
bound section, pyscroll automatically instantiates the corresponding Python
class and routes engine callbacks to it.

Inspired by the Scroll C++ extension provided with orx.
"""
from dataclasses import dataclass
from typing import Callable, Self

import orx

@dataclass
class Collision:
  """Data describing a physics collision event, passed to ``on_collide``."""
  body_part_name: str
  collider: orx.object.Object
  collider_body_part_name: str
  position: orx.vector.Vector
  normal: orx.vector.Vector

@dataclass
class Separation:
  """Data describing a physics separation event, passed to ``on_separate``."""
  body_part_name: str
  collider: orx.object.Object
  collider_body_part_name: str

class _Context:
  def __init__(self, section: str, input_set: str | None = None):
    self.config_section: str = section
    self.input_set: str | None = input_set

  def __enter__(self) -> Self:
    orx.config.push_section(self.config_section)
    if self.input_set is not None:
      orx.input.push_set(self.input_set)
    return self

  def __exit__(self, *_: None) -> None:
    orx.config.pop_section()
    if self.input_set is not None:
      orx.input.pop_set()

_all_objects: dict[orx.object.Object, "ScrollObject"] = {}
"""All live pyscroll instances keyed by their orx ``Object``."""

class ScrollObject:
  """Base class for all pyscroll game objects.

  Subclass this and use ``bind`` or ``bind_all`` to associate the subclass
  with an orx config section.  When a matching orx object is created,
  pyscroll instantiates the subclass and routes engine callbacks to it.

  Override ``on_update``, ``on_collide``, etc. to define behavior.

  ``__init__`` is called with the class's config section already pushed onto
  the config stack, so ``orx.config.get_*()`` reads from the bound section
  directly.
  """
  o: orx.object.Object # pyright: ignore[reportUninitializedInstanceVariable]
  """The orx ``Object`` associated with this instance."""
  context: _Context # pyright: ignore[reportUninitializedInstanceVariable]
  """Context manager for the class-level config section and input set.

  Pushing it selects the section matching the bound config name and enables
  the associated input set (if one is configured).
  """
  instance_context: _Context # pyright: ignore[reportUninitializedInstanceVariable]
  """Context manager for the instance-level config section and input set.

  Pushing it selects the runtime instance section (the object's GUID as a
  string) and enables the associated input set (if one is configured).
  """

  @classmethod
  def _init_instance(cls, o: orx.object.Object) -> Self:
    """Creates and initializes a new pyscroll instance wrapping ``o``."""
    instance = super().__new__(cls)

    instance.o = o
    config_section = instance.o.get_name()

    input_set_name = None
    with orx.config.Section(config_section):
      # Initialize input
      if orx.config.has_value("Input"):
        input_set_name = orx.config.get_string("Input")
        orx.input.enable_set(input_set_name)

    instance.context = _Context(config_section, input_set_name)
    instance.instance_context = _Context(str(instance.o), input_set_name)

    # Remember the object
    _all_objects[o] = instance

    # Call __init__ with config context
    if hasattr(instance, "__init__"):
      with instance.context:
        instance.__init__()

    return instance

  @classmethod
  def exists(cls, o: orx.object.Object) -> bool:
    """Returns ``True`` if ``o`` has a live instance associated with this class."""
    instance = _all_objects.get(o)
    return instance is not None and isinstance(instance, cls)

  @classmethod
  def all(cls) -> list[Self]:
    """Returns all live instances of this class."""
    return [value for value in _all_objects.values() if isinstance(value, cls)]

  @classmethod
  def create_from_config(cls, name: str) -> Self | None:
    """Creates a new orx object from config section ``name`` and returns its wrapper.

    Args:
      name: Config section name to create the object from.

    Returns:
      The new instance of this class, or ``None`` if the object could not be
      created or had no bound class.
    """
    o = orx.object.create_from_config(name)
    if o is None:
      return None
    return cls.find(o)

  @classmethod
  def find(cls, obj: orx.object.Object | orx.guid.Guid) -> Self | None:
    """Returns the instance of this class wrapping ``obj``, or ``None``.

    Args:
      obj: An ``Object`` or ``Guid`` to look up.

    Returns:
      The matching instance of this class, or ``None`` if no bound instance
      exists.
    """
    o = obj if isinstance(obj, orx.object.Object) else orx.object.from_guid(obj)
    if o is None:
      return None
    instance = _all_objects.get(o)
    return instance if isinstance(instance, cls) else None

  def push_config_section(self, instance: bool = False) -> None:
    """Pushes the config section for this object onto the stack.

    Args:
      instance: If ``False`` (default), pushes the class-level section (the
        object's name). If ``True``, pushes the instance-level section (the
        object's GUID string).
    """
    section_name = str(self.o) if instance else self.o.get_name()
    orx.config.push_section(section_name)

  def pop_config_section(self) -> None:
    """Pops the most recently pushed config section from the stack."""
    orx.config.pop_section()

  def on_delete(self) -> None:
    """Called when this object is deleted."""
    ...

  def on_spawn(self, spawned: orx.object.Object) -> None:
    """Called when ``spawned`` is created by a spawner owned by this object."""
    ...

  def on_update(self, dt: float) -> None:
    """Called each clock tick with the elapsed time ``dt`` in seconds."""
    ...

  def on_collide(self, collision: Collision) -> None:
    """Called when this object begins a physics collision."""
    ...

  def on_separate(self, separation: Separation) -> None:
    """Called when this object ends a physics collision."""
    ...

  def on_shader_param(self, shader_name: str, param_name: str, param_type: type[float | orx.vector.Vector]) -> float | orx.vector.Vector:
    """Called when the engine needs a shader parameter value for this object.

    Raises:
      NotImplementedError: If not overridden by a subclass.
    """
    raise NotImplementedError(f"{type(self).__name__}.on_shader_param is not implemented")

# Tracking class and object associations

_classes: dict[str, type[ScrollObject]] = {}
"""Bindings from config section names to their associated ``ScrollObject`` subclasses."""

def bind(class_: type[ScrollObject], name: str | None = None):
  """Binds a ``ScrollObject`` subclass to an orx config section name.

  When an orx object is created whose config hierarchy includes ``name``,
  pyscroll will instantiate ``class_`` and route engine callbacks to it.

  Args:
    class_: The ``ScrollObject`` subclass to bind.
    name: Config section name to bind to. Defaults to ``class_.__name__``.
  """
  _classes[name if name is not None else class_.__name__] = class_

def bind_all(bindings: dict[str, type[ScrollObject]]):
  """Binds multiple ``ScrollObject`` subclasses at once.

  Equivalent to calling ``bind(class_, name)`` for each ``(name, class_)``
  pair in ``bindings``.

  Args:
    bindings: Mapping from config section names to ``ScrollObject`` subclasses.
  """
  for name, class_ in bindings.items():
      bind(class_, name)

def _find_bound_class(o: orx.object.Object) -> type[ScrollObject] | None:
  """Returns the ``ScrollObject`` subclass bound to ``o``'s config hierarchy, or ``None``."""
  section_name = o.get_name()
  while section_name not in _classes:
    parent = orx.config.get_parent(section_name)
    if parent is None:
      # No further config parent to check, so this object is not bound to a class
      return None
    section_name = parent
  # name exists as a key in _classes
  return _classes[section_name]

# Engine callbacks

def on_create(o: orx.object.Object):
  """Called when an object is created."""
  # Find a name in the object's hierarchy with a bound class
  bound = _find_bound_class(o)
  # Nothing to do if this object does not match a bound class
  if bound is None:
    return
  # Create a wrapper Python object if one not been created yet
  if not bound.exists(o):
    # Setup wrapper object
    _ = bound._init_instance(o)  # pyright: ignore[reportPrivateUsage]

def on_delete(o: orx.object.Object):
  """Called when an object is deleted."""
  bound = _find_bound_class(o)
  if bound is None:
    return
  base = bound.find(o)
  if base is None:
    return
  with base.context:
    base.on_delete()
  _ = _all_objects.pop(o, None)

def on_spawn(spawner: orx.object.Object, spawned: orx.object.Object):
  """Called when an object is spawned."""
  base = ScrollObject.find(spawner)
  if base is None:
    return
  with base.context:
    base.on_spawn(spawned)

_per_frame_update: Callable[[float], None] | None = None

def _set_per_frame_update(update: Callable[[float], None] | None) -> None:
  """Sets the per-frame update callback."""
  global _per_frame_update
  _per_frame_update = update

def _call_per_frame_update(dt: float) -> None:
  """Calls the per-frame update callback if one is set."""
  global _per_frame_update
  if _per_frame_update is not None:
    _per_frame_update(dt)

def _fire_input_trigger(o: orx.object.Object, input_name: str) -> None:
  """Fires input trigger events on ``o`` for the given input."""
  # Get input status and value
  new_status = orx.input.has_new_status(input_name)
  active = orx.input.is_active(input_name)
  value = orx.input.get_value(input_name)

  # Build trigger refinements
  if new_status:
    input_name_prefix = "!." if active else "!-"
  else:
    input_name_prefix = "!" if active else "!-"

  refinements = [f"{input_name_prefix}{input_name}", f"{value}"]

  # Fire trigger
  fired = o.fire_trigger("Input", refinement=refinements)
  if not fired and new_status:
    # Fire the non-instant trigger event
    refinements[0] = input_name
    _ = o.fire_trigger("Input", refinement=refinements)

def on_update(dt: float):
  """Called with every tick of the core clock."""
  for base in ScrollObject.all():
    if base.o.is_paused() or base.o.get_life_time() == 0.0:
      # Skip object if it is paused or pending deletion
      continue
    with base.context:
      # Input triggers
      input_name: str | None = None
      while (input_name := orx.input.get_next(input_name)) is not None:
        _fire_input_trigger(base.o, input_name)
      # Object clock
      clock = base.o.get_clock()
      object_dt = clock.compute_dt(dt) if clock is not None else dt
      # Object update callback
      base.on_update(object_dt)

  _call_per_frame_update(dt)

def on_collide(o1: orx.object.Object, body_name1: str, o2: orx.object.Object, body_name2: str, position: orx.vector.Vector, normal: orx.vector.Vector):
  """Called when two objects collide."""
  base = ScrollObject.find(o1)
  if base:
    with base.context:
      base.on_collide(Collision(body_name1, o2, body_name2, position, normal))
  base = ScrollObject.find(o2)
  if base:
    with base.context:
      base.on_collide(Collision(body_name2, o1, body_name1, position, normal))

def on_separate(o1: orx.object.Object, body_name1: str, o2: orx.object.Object, body_name2: str):
  """Called when two objects separate."""
  base = ScrollObject.find(o1)
  if base:
    with base.context:
      base.on_separate(Separation(body_name1, o2, body_name2))
  base = ScrollObject.find(o2)
  if base:
    with base.context:
      base.on_separate(Separation(body_name2, o1, body_name1))

def on_shader_param(o: orx.object.Object, shader_name: str, param_name: str, param_type: type[float | orx.vector.Vector]) -> float | orx.vector.Vector | None:
  """Called when a shader parameter is requested."""
  base = ScrollObject.find(o)
  if base:
    with base.context:
      arg = base.on_shader_param(shader_name, param_name, param_type)
    return arg

def setup(init: Callable[[], None] | None = None, update: Callable[[float], None] | None = None):
  """Registers pyscroll's engine callbacks with orx.

  Call this once during startup to wire pyscroll into the engine lifecycle.
  ``init`` is called during engine initialization; ``update`` is called once
  per frame in addition to per-object ``on_update`` callbacks.

  Args:
    init: Optional function called once when the engine is initialized.
    update: Optional function called once per frame.
  """
  if init is not None:
    orx.on_init = init
  _set_per_frame_update(update)
  orx.on_update = on_update
  orx.on_create = on_create
  orx.on_spawn = on_spawn
  orx.on_delete = on_delete
  orx.on_collide = on_collide
  orx.on_separate = on_separate
  orx.on_shader_param = on_shader_param
