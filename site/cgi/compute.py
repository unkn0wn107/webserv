import os
import sys
import time

n = 200000
num_iterations = 10

sys.set_int_max_str_digits(n)

print("Content-Type: text/html\r\n\r\n")

print("<html>")
print("<head>")
print("<title>Computation - CGI Python Script</title>")
print("</head>")
print("<body>")
print("<h2>Computation Intensive Task</h2>")

print("<h3>Environment Variables:</h3>")
print("<pre>")
for var_name, var_value in os.environ.items():
    print(f"{var_name} = {var_value}")
print("</pre>")

# Computation intensive task: Calculate the nth Fibonacci number
def fibonacci(n):
    if n <= 0:
        return 0
    elif n == 1:
        return 1
    else:
        a, b = 0, 1
        for _ in range(2, n + 1):
            a, b = b, a + b
        return b

start_time = time.time()
for _ in range(num_iterations):
    fib_n = fibonacci(n)
end_time = time.time()

print(f"<h3>Fibonacci({n}) = {fib_n}</h3>")
print(f"<h3>Computation Time: {end_time - start_time} seconds</h3>")

print("</body>")
print("</html>")
