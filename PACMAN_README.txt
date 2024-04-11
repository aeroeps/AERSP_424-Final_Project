Pacman: The New Age

This is a C++ program that implements a simplified game of Pacman. 

The program is a single-player, user-interactive game that uses a GUI to 
show a world where the main character, Pacman, travels around the maze trying 
to eat all of the dots before his enemies, the ghosts, catch him. To successfully
win this game, pacman must eat all of the dots in the maze without being touched
by any of the ghosts; in the event he does run into any of the ghosts, the game is
over and the player loses. In the normal version of Pacman, if a larger powerup circle 
is eaten, then Pacman can turn the tables and eat the ghosts, sending them back to their 
home at the center of the maze. However, in our simplified version of the game, there 
are no powerups, Pacman has only one life, and the map of the maze is different than in 
the original game. 

All files necessary to run the game are included. To play, the user needs only to download the 
provided .zip files to a known location in their drive, open the .sln in Microsoft Visual 
Studio, and run the program. When prompted to begin the game, the user should press 
play and use the keyboard keys "w', "a", "s", and "d" to move the yellow Pacman 
character up, left, down, and right, respectively. If the user successfully moves their 
Pacman to consume all of the dots in the maze without running into any ghosts, then the 
program will present them with a victory screen to let them know they have completed the 
game. If the user lets their Pacman be caught by any ghosts at any point during the game, 
the game stops, and the program will let the user know that they have lost. To play again, 
simply rerun the program from inside Visual Studio.

If the user wishes to modify any part of the game, they are able to go into the respective files
and update the number of characters (Pacmen, ghosts, etc.) present in the maze, change the grid
layout, add extra lives by counting the number of times that Pacman gets caught by ghosts, etc. 

