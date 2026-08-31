# EBA Models

## Daniel Mishler


A collection of Exposed Buffer Architecture (EBA) models that are
made to demonstrate the effectiveness of EBA and as educational tools



The most recent version of EBA can be found in the `EBA_IR` folder



## To run on the UTK Hydra machines


- ssh into hydra, making sure X11 forwarding is enabled
   - `ssh -X <you>@hydra<favoritenumber>.eecs.utk.edu`
- find a good directory to
   - `git clone https://github.com/dsmishler/EBA\_models.git`
- `cmake -B build`
- `cd build`
- `make`
- `./eba`
- when prompted, type `stream_demo.so`


Because X11 is doing forwarding on the hydra machines,
graphics will be *very* slow. Try it on your own machine for
faster results.
