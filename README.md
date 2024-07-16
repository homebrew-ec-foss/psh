# psh: A Fancy POSIX-like Shell

This project is a custom implementation of a shell written in C. It aims to achieve all the features of a POSIX-compliant shell. It is part of the TILDE 3.0 mentored program where 5 students worked on this project under the mentorship of 3 mentors.

## Contributors:
- Aditya R
- Krishna Kumar
- Alayna Monteiro
- Sumithra Suresh
- Siddhartha Rao

## Mentors:
- Nathan Paul
- Tejas R
- Anupam G

## Progress by End of Week 2:
Handing off from the progress we made in week1 where we succesfully began the implementations for few built in functions  
such as cd and history, we began adding extensive support for all possible flags for these functions. We now have almost  
all posix like required flags for functions such as `cd`,`exit`,`echo`,`pwd`,`export`,`fc`. While trying to implement   
the history function, we learned that history is not POSIX compliant. In order to make sure that psh is POSIX like  
compliant, we changed it to `fc`. We added extensive support for symlinks for better ease of directory movements.

## Current Workflow:
1. **Read Input**: Read an input from the user.
2. **Tokenize**: Tokenize the input with " " as a delimiter.
3. **Command Execution**:
   - Check whether the first token is a built-in or external command.
   - Call the corresponding function and return the result.
4. **Loop**: Go back to reading an input infinitely until `exit` is called.


## Future Work:
- Finish all built in functions  
- Arrow keys  
- Handling signals  
- Shell expansion  
- Bug Fixes  
- Read and parse .sh files  

### How to Build and Run:
```bash
cd psh
make run
<<<<<<< HEAD
./run
=======
>>>>>>> e32d55693f1ad0dd856c5091dc3023da7c146333
