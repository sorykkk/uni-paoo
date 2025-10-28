# Image Box 
**Image Box** is a small C++ library created to demonstrate the **Rule of 5** and other class design concepts through image manipulation functionality.

---

## Purpose
This mini library was developed for educational purposes — to illustrate how constructors, destructors, copy/move semantics, and encapsulation work in C++ classes.

---

## Basic Requirements
 - CMake build system  
 - Proper encapsulation  
 - Constructor member initialization  
 - Destructor that frees heap memory  
 - Copy constructor override  
 - Move constructor override  

---

## Image Manipulation Features
 - Read/Write image functionality (via lightweight single-header third-party libraries)  
 - Flip **horizontally**  
 - Flip **vertically**  
 - Convert to **grayscale**  
 - Query **image height** and **width**

---

## Instalation & build
```bash
git clone https://github.com/sorykkk/uni-paoo.git
cd uni-paoo/image-box
cmake -S . -Bbuild && cmake --build build -j