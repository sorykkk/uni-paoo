#include "image.h"
#include <iostream>
#include <vector>
#include <filesystem>  // C++17

// Factory function that returns by value, triggers move constructor
Image createImage(int width, int height, const std::string& name) {
    std::cout << "    [Factory function creating image]\n";
    Image img{width, height, name};
    return img; // move constructor
}

// Load and process function, returns by value
Image LoadAndProcess(const std::string& filepath) {
    std::cout << "    [LoadAndProcess function loading image]\n";
    Image img(0, 0, "processed");
    if (img.loadFromFile(filepath)) {
        img.flipHorizontal();
    }
    return img; // move constructor
}

int main() {
    std::cout << "=== 1. Factory Function ===\n";
    Image img1 = createImage(100, 100, "factory_image");
    std::cout << "Image created with name: " << img1.getName() << "\n\n";

    std::cout << "=== 2. Function return ===\n";
    Image img2 = LoadAndProcess("example/images/input.jpg");
    std::cout << "Image processed with name: " << img2.getName() << "\n\n";

    std::cout << "=== 3. Vector push_back ===\n";
    std::vector<Image> images;
    std::cout << "    [Pushing temporary image to vector]\n";
    images.push_back(Image{50, 50, "temp_image"}); // move constructor
    std::cout << "Vector now has " << images.size() << " image(s)\n\n";

    std::cout << "=== 4. Reassignment with Temporary ===\n";
    Image img3{10, 10, "original"};
    std::cout << "  [Assigning new temporary image]\n";
    img3 = Image{200, 200, "replacement"};  // Move assignment called
    std::cout << "Now has name: " << img3.getName() << "\n\n";

    std::cout << "=== 5. Reassignment from Function ===\n";
    img3 = createImage(300, 300, "another_factory");  // Move assignment called
    std::cout << "Now has name: " << img3.getName() << "\n\n";

    std::cout << "=== 6. Copy Constructor ===\n";
    Image img4{100, 100, "to_be_copied"};
    Image img5{img4};  // Copy constructor (lvalue)
    std::cout << "Copy has name: " << img5.getName() << "\n\n";

    std::cout << "=== 7. Copy Assignment ===\n";
    Image img6{50, 50, "target"};
    img6 = img4;  // Copy assignment (lvalue)
    std::cout << "Copy has name: " << img6.getName() << "\n\n";

    std::cout << "=== 8. Testing with actual file ===\n";
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