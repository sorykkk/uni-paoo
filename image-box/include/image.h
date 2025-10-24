#pragma once
#include <cstdint>
#include <iostream>
#include <cstring>

class Image {
private:
    int _width{0};
    int _height{0};
    uint8_t* _data{nullptr}; 

public:
    Image(int width, int height);
    ~Image();

    Image(const Image& other);
    Image(Image&& other) noexcept;

    Image& operator=(const Image& other);
    Image& operator=(Image&& other) noexcept;

    uint32_t getWidth() const;
    uint32_t getHeight() const;

    void flipHorizontal();
    void flipVertical();
    void toGrayscale();

    bool loadFromFile(const std::string& filepath);
    bool saveToFile(const std::string& path) const;
};