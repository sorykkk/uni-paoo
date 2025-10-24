#include "image.h"
#include <iostream>

int main() {
    Image img;
    if(!img.loadFromFile("input.png")) {
        std::cerr << "Failed to load image\n";
        return 1;
    }

    img.toGrayscale();
    img.flipHorizontal();

    if(!img.saveToFile("output.png")) {
        std::cerr << "Failed to save image\n";
        return 1;
    }

    std::cout << "Image processed successfully!\n";

    return 0;
}