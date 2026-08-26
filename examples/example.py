# Exemple Python
def calculate_sum(a, b):
    result = a + b
    message = "Somme calculee"
    print(message, result)
    return result

def main():
    x = 42
    y = 100
    secret = "Hello Python"
    total = calculate_sum(x, y)
    print(secret)
    for i in range(5):
        print(f"Iteration {i}")

if __name__ == "__main__":
    main()
