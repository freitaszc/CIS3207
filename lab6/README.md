# Socket Programming and Shell Integration

### AI Tool used: ChatGPT

## 1. 
### Overview
This first step was very simple as I got so tested out the ping-pong from an existing code and I understood the complexity of the files well. I renamed the files lab6c and lab6 and they communicated well as I ran lab6 first and then lab6c at another terminal. This interaction can be seen on pingpong.png
### Learning questions
How do a client and server communicate using sockets?
What is the difference between bind, listen, accept, connect, send, and recv?
### Debugging questions
Why did my lab6c failed as I started it first?

## 2. 
### Overview
I opened tsh in a terminal at port 2110 and tshtest at another terminal at the same port. My first action was using PUT to create a tuple fruits with priority 4, length 5 and tuple name food. The READ was used to access all the information and worked perfectly and the GET was used to read and extract the information. After using GET on fruits, I lost acess to it and could not see it anymore. This flow can be seen at tsh_put.png, tsh_get.png and then tsh_get2.png, where the tuple could not be found after using GET for the first time.
### Learning questions
What is a tuple and what does it represent?
What do the priority, length means?

## 3. 
### Overview
In order to fulfill the requirements I added OP_Shell to tsh.h, tsh.c and synergy.h to use the new service. I added the new operation code and the necessary structure so the server could receive a shell command, execute and return the output. I also updated the TSH dispatch table so OpShell() became part of the normal request flow, just like the other tuple-space operations. The code added can be analyzed in op_shell.png.
### Learning questions
What additional structs are needed in synergy.h so the shell operation can send and receive data correctly?
How does the server-side OpShell() function fit into the same request/response model as the other tuple-space operations?
### Debugging questions
How can I confirm that OpShell() is actually connected to the dispatch table and not just declared in the header?
### Submission questions
Is adding TSH_OP_SHELL to synergy.h, tsh.h, and tsh.c enough to satisfy the shell service requirement?

## 4. 
### Overview
Then, I modified tshtest.c and tshtest.h to include the new Op_Shell. I added the new prototype, updated the dispatch table and added the shew option into the menu. I tested this new feature with the command pwd and the working flow is in op_shell_command.png.
### Learning questions
How should the client-side shell test send a shell command to TSH?
What is the role of the tshtest menu in testing a new service like OP_SHELL?
### Debugging questions
Why did adding option 5 to the menu not work until I also updated the function dispatch table?
Why did the shell test need explicit null termination for the received output buffer?
### Submission questions
Does adding a Shell menu option and testing a command like echo hello count as a valid unit test for OP_SHELL?

## 5. 
### Overview
Next, I converted the shell-related parts of tshtest.c|h into a new reusable library called tshlib.c|h by removing the interacive menu and prompt behavior. I kept only the core of the non-interactive logic needed to connect to TSH, send the Op_Shell request and receive the output. This removed the shell feature dependence on the interactive menu.
### Learning questions
How do I turn the shell client code into a non-interactive function that can be reused by other programs?
Which parts of tshtest.c are interactive and which parts should be moved into a reusable library?
### Debugging questions
Why did including tshtest.h inside tshlib.h create issues with shared globals and multiple definitions?
### Submission questions
Does converting only the working shell path from tshtest.c into tshlib.c|h satisfy the library requirement?

## 6. 
### Overview
Finally, the launch.c command was created to implement the command-line Op_Shell. launch works as a thin client that accepts a port number and a command from the command line, sends that command to TSH through tshlib, and prints the returned output. I changed the makefile so the new launch client build correctly with the rest of the files. This final implementation can be seen in the screenshot launch.png. I initially faced an error when trying to run launch without the port and command. Then I ran it in the expected format but I received the OP_SHELL failed because the server was not running in another terminal. After realizing these mistakes, I ran the correct format with the commands "ls" and "echo hello". Both commands worked successfully.
### Learning questions
What should a command-line OP_SHELL client do differently from the old interactive shell I wrote in lab4?
How should command-line arguments be combined into one shell command string before sending to TSH?
### Debugging questions
Why did launch fail at first even after compiling successfully?
Why did the Makefile need to be updated and what header definition conflicts had to be resolved for launch.c to build correctly alongside the other files?