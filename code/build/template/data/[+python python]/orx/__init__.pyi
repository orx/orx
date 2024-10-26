# Expose each submodule as `orx.<module_name>`
from . import command, config, input, object, vector

# Log a message to the terminal and console using orx's logging
def log(message: str) -> None: ...
