#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <algorithm>

Image::Image(int width, int height, const std::string& name) 
    : ImageBase(name, "raw"), _width(width), _height(height) {
    size_t size = _width * _height * 3; // RGB
    _data = new uint8_t[size];
    std::memset(_data, 0, size);
    std::cout << "[Image] Constructor: allocated " << size << " bytes\n";
}

void Image::copyImageData(const Image& other) {
    _width = other._width;
    _height = other._height;
    _compressionQuality = other._compressionQuality;

    // Deep copy the pixel data
    size_t size = _width * _height * 3;
    _data = new uint8_t[size];
    std::memcpy(_data, other._data, size);

    std::cout << "[Image] copyImageData: copied all members and " << size << " bytes of data\n";
}

void Image::cleanup() {
    delete[] _data;
    _data = nullptr;
    _width = 0;
    _height = 0;
}

Image::Image(Image&& other) noexcept 
    : ImageBase(std::move(other)),
      _width(other._width), 
      _height(other._height), 
      _data(other._data),
      _compressionQuality(other._compressionQuality) {
    other._width = 0;
    other._height = 0;
    other._data = nullptr;
    other._compressionQuality = 90;

    std::cout << "[Image] Move Constructor: transferred ownership of image data\n";
}

Image::~Image() {
    cleanup();
    std::cout << "[Image] Destructor: deallocated image data\n";
}

Image::Image(const Image& other)
    : ImageBase(other) {
    copyImageData(other);
    std::cout << "[Image] Copy Constructor: completed\n";
}

Image& Image::operator=(const Image& other) {
    if (this != &other) {   // handle assing to self
        // Copy base class parts
        ImageBase::operator=(other);
        // clean up old data
        cleanup();
        // copy derived class parts using helper
        copyImageData(other);

        std::cout << "[Image] Copy Assignment: completed\n";
    }
    return *this;   // always return *this for chain assignment
}

// noexcept qualifier here is more of the good practice i got from the work env
// Generally it means that we guarantee the compiler that this code won't throw 
// or propagate any exceptions (eliminating the need for it to keep the tracestack)
// this allows speed optimization
// half of stl containers use move semantics if the assign operator is noexcept
// but if there was no noexcept then it would use copy assing which is more expensive
// example:
//  v.push_back(Image(100, 100)); -> will use move assign if noexcept
//                                -> will use copy assign if noexcept is not present
Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        ImageBase::operator=(std::move(other));
        cleanup();
        _width = other._width;
        _height = other._height;
        _data = other._data;
        _compressionQuality = other._compressionQuality;

        other._width = 0;
        other._height = 0;
        other._data = nullptr;
        other._compressionQuality = 90;

        std::cout << "[Image] Move Assignment: transferred ownership of image data\n";
    }
    return *this;
}

bool Image::loadFromFile(const std::string& path) {
    int width, height, channels;
    uint8_t* img = stbi_load(path.c_str(), &width, &height, &channels, 3); 

    if (!img) {
        std::cerr << "Error loading image from file: " << path << "\n";
        std::cerr << "STB Error: " << stbi_failure_reason() << "\n";
        return false;
    }

    _width = width;
    _height = height;

    delete[] _data;
    size_t size = _width * _height * 3;
    _data = new uint8_t[size];

    std::memcpy(_data, img, size);
    stbi_image_free(img);

    std::string ext = path.substr(path.find_last_of(".") + 1);
    setFormat(ext);

    std::cout << "Loaded image from: " << path << " (" << _width << "x" << _height << ")\n";
    return true;
}

void Image::flipHorizontal() {
    for (int y = 0; y < _height; y++) {
        for (int x = 0; x < _width / 2; x++) {
            int leftIdx = (y * _width + x) * 3;
            int rightIdx = (y * _width + (_width - 1 - x)) * 3;

            // Swap RGB values
            std::swap(_data[leftIdx], _data[rightIdx]);         // R
            std::swap(_data[leftIdx + 1], _data[rightIdx + 1]); // G
            std::swap(_data[leftIdx + 2], _data[rightIdx + 2]); // B
        }
    }
    std::cout << "Flipped image horizontally\n";
}

void Image::flipVertical() {
    for (int y = 0; y < _height / 2; y++) {
        for (int x = 0; x < _width; x++) {
            int topIdx = (y * _width + x) * 3;
            int bottomIdx = ((_height - 1 - y) * _width + x) * 3;

            // Swap RGB values
            std::swap(_data[topIdx], _data[bottomIdx]);         // R
            std::swap(_data[topIdx + 1], _data[bottomIdx + 1]); // G
            std::swap(_data[topIdx + 2], _data[bottomIdx + 2]); // B
        }
    }
    std::cout << "Flipped image vertically\n";
}

void Image::toGrayscale() {
    for (int i = 0; i < _width * _height; i++) {
        int idx = i * 3;
        uint8_t r = _data[idx];
        uint8_t g = _data[idx + 1];
        uint8_t b = _data[idx + 2];

        // Standard luminosity formula
        uint8_t gray = static_cast<uint8_t>(0.299 * r + 0.587 * g + 0.114 * b);

        _data[idx] = gray;
        _data[idx + 1] = gray;
        _data[idx + 2] = gray;
    }
    std::cout << "Converted image to grayscale\n";
}

bool Image::saveToFile(const std::string& path) const {
    if (!_data || _width == 0 || _height == 0) {
        std::cerr << "Error: cannot save empty image\n";
        return false;
    }

    // Get format from file extension
    std::string ext = path.substr(path.find_last_of(".") + 1);
    int result = 0;
    
    if (ext == "png" || ext == "PNG") {
        result = stbi_write_png(path.c_str(), _width, _height, 3, _data, _width * 3);
    } else if (ext == "jpg" || ext == "JPG" || ext == "jpeg" || ext == "JPEG") {
        result = stbi_write_jpg(path.c_str(), _width, _height, 3, _data, 90);
    } else if (ext == "bmp" || ext == "BMP") {
        result = stbi_write_bmp(path.c_str(), _width, _height, 3, _data);
    } else {
        std::cerr << "Unsupported image format: " << ext << "\n";
        return false;
    }

    if (!result) {
        std::cerr << "Error saving image to file: " << path << "\n";
        return false;
    }

    std::cout << "Saved image to: " << path << "\n";
    return true;
}
