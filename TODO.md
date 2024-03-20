# Debug tools
- In-built disassembly of current instruction/op code
- Easy view of CPU function queue and timing

# Architecture modifications
- Formalize CPU function queue to be a little more encapsulated
- CPU implicit instruction fetches are concerning for traceability

# Design Notes
- Attempted a CPU function queue design for cycle timing but decided it was too opaque and difficult to track
  - Most CPU behaviors on an individual cycle depend highly on other cycles behaving exactly a certain way
  - Hard to establish common interfaces or "contracts" functions can rely on to guarantee correctness
  - Is also just messy having a lot of functions built off of common util functions
  - Since I'm still unsure how parts of the CPU are timed (e.g., does memory access happen before or after the PC increment?) it's difficult to provide a unified way for the CPU to call functions that won't require overhauling in the future when implementing weirder opcodes
  - For example: every cycle a CPU must make exactly 1 memory access every cycle. If we're making each opcode a function, and using functions to avoid copying code for common behaviors different opcodes have, then how do we guarantee that all functions combined make exactly 1 memory access?
- Opting for resumable function design using simple macro