
- offset inside a class object
- runtime support with offsets -> source file info
- RUNTIME Support mostly remaining for us --> will need to discuss 
- Let us try list of user-defined classes 
- variable declared in parent class can be used in child class 
- Implicit coercion is allowed. Example, var : float = 2 
- apart from main and __init__ --> return type will be mentioned 
    "NONE" type in case of void return 
- All attributes will be declared with types in the class body or
    the __init__() functions. public3.py from Milestone 1 uses the latter.
- value of a global variable can be different for a function scope
- Class inheritance: You can treat all attributes and methods as public.
- return of a function can be a class object as well
- discussions: = https://piazza.com/class/lqrto1fry5u57e/post/212
- discussions: = https://piazza.com/class/lqrto1fry5u57e/post/219
- You can ignore keyword arguments while invoking functions.
- Inbuilt function to tackle: print() , range() , len()
- (225) offset for string: You can put the string as a constant
    in the data area and use a pointer type for the string variable
    or replace all uses of the string variable with the fixed address.
- tackle  = "some string" cases for 3AC generation 
- support range(2,10) in the for loop 
- (236) overriding of derived attributes in the class field declaration
- support for len : input will only be a list type 
- "In fact, our project aims to mimick C-like behavior for a Python-subset."
- (251) Lists will always be passed through variables.
- (250) See follow up discussion {Couldn't understand it properly}
- No need to support relational operator for list or class objects
- (261) Nothing special here but can have a look at it 
- indexing of string -> no need to support it 
- 

