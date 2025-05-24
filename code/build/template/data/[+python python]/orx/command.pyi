from typing import Callable
from guid import Guid

# Command parameter types
STRING: int
FLOAT: int
INT: int
GUID: int
BOOL: int
VECTOR: int

def evaluate(command: str, guid: Guid | None = None) -> None: ...

def register(
    name: str,
    callback: Callable,
    required: list[tuple[str, int]],
    optional: list[tuple[str, int]],
    result_type: int,
) -> None: ...

def unregister(name: str) -> None: ...
