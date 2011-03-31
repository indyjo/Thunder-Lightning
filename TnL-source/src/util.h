#ifndef UTIL_H
#define UTIL_H

template <class T, int BUFFERSIZE> class buffer {
    int end, size;
    T buf[BUFFERSIZE];
public:
    inline buffer() : end(0), size(0) { }
    
    inline void add(T& el) {
        buf[end]=el;
        if (++end == BUFFERSIZE) end=0;
        if (size++ == BUFFERSIZE) size--;
    }
    
    inline T& operator[] (int index) {
        //ls_error("Element %d\n", (end - size + index + BUFFERSIZE) % BUFFERSIZE);
        return buf[(end - size + index + BUFFERSIZE) % BUFFERSIZE];
    }
    
    inline int getSize() { return size; }
};

#endif
