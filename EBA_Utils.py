import random


alphabet = ["a", "b", "c", "d", "e", "f"]

def random_name(length=20):
    return "".join([random.choice(alphabet) for i in range(length)])
