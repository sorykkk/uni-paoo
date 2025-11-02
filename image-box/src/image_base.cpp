#include "image_base.h"
#include <ctime>

ImageBase::ImageBase(const std::string& name, const std::string& format)
    : _name(name), _format(format), _creationTime(std::time(nullptr)) {
    std::cout << "[Base] Constructor: name=" << _name << ", format=" << _format << "\n";
}

ImageBase::~ImageBase() {
    std::cout << "[Base] Destructor: name=" << _name << "\n";
}

/**
 * This is shared helper function that contains common copy logic.
 * Both copy constructor and copy assignment call this.
 */
void ImageBase::copyFrom(const ImageBase& other) {
    _name = other._name;
    _format = other._format;
    _creationTime = other._creationTime;
    std::cout << "[Base] copyFrom: copied all base members (name, format, creationTime)\n";
}

ImageBase::ImageBase(const ImageBase& other) {
    copyFrom(other);
    std::cout << "[Base] Copy Constructor: completed\n";
}

ImageBase& ImageBase::operator=(const ImageBase& other) {
    if(this != &other) {
        copyFrom(other);
        std::cout << "[Base] Copy Assignment: completed\n";
    }
    return *this;
}

// TODO: explain why here creation time has no move
ImageBase::ImageBase(ImageBase&& other) noexcept
    : _name(std::move(other._name)),
      _format(std::move(other._format)),
      _creationTime(other._creationTime) {

    other._creationTime = 0;
    std::cout << "[Base] Move Constructor: completed\n";
}

ImageBase& ImageBase::operator=(ImageBase&& other) noexcept {
    if(this != &other) {
        _name = std::move(other._name);
        _format = std::move(other._format);
        _creationTime = other._creationTime;
        other._creationTime = 0;
        std::cout << "[Base] Move Assigment: completed\n";
    }
    return *this;
}
