#!/bin/bash
# PortMaster launcher for PvZ-Portable.
# Modeled on the reviewed PvZ-PortMaster launcher (nullptr97j/PvZ-PortMaster).

XDG_DATA_HOME=${XDG_DATA_HOME:-$HOME/.local/share}

if [ -d "/opt/system/Tools/PortMaster/" ]; then
  controlfolder="/opt/system/Tools/PortMaster"
elif [ -d "/opt/tools/PortMaster/" ]; then
  controlfolder="/opt/tools/PortMaster"
elif [ -d "$XDG_DATA_HOME/PortMaster/" ]; then
  controlfolder="$XDG_DATA_HOME/PortMaster"
else
  controlfolder="/roms/ports/PortMaster"
fi

source $controlfolder/control.txt
[ -f "$controlfolder/device_info.txt" ] && source $controlfolder/device_info.txt

[ -f "${controlfolder}/mod_${CFW_NAME}.txt" ] && source "${controlfolder}/mod_${CFW_NAME}.txt"

get_controls

GAMEDIR="/$directory/ports/pvz_portable"
BINARY="pvz_portable.${DEVICE_ARCH}"
CONFDIR="$GAMEDIR/conf/"
LIB_DIR="$GAMEDIR/libs.${DEVICE_ARCH}"

> "$GAMEDIR/log.txt" && exec > >(tee "$GAMEDIR/log.txt") 2>&1

cd $GAMEDIR
$ESUDO chmod +x $BINARY

export LD_LIBRARY_PATH="$LIB_DIR:$LD_LIBRARY_PATH"
export SDL_GAMECONTROLLERCONFIG="$sdl_controllerconfig"
export TEXTINPUTINTERACTIVE="Y"
export XDG_DATA_HOME="$CONFDIR"

# Gamepad tuning. Uncomment and edit to override what the in-game controller
# settings saved; the in-game values are used when these are unset.
#export PVZ_CURSOR_SENSITIVITY=1.4   # cursor speed, 0.5-2.0
#export PVZ_SUN_RADIUS=220           # auto-collect radius in pixels, 60-640
#export PVZ_SHOW_CONTROLS=1          # show the controls card on every launch

# Keep glibc's allocator lean on low-memory handhelds: no thread caches and a
# low mmap threshold so freed pages actually return to the OS.
export MALLOC_CHECK_=0
export GLIBC_TUNABLES="glibc.malloc.tcache_count=0:glibc.malloc.mmap_threshold=16384"

$GPTOKEYB "$BINARY" -c "$GAMEDIR/PvzPortable.gptk" textinput &
export IS_PORTMASTER=1

pm_platform_helper "$BINARY"
./$BINARY -resdir=$GAMEDIR -savedir=$GAMEDIR
pm_finish
