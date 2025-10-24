#include "image.h"
#include "SFML/Graphics.hpp"
#include <iostream>
#include <algorithm>

Image::Image(int width, int height) 
    : _width(width), _height(height) {
    size_t size = _width * _height * 3; // RGB
    _data = new uint8_t[size];
    std::memset(_data, 0, size);
    std::cout << "Constructor: allocated " << size << " bytes\n";
}

Image::Image(Image&& other) noexcept 
    : _width(other._width), _height(other._height), _data(other._data) {
    other._width = 0;
    other._height = 0;
    other._data = nullptr;
    std::cout << "Move Constructor: transferred ownership of image data\n";
}

Image::~Image() {
    delete[] _data;
    std::cout << "Destructor: deallocated image data\n";
}

Image::Image(const Image& other)
    : _width(other._width), _height(other._height) {
    size_t size = _width * _height * 3;
    _data = new uint8_t[size];
    std::memcpy(_data, other._data, size);
    std::cout << "Copy Constructor: allocated and copied " << size << " bytes\n";
}

Image& Image::operator=(const Image& other) {
    if (this != &other) {
        delete[] _data;

        _width = other._width;
        _height = other._height;
        size_t size = _width * _height * 3;
        _data = new uint8_t[size];
        std::memcpy(_data, other._data, size);
        std::cout << "Copy Assignment: allocated and copied " << size << " bytes\n";
    }
    return *this;
}

Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        delete[] _data;

        _width = other._width;
        _height = other._height;
        _data = other._data;

        other._width = 0;
        other._height = 0;
        other._data = nullptr;

        std::cout << "Move Assignment: transferred ownership of image data\n";
    }
    return *this;
}

Image::uint32_t Image::getWidth() const {
    return _width;
}

Image::uint32_t Image::getHeight() const {
    return _height;
}

Image Image::loadFromFile(const std::string& path) {
    sf::Image img;
    if(!img.loadFromFile(path)) {
        std::cerr << "Error loading image from file: " << path << "\n";
        return Image(0, 0);
    }

    Image image(img.getSize().x, img.getSize().y);
    const sf::Uint8* pixels = img.getPixelsPtr();
    std::memcpy(image._data, pixels, image._width * image._height * 4); // RGBA to RGB
    return image;
}