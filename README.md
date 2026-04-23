# NICO

NICO is a toolkit for building, training, and evaluating recurrent time delay neural networks (RTDNNs). The original documentation describes it as a command-line toolbox: you define network topology and stream configuration with shell commands, train networks with backpropagation through time, and run or evaluate trained models with separate analysis tools.

This codebase is primarily of historical interest. It originated in the 1990s, and the design, tooling, and documentation reflect that period.

At a high level, the toolkit is organized into three groups of tools:

- Building tools for creating network objects, groups, units, streams, and connections.
- Training tools for normalizing data, running backpropagation, pruning, and related adjustments.
- Evaluation tools for exciting trained networks and measuring classification results.

Typical usage is script-driven. A network is created by assembling groups and units, connecting them, attaching input and output streams, and then setting stream properties such as data format, directory, and filename extension. The `doc/intro.html` manual page walks through this workflow with a small three-layer example network.

The repository also includes a library side intended for developers. The `RTSim` module lets a C or C++ program load a trained NICO network and run it in a streaming fashion, pushing input vectors and pulling output vectors as they become available instead of processing whole files at once.

For the original manual and command reference, start in `doc/index.html`. The most useful overview pages are:

- `doc/intro.html` for the main introduction and tool categories.
- `doc/rtsim.html` for embedding trained networks in C/C++ applications.
- `doc/COMMANDS/` for individual command documentation.
