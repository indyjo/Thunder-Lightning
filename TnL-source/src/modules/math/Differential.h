#ifndef DIFFERENTIAL_H
#define DIFFERENTIAL_H

template<class T>
class Differential {
    T last_value;
    bool initialized;
    
public:
    Differential() : initialized(false) { }
    
    T differentiate(const T& value, float delta_t) {
        if (initialized) {
            T res = (value - last_value) / delta_t;
            last_value = value;
            return res;
        } else {
            initialized = true;
            last_value = value;
            return 0;
        }
    }
    
    void reset() { initialized = false; }
};

template<class T>
class Integral {
    T integral_value;
    
public:
    Integral() : integral_value(0) { }
    
    T integrate(const T& value, float delta_t) {
        integral_value += delta_t * value;
        return integral_value;
    }

    void reset() { integral_value = 0; }
};

#endif

