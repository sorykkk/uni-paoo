#include "image.h"
#include <iostream>
#include <filesystem>  // C++17

int main() {
    // Create an image and load from file
    std::cout << "--- Creating original image ---\n";
    Image img1{100, 100};
    img1.setCompressionQuality(85);
    std::cout << "\n";

    std::cout << "--- Testing Copy Constructor ---\n";
    Image img2{img1};
    std::cout << "Copy has name: " << img2.getName() << ", quality: " << img2.getCompressionQuality() << "\n\n";
    
    std::cout << "--- Testing Copy Assignment ---\n";
    Image img3{50, 50, "temp"};
    img3 = img1;
    std::cout << "Copy has name: " << img3.getName() << ", quality: " << img3.getCompressionQuality() << "\n\n";
    
    std::cout << "--- Testing Move Constructor ---\n";
    Image img4{std::move(img2)};
    std::cout << "Moved image has name: " << img4.getName() << "\n\n";

    std::cout << "-- Testing Move Assigment ---\n";
    Image img5{10, 10, "to_be_replaced"};
    img5 = std::move(img3);
    std::cout << "Moved image has name: " << img5.getName() << "\n\n";
    
    std::cout << "--- Testing with actual file ---\n";
    Image fileImg{0, 0, "loaded_image"};
    if(!fileImg.loadFromFile("example/images/input.jpg")) {
        return 1;
    }

    std::cout << "Image has name: " << fileImg.getName()
            << ", format: " << fileImg.getFormat()
            << ", size: " << fileImg.getWidth() << "x" << fileImg.getHeight() << "\n";
    
    // Copy and check all members are copied
    Image fileCopy{fileImg};
    std::cout << "Copy has name: " << fileCopy.getName() 
            << ", format: " << fileCopy.getFormat() 
            << ", size: " << fileCopy.getWidth() << "x" << fileCopy.getHeight() << "\n";

    fileCopy.flipHorizontal();
    fileCopy.saveToFile("output/flipped_horizontal.png");
    
    std::cout << "\n";
    
    // Demonstrate move constructor
    Image imgMoved{std::move(fileCopy)};
    imgMoved.flipVertical();
    imgMoved.saveToFile("output/flipped_both.png");
    
    std::cout << "\n";
    
    // Demonstrate grayscale
    fileImg.toGrayscale();
    fileImg.saveToFile("output/grayscale.png");
    
    std::cout << "\n";

    std::cout << "--- Destructors call ---\n";
    // data for moved images should be empty

    return 0;
}