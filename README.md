# CMSC621 Project - 2

1. Directory structure
Top directory: Clocks

Clocks/test: Test cases
Clocks/bin : Binary executable
Clocks/nodes and Clocks/utils: Contains the source code

2. Compile the code:
>cd Clocks
>make

Make command will compile the code and generate the binary in bin directory.

3. Command:
>./node
usage ./node <id> <nodelist> <d> <clock> <causality> <num_msg> <filename>
                         1. id -> Node identifier
                         2. nodelist -> A file containing a list of all processes
                         3. d -> Whether a node is time-daemon
                         4. clock -> initial clock
                         5. causality -> 0/1 Messages to be ordered?
                         6. number of msg -> number of multicasts
                         7. filename -> Accesses to this file are serialized

4. Running the test cases:
>cd test/
4.1 Without Causal Ordering
  >./nocausal_order.sh
Logs are available under “nocausalproc<id>” file

4.2 With Causal Ordering
  >./causal_order.sh
Logs are available under “causalproc<id>” file
