# !/bin/bash
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:../lib/dynamic:./modules:./plugins/core/display:./plugins/core/file:./plugins/core/joystick:./plugins/core/keyboard:./plugins/core/mouse:./plugins/core/package:./plugins/core/script:./plugins/core/sound:./plugins/core/time"
./orx -P plugins/core/time/Time_SFML.so plugins/core/display/Display_SFML.so plugins/core/render/Render_Home.so plugins/demo/Scroll.so

