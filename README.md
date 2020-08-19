# Break Bricks

- Game/demo similar to the game [Breakout](https://en.wikipedia.org/wiki/Breakout_(video_game))
- Written in C
- On purpose, there is no "vector2" struct, in order to see how that would turn out
- The included `makefile` does incremental compilation
- The assets are not checked into the git repo and can be downloaded from [here](https://github.com/Costava/break-bricks/releases/tag/assets)
- Dependencies:
- - C standard library
- - SDL2
- - SDL2_image

![Break Bricks](https://i.imgur.com/eQeMZWc.png)
![Break Bricks](https://i.imgur.com/xkX8MjZ.png)

![Break Bricks](https://i.imgur.com/F3AWMgp.gif)

## How to run

1. Clone the repo
2. Download the assets from [here](https://github.com/Costava/break-bricks/releases/tag/assets). Extract the `assets` folder to the top level of the repo.
3. `make init`
4. `make run`

## Controls
- Control the paddle with the mouse
- `f`: Toggle fullscreen
- `r`: Reset the game
- `w`: Double the speed of the ball
- `s`: Cut the speed of the ball in half
- `F12`: Save a screenshot to the `screenshots` folder

## License
The contents of the git repo are available under the GNU General Public License Version 3. See file `LICENSE`.

The assets are available under the license Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).  
You can download the assets [here](https://github.com/Costava/break-bricks/releases/tag/assets)

## Contributing
Not currently accepting contributions.  
Feel free to open an issue.
