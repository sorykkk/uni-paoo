#pragma once
#include <string>
#include <iostream>

/**
 * Base class for Image to demonstrate proper ocpying of base class parts.
 * This class manages about an image (name, format, creation time).
 */
class ImageBase {
protected:
    std::string _name;
    std::string _format;
    time_t _creationTime;

    // Helper function for copying - called by both copy assignment
    void copyFrom(const ImageBase& other);

public:
    ImageBase(const std::string& name = "untitled", const std::string& format = "unknown");
    /**
     * Destructor for base classes usually should be declared virtual to be implemented by 
     * derived classes.
     * Suppose that the base class wouldn't be virtual. Then when an object: ImageBase *bobj = Image(10, 10), 
     * would be declared, then at the destruction, it will have memory leaks, because implementation of 
     * base destructor would be invoked, not the derived one.
     */
    virtual ~ImageBase();

    // copy constructor
    ImageBase(const ImageBase& other);

    // copy assignment op
    ImageBase& operator=(const ImageBase& other);

    // move ctor
    ImageBase(ImageBase&& other) noexcept;

    // move assignment op
    ImageBase& operator=(ImageBase&& other) noexcept;

    // getters
    std::string getName() const { return _name; }
    std::string getFormat() const { return _format; }
    time_t getCreationTime() const { return _creationTime; }

    // setters
    void setName(const std::string& name) { _name = name; }
    void setFormat(const std::string& format) { _format = format; }
};