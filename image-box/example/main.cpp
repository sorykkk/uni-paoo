#include "image.h"
#include <iostream>
#include <filesystem>  // C++17

int main() {
    // Create an image and load from file
    Image img(0, 0);
    if (!img.loadFromFile("example/images/input.jpg")) {
        return 1;
    }
    
    std::cout << "Image size: " << img.getWidth() << "x" << img.getHeight() << "\n\n";
    
    // Demonstrate copy constructor
    Image imgCopy = img;
    imgCopy.flipHorizontal();
    imgCopy.saveToFile("output/flipped_horizontal.png");
    
    std::cout << "\n";
    
    // Demonstrate move constructor
    Image imgMoved = std::move(imgCopy);
    imgMoved.flipVertical();
    imgMoved.saveToFile("output/flipped_both.png");
    
    std::cout << "\n";
    
    // Demonstrate grayscale
    img.toGrayscale();
    img.saveToFile("output/grayscale.png");
    
    std::cout << "\n";
    
    return 0;
}