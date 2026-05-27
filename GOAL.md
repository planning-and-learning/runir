# List of Requirements:
- Do not recompile after every single code change.
- Use 2 cores for compilation.

# List of Goals:

1. Read and continue keeping track of recent changes in LOG.md
2. Move as much error checking as possible from the module execution into the module parser.
In the parser, we also want to pinpoint to illformed inputs using the boost spirit x3 error handler positions.
This will help the LLM figure out more precisely where the inputs are syntactically wrong.
3. bring the module program find_solution and prove_solution inline with the paper bonet-et-al-icaps2024.pdf.
- at the high level there should be the distinction between internal and external memory states.
- maybe partitioning them is necessary in advance
- we have a large execution.hpp but also code in detail. move the whole execution into details and keep only the paper high levle loop in the impl.