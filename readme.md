# Chess Assignment
CMPM 123 - Aegis Michael

## FEN Strings

Setting up the board with FEN strings wasn't so bad. I iterated through the string one char at a time, adding logic to handle all of the different possible chars and keeping a counter to represent the current index on the chessboard. First up was the '/' char, in which case I skip and go to the next one. In the case that there's a number, I add that much to the counter and then skip. If it's a letter, I add one to the counter, and create a piece at that index, using the letter and its case to determine who's piece it is and what type.

I had a little hiccup after implementing all of that, where black was at the bottom and white at the top, despite using the correct starter FEN string and handling cases correctly. I peeped in on the class Discord, and saw Divine's message that if you use initializeChessSquares instead of initializeSquares in the setup function, it creates the board upside-down. I'm really used to having the top left be the origin, and I knew it would be cause for plenty more problems in the future, so I decided to switch it up save myself some future pain.

## Pawn, Knight, & King Movement

This assignment was really hard, I ended up looking back and forth through the codebase and the lecture demoing knight movement a lot to get it done. I tried using bitboards initially, but I felt like I was making no progress and eventually switched to using an iterative approach instead, which is much more intuitive to me.

To set things up, I first defined some methods for generating moves: generateAllMoves & generatePawnMoves.

Using the example from the lecture, I iterated through every position in the state string. If it's a pawn, I call generatePawnMoves on it to find all the possible spaces it can go. That looks like deciding which direction it's moving based on the current player, then checking the space in front of it to see if its empty. If it is, add that space to the list of valid moves. If the pawn is at the starting row, you can also check the space after that, and if it's empty then add that too. Finally, for capturing, I check the two spaces to the left and right of the pawn, one column advanced from its position. If they're inside the chessboard and occupied by an enemy piece, add that to the list of possible moves.

Once I got that logic defined, I had to fill our the canBitMoveFrom and canBitMoveFromTo functions so that the pieces could actually be moved. That looked like polling the requesting bit to see if it's in the move list and highlighting all of the available moves while I do so for the former, and checking if both the requesting bit and the requested destination are both inside a single move in the _moves vector for the latter.

After I had the pawns moving and capturing other pieces, the knight movement and king movement were MUCH easier to implement--they took me about 10 and 5 minutes, respectively. They have similar logic to the pawn; the knight runs through an array of all the available row and column offsets from its current position, checks of they're inside the chessboard, and adds them if they're unoccupied or occupied by an enemy, and the king does the same but for its smaller offset using a nested for loop.

After all of that was done, I revisited my generateAllMoves function to make sure it was correctly finding 20 possible moves from the starting position. After it printed 40, I knew I was accidentally checking for both players' possible moves, instead of just the current player. I imagine that would cause some optimization problems down the road, so I patched that out and now it correctly prints out 20.

### Breakpoint issues:
Lastly, I wasn't able to use VSC breakpoints for whatever reason, so I recorded a quick video of my code working in-action, I hope that's ok.
https://youtu.be/v7stTm7twF04
