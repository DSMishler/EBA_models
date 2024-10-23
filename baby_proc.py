import sys
import pickle

assert len(sys.argv) == 2, f"exactly two args allowed"

pickup_fname = sys.argv[1]
pf = open(pickup_fname, "rb")
pickup_info = pickle.load(pf)
pf.close

dropoff_fname = pickup_info["dropoff"]
dropoff_info = {"message": "RAAAAAAAAHHHH"}

pf = open(dropoff_fname, "wb")
pickle.dump(dropoff_info, pf)
pf.close()
