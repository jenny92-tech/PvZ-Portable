# Plants vs. Zombies Portable

An open-source reimplementation of Plants vs. Zombies GOTY, packaged for
PortMaster handhelds. The game assets are not included: copy `main.pak` and the
`properties/` folder from your own legally obtained copy into the
`pvz_portable` folder.

## Controls

| Button       | Action                                                          |
| ------------ | --------------------------------------------------------------- |
| D-pad        | Move the cursor (steps cell by cell on the lawn)                 |
| Any joystick | Move the cursor                                                  |
| A            | Click: plant the selected seed, or collect sun under the cursor  |
| B            | Grab the shovel; with something on the cursor, put it back       |
| X            | Open the store, swing the whacking hammer, pull the slot lever   |
| Y            | Wake Stinky in the Zen Garden                                    |
| L1 / R1      | Select the previous / next seed packet                           |
| L3           | Hold to move the cursor faster                                   |
| R2           | Hold to run the game at 3x, for waiting out a lull               |
| L2           | Hold to run the game a quarter slower                            |
| Start/Select | Pause; in menus and dialogs, go back                             |

R2 and L2 only act during a level, so the menus always run at their own pace.

Holding A over the lawn keeps clicking, so sweeping the cursor collects the sun
it passes.

## In-game settings

The options menu has a **Controller** entry with cursor speed, the auto-collect
radius, a free-cursor toggle (off: the cursor is held on the lawn during normal
play; on: it may reach the whole screen), a toggle for the L3 cursor sprint, and
A/B and X/Y swaps for pads whose face buttons are labelled the other way round.
Settings are saved.

There is also a **Cheats** entry: free planting, infinite sun, a button that
adds coins, and one that finishes the current level outright.

A short controls card is shown every time the game starts, and both screens are
reachable from the **Port Options** entry in the options menu.

## Credits

- Original game by **PopCap Games** / **Electronic Arts**.
- Engine: [**PvZ-Portable**](https://github.com/wszqkzqk/PvZ-Portable) by
  wszqkzqk — the open-source reimplementation everything here runs on.
- PortMaster packaging, the memory-mapped pak reader, and the stb_image switch
  build on [**PvZ-PortMaster**](https://github.com/nullptr97j/PvZ-PortMaster)
  by gama97, whose port proved them out on real handhelds first.
- This build: [jenny92-tech/PvZ-Portable](https://github.com/jenny92-tech/PvZ-Portable)
  — gamepad controls, in-game controller settings, and TrimUI fixes.

## License

The engine is LGPL-3.0-or-later; see the `licenses` folder for the components
bundled with this port. The original game assets remain the property of their
copyright holders and are not distributed here.
