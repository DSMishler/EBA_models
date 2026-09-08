# EBA Models

## Daniel Mishler


A collection of Exposed Buffer Architecture (EBA) models that are
made to demonstrate the effectiveness of EBA and as educational tools



The most recent version of EBA can be found in the `EBA_IR` folder



## To run on the UTK Hydra machines


- ssh into hydra
   - `ssh <you>@hydra<favoritenumber>.eecs.utk.edu`
- find a good directory to
   - `git clone https://github.com/dsmishler/EBA_models.git`
- `cd EBA_v0`
- `cmake -B build`
- `cd build`
- `make`
- `./eba`
- when prompted, type `circ_buf_demo.so`

You should see a message announcing the testing of `par_sched_circ_buf`,
following quickly by a message saying "Success!"
If you see this, the demo worked!


### The Streaming Demo

The streaming demo requires X11 forwarding, which has historically been
tenuous in how well it works. Getting this demo to work is optional,
but it is a fun demo.

- ssh into hydra, this time making sure X11 forwarding is enabled
   - `ssh -X <you>@hydra<favoritenumber>.eecs.utk.edu`
   - You may need to do some looking into your terminal to make sure X11
     forwarding works. Alternatively, you can run this code on your local
     machine.
- `./eba`
- when prompted, type `stream_demo.so`

If all is working well, a window should pop up with five triangles moving
across from left to right with different trajectories and velocities.

Because X11 is doing forwarding on the hydra machines,
graphics will be *very* slow. Try it on your own machine for
faster results.
