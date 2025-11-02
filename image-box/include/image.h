#pragma once
#include "image_base.h"
#include <cstdint>
#include <iostream>
#include <cstring>

/**
 * Image class inherits from ImageBase to demonstrate:
 *  * Copying all data members (both derived and base class parts)
 *  * Using a helper function for common copy logic
 *  * not implementing copy functions in terms of each other
 */
class Image : public ImageBase {
private:
    int _width{0};
    int _height{0};
    uint8_t* _data{nullptr}; 
    int _compressionQuality{90}; 

    // third function that both copy constructor and copy assigment call
    void copyImageData(const Image& other);

    void cleanup();

public:
    /**
     * Project 1 & 2 requirements:
     * Note: 
     *  I thought that we should implement rule of 5
     *  that's why i already did copy assign and move assign
     */

    // Non-empty constructor & destructor
    Image(int width, int height, const std::string& name = "image");
    ~Image();

    // Copy constructor & move constructor
    Image(const Image& other);
    Image(Image&& other) noexcept;

    // Copy assign & move assign operators
    Image& operator=(const Image& other);
    Image& operator=(Image&& other) noexcept;

    // getters
    uint32_t getWidth() const { return _width; }
    uint32_t getHeight() const { return _height; }
    int getCompressionQuality() const { return _compressionQuality; }

    // setters
    void setCompressionQuality(int quality) { _compressionQuality = quality; }

    // Image manipulation functions (just for demonstration)
    void flipHorizontal();
    void flipVertical();
    void toGrayscale();

    // File operations
    // function wrappers 
    bool loadFromFile(const std::string& filepath);
    bool saveToFile(const std::string& path) const;
};