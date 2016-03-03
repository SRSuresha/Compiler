Xujia Cao
cs 160 project4
   In this project, I fixed 2 minor errors in my parser and implement the full typechecker. My typechecker correctly catch the type error of the test code in the project 3 and I also include my own test file.
   I used default rule in all of the visit except the primite. I used default rule in addDeclImpl but I preprocess the type of the arguments and the return type so I can add the procedure to the symbol table before the procedure block is being processed. Therefore I support the recursion.
   I also set the node's scope to the current scope but so far I didn't use this variable in my check and visit function.  
