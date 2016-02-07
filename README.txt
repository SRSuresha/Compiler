Name: Xujia Cao

This is my lexer and parser for project2. use command make to generate the files and then type ./csimple <testfilename to run the test. I implement more feature than I suppose to becuase I didn't read the "What Your Parser Has to Do!" section in project webpage until I finsihed the parser. I figure it won't hurt to reject some errors. 

I enforce the parser to have one and only one main. I enforce the expr in [] must be a number expersion. I did not put the EOF in my lexer because it seems like lex and yacc have the special $end symbol to do the same trick. I enforce the precedence used %left and %right and the order to declare them. Dummy is used to enforce the parser to recognize negative number first. 

I passed test.1234 and all valid example in language manual. All terminals are upper case and all productions start with lower case. Some productions follow with a printf for debug purpose but I did not write all printf. 

