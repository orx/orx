# pyright: strict

from dataclasses import dataclass
from typing import Callable, Self

import orx

@dataclass
class Collision:
  body_part_name: str
  collider: orx.object.Object
  collider_body_part_name: str
  position: orx.vector.Vector
  normal: orx.vector.Vector

@dataclass
class Separation:
  body_part_name: str
  collider: orx.object.Object
  collider_body_part_name: str

class Context:
  def __init__(self, section: str, input_set: str | None = None):
    self.config_section = section
    self.input_set = input_set

  def __enter__(self) -> None:
    orx.config.push_section(self.config_section)
    if self.input_set:
      orx.input.push_set(self.input_set)

  def __exit__(self, *_: None) -> None:
    orx.config.pop_section()
    if self.input_set:
      orx.input.pop_set()

class Mini:
  o: orx.object.Object
  context: Context
  instance_context: Context
  input_names: list[str]
  _objects: dict[orx.object.Object, Self] = {}

  def __init__(self, o: orx.object.Object):
    self.o = o
    config_section = self.o.get_name()

    input_set_name = None
    self.input_names = []
    with orx.config.Section(config_section):
      # Initialize input
      if orx.config.has_value("Input"):
        input_set_name = orx.config.get_string("Input")
        orx.input.enable_set(input_set_name)
        with orx.input.Set(input_set_name):
          self.input_names = orx.input.get_all()
    self.context = Context(config_section, input_set_name)
    self.instance_context = Context(str(self.o), input_set_name)

    # Remember the object
    Mini._objects[o] = self

    # Object-specific on_create initialization
    with self.context:
      self.on_create()

  @classmethod
  def __cleanup__(cls, o: orx.object.Object):
    """Internal only: Remove object association with the class"""
    _ = Mini._objects.pop(o, None)

  @classmethod
  def exists(cls, o: orx.object.Object) -> bool:
    """Check if an object has an instance associated with this class"""
    instance = Mini._objects.get(o)
    return instance is not None and isinstance(instance, cls)

  @classmethod
  def objects(cls):
    """All objects associated with this class"""
    for value in Mini._objects.values():
      if isinstance(value, cls):
        yield value

  @classmethod
  def create_from_config(cls, name: str) -> Self | None:
    """Create a new object using the section `name`"""
    o = orx.object.create_from_config(name)
    if o is None:
      return None
    return cls.from_object(o)

  @classmethod
  def from_object(cls, o: orx.object.Object) -> Self | None:
    """Find the object from this class associated with `o` if one exists"""
    instance = cls._objects.get(o)
    if instance is not None and isinstance(instance, cls):
      return instance
    return None

  @classmethod
  def from_guid(cls, guid: orx.guid.Guid) -> Self | None:
    """Find the object from this class with the GUID `guid` if one exists"""
    o = orx.object.from_guid(guid)
    if o is None:
      return None
    return cls.from_object(o)

  def push_config_section(self, instance: bool = False):
    """Push the config section associated with this object onto the stack"""
    section_name = str(self.o) if instance else self.o.get_name()
    orx.config.push_section(section_name)

  def pop_config_section(self):
    """Pop the latest config section from the stack"""
    orx.config.pop_section()

  def on_create(self) -> None: ...
  """Called on object creation"""

  def on_delete(self) -> None: ...
  """Called on object deletion"""

  def on_update(self, dt: float): ...
  """Called each clock update"""

  def on_collide(self, collision: Collision): ...
  """Called on collision with other Mini-derived objects"""

  def on_separate(self, separation: Separation): ...
  """Called on separation from collision with other Mini-derived objects"""

  def on_shader_param(self, shader_name: str, param_name: str, param_type: type[float | orx.vector.Vector]) -> float | orx.vector.Vector : ...
  """Called on shader parameter lookups for any shaders associated with the object"""

# Tracking class and object associations

classes: dict[str, type[Mini]] = {}
"""
Bindings from configuration section names (key) to Python classes matching
those names
"""

def _find_bound_class(o: orx.object.Object) -> type[Mini] | None:
  """
  Find class bound to a name in object's config section hierarchy
  """
  section_name = o.get_name()
  while section_name not in classes:
    parent = orx.config.get_parent(section_name)
    if parent is None:
      # No further config parent to check, so this object is not bound to a class
      return None
    section_name = parent
  # name exists as a key in classes
  return classes[section_name]

# Engine callbacks

def on_create(o: orx.object.Object):
  """Engine object creation callback"""
  # Find a name in the object's hierarchy with a bound class
  bound = _find_bound_class(o)
  # Nothing to do if this object does not match a bound class
  if bound is None:
    return
  # Create a wrapper Python object if one not been created yet
  if not bound.exists(o):
    _ = bound(o)

def on_delete(o: orx.object.Object):
  """Engine object deletion callback"""
  bound = _find_bound_class(o)
  if bound is None:
    return
  mini = bound.from_object(o)
  if mini is None:
    return
  bound.__cleanup__(o)
  with mini.context:
    mini.on_delete()

_per_frame_update: Callable[[float], None] | None = None

def on_update(dt: float):
  """Engine clock update callback"""
  for mini in Mini.objects():
    with mini.context:
      # Input triggers
      for input_name in mini.input_names:
        if orx.input.has_been_activated(input_name):
          mini.o.fire_trigger("Input", refinement=[input_name])
      # Object clock
      clock = mini.o.get_clock()
      if clock is not None:
          # Scale dt based on object's clock
          dt = clock.compute_dt(dt)
      # Object update callback
      mini.on_update(dt)

  if _per_frame_update is not None:
    _per_frame_update(dt)

def on_collide(o1: orx.object.Object, body_name1: str, o2: orx.object.Object, body_name2: str, position: orx.vector.Vector, normal: orx.vector.Vector):
  """Engine callback for object collision events"""
  mini = Mini.from_object(o1)
  if mini:
    with mini.context:
      mini.on_collide(Collision(body_name1, o2, body_name2, position, normal))
  mini = Mini.from_object(o2)
  if mini:
    with mini.context:
      mini.on_collide(Collision(body_name2, o1, body_name1, position, normal))

def on_separate(o1: orx.object.Object, body_name1: str, o2: orx.object.Object, body_name2: str):
  """Engine callback for object separation events"""
  mini = Mini.from_object(o1)
  if mini:
    with mini.context:
      mini.on_separate(Separation(body_name1, o2, body_name2))
  mini = Mini.from_object(o2)
  if mini:
    with mini.context:
      mini.on_separate(Separation(body_name2, o1, body_name1))

def on_shader_param(o: orx.object.Object, shader_name: str, param_name: str, param_type: type[float | orx.vector.Vector]) -> float | orx.vector.Vector | None:
  """Engine callback for setting shader parameters"""
  mini = Mini.from_object(o)
  if mini:
    with mini.context:
      arg = mini.on_shader_param(shader_name, param_name, param_type)
    return arg

def setup(init: Callable[[], None] | None = None, update: Callable[[float], None] | None = None):
  global _per_frame_update
  if init is not None:
    orx.on_init = init
  if update is not None:
    _per_frame_update = update
  orx.on_update = on_update
  orx.on_create = on_create
  orx.on_delete = on_delete
  orx.on_collide = on_collide
  orx.on_separate = on_separate
  orx.on_shader_param = on_shader_param
