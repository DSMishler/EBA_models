import sys
import pickle


assert_correct_args(sys.argv)
pickup_info = get_pickup_info(sys.argv)


dropoff_fname = pickup_info["dropoff"]
dropoff_info = {"message": "RAAAAAAAAHHHH"}

prep_dropoff_file(dropoff_info, dropoff_fname)
