1. Move as much error checking as possible from the module execution into the module parser.
In the parser, we also want to pinpoint to illformed inputs using the boost spirit x3 error handler positions.
This will help the LLM figure out more precisely where the inputs are syntactically wrong.
2. The difference between internal and external memory is not fully clear from the code.
Bring the code closer to the pseudocode given in Algorithm 2 of the paper bonet-et-al-icaps2024.
To do this we have to partition the memory states into internal and external.
Lets hardcode this in the type via templates, i.e., MemoryState<Internal>, MemoryState<External>.
We then also need a MemoryStateVariant for runtime choices.
3. Create an ExtendedState, which wraps a AnnotatedStateGraphVertexLabel, a MemoryStateVariant, and register contents.
These must become contents of nodes in the ModuleProgramProofGraph according to the paper.
In other words, we are currently missing the register contents, and reuse of the existing AnnotatedStateGraphVertexLabel.
Since register content are stack allocated fixed size arrays, we dont need fancy datastructures to avoid frequent allocations.
4. Simultaneously start writing a latex file that clearly documents the syntax and semantics
of the implementation. You can take inspiration from the paper.
the purpose of this document is precision and compactness.
5. For the two proof graphs, we would like to also store along edges, which rule applied.
Hence we need to do two things: 
- Introduce a SketchProofGraph, where nodes are AnnotatedStateGraphVertexLabel and edges 
are a struct wrapping StateGraphEdgeLabel and RuleView.
- Extend ModuleProgramProofGraph edges in a similar way, i.e.,
have a struct that wraps StateGraphEdgeLabel and RuleVariantView.