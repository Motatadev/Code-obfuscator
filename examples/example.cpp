#include <iostream>
#include <string>

// Ceci est un commentaire qui sera supprime
int calculateSum(int a, int b) {
    int result = a + b;
    std::string message = "Somme calculee";
    std::cout << message << ": " << result << std::endl;
    return result;
}

int main() {
    int x = 42;
    int y = 100;
    int sum = calculateSum(x, y);
    std::string greeting = "Hello World";
    std::cout << greeting << std::endl;
    return 0;
}
