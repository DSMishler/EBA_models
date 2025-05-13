import threading

TL = threading.Lock()


with TL:
    print("hello1")
    
with TL:
    print("hello2")

i = 0
while True:
    with TL:
        print(i)
        i += 1
        if i == 10:
            break

with TL:
    print("hello2")
