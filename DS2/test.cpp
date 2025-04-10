#include <iostream>
#include <string>

int main() {
    std::cout << "Test program - can you see this output?" << std::endl;
    std::string input;
    std::cout << "Type something and press Enter: ";
    std::getline(std::cin, input);
    std::cout << "You typed: " << input << std::endl;
    std::cout << "Press Enter to exit...";
    std::cin.get();
    return 0;
}