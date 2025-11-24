/**
 * Real-world cases demonstrating:
 * 1. Return reference to *this in assignment operators
 * 2. Handle self-assignment in operator=
 * 3. Copy all parts of an object
 */

#include "image.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "=== 1. RETURN REFERENCE TO *this ===\n\n";
    
    // Case 1: Chained assignment (common pattern)
    std::cout << "Case 1: Chained assignment\n";
    Image img1(50, 50, "img1");
    Image img2(100, 100, "img2");
    Image img3(200, 200, "img3");
    img1 = img2 = img3;  // Only works if operator= returns Image&
    std::cout << "Result: img1 size = " << img1.getWidth() << "x" << img1.getHeight() << "\n\n";
    
    // Case 2: Assignment in conditional
    std::cout << "Case 2: Assignment in conditional\n";
    Image source(300, 300, "source");
    Image dest(100, 100, "dest");
    if ((dest = source).getWidth() > 250) {
        std::cout << "Assignment done and width check passed\n\n";
    }
    
    // Case 3: Assignment in loop with method call
    std::cout << "Case 3: Update and verify in one expression\n";
    std::vector<Image> images;
    images.push_back(Image(50, 50, "item1"));
    Image template_img(100, 100, "template");
    for (auto& img : images) {
        (img = template_img).setCompressionQuality(95);  // Assign and set quality
    }
    std::cout << "Updated quality: " << images[0].getCompressionQuality() << "\n\n";

    std::cout << "=== 2. HANDLE SELF-ASSIGNMENT ===\n\n";
    
    // Case 1: Direct self-assignment
    std::cout << "Case 1: Direct self-assignment\n";
    Image img(150, 150, "test");
    img = img;  // Without check: would delete data then try to copy it (crash!)
    std::cout << "Safe: " << img.getName() << " still valid\n\n";
    
    {
        // Case 2: Self-assignment via reference
        std::cout << "Case 2: Self-assignment via reference\n";
        Image original(200, 200, "original");
        Image& ref = original;
        original = ref;  // Same object, check prevents disaster
        std::cout << "Safe: " << original.getName() << " still valid\n\n";
    }
    
    // Case 3: Array index scenario
    std::cout << "Case 3: Array element assignment with variable index\n";
    std::vector<Image> imageList;
    imageList.push_back(Image(100, 100, "item_0"));
    imageList.push_back(Image(100, 100, "item_1"));
    int i = 1;  // Could be from user input or calculation
    imageList[1] = imageList[i];  // Self-assignment when i happens to be 1
    std::cout << "Safe: no crash despite self-assignment\n\n";

    std::cout << "=== 3. COPY ALL PARTS OF OBJECT ===\n\n";
    
    // Our Image class hierarchy:
    // ImageBase: _name, _format, _creationTime
    // Image: _width, _height, _data, _compressionQuality
    {
        std::cout << "Case 1: Copy constructor must copy ALL parts\n";
        Image original(400, 300, "vacation");
        original.setFormat("jpeg");
        original.setCompressionQuality(95);
        
        Image copy(original);  // Calls ImageBase copy ctor + copyImageData()
        std::cout << "Original: " << original.getName() << ", " << original.getFormat() 
                  << ", quality=" << original.getCompressionQuality() << "\n";
        // if there wasn't initialization for base class members
        // then name, format and creationTime would be garbage values
        std::cout << "Copy: " << copy.getName() << ", " << copy.getFormat() 
                  << ", quality=" << copy.getCompressionQuality() << "\n";
        std::cout << "All parts copied correctly\n\n";
    }
    
    std::cout << "Case 2: Copy assignment must copy ALL parts\n";
    Image src(640, 480, "screenshot");
    src.setFormat("png");
    src.setCompressionQuality(100);
    
    Image tgt(100, 100, "old");
    tgt = src;  // Calls ImageBase::operator= + copyImageData()
    std::cout << "Target now: " << tgt.getName() << ", " << tgt.getFormat() 
              << ", quality=" << tgt.getCompressionQuality() << "\n\n";
    
    std::cout << "Case 3: Deep copy of dynamic memory\n";
    Image img_a(100, 100, "image_a");
    if (img_a.loadFromFile("example/images/input.jpg")) {
        Image img_b = img_a;  // Must allocate NEW buffer, not share pointer
        img_a.flipVertical();
        std::cout << "Modified original, copy remains independent\n";
        img_a.saveToFile("output/demo_flipped.png");
        img_b.saveToFile("output/demo_copy.png");
        std::cout << "Saved two different files - deep copy works!\n";
    }

    std::cout << "\n--- Cleanup ---\n";
    return 0;
}
