# Gimme an X, Gimme an O - Tic Tac Toe

This project implements a digital version of Tic Tac Toe game where players take
turns pressing buttons corresponding to positions on the 9x9 grid. When a
valid move is made, an LED corresponding to the chosen position lights
up and the players switch turns. Different colored LEDs are used to
indicate the different players (Xs are green and Os are blue, using the
convention that X always moves first). If a winner emerges, the winning
sequence of LEDs blinks and the next button press will reset the game; in
the event of a tie, the next button press will perform the reset without
blinking anything.
A heuristic-based AI was implemented so that the game could be played
by only one player. Different programs are loaded for the two game types.
