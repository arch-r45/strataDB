import time
start = time.time()
for i in range(1000000000):
    continue

end = time.time()

time_taken = end - start
print(f"Time taken: {time_taken} seconds")



