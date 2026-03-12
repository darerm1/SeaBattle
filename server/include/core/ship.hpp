#pragma once

class Ship {
public:
    Ship(int length, int x, int y, bool is_horizontal) 
        : length_(length), x_(x), y_(y), is_horizontal_(is_horizontal) {}
    
    int get_length() const { return length_; }
    
private:
    int length_;
    int x_, y_;
    bool is_horizontal_;
};
