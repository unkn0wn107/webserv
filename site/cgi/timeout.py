import time

timeout = 9

start_time = time.time()
end_time = start_time + timeout

while time.time() < end_time:
    time.sleep(1)  # CPU friendly

print("Content-Type: text/plain\r\n\r\n")
print("Script completed after {} seconds.".format(timeout))
