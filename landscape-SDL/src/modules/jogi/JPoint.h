#ifndef _JOGI_POINT_H
#define _JOGI_POINT_H

#include <iostream>

class JPoint
{
public:
    jpoint_t point;

    /* Initialisiert den Punkt mit (0|0|0) */
    JPoint();

    /* Initialisiert den Punkt mit einer point_t-Struktur */
    JPoint(const jpoint_t *point);

    /* Initialisiert den Punkt mit den angegebenen Werten */
    JPoint(float x, float y, float z);

    /* Setzt den Punkt auf (0|0|0) */
    void clear();

    /* Setzt den Punkt auf die Koordinaten der angegebenen jpoint_t-Struktur */
    void set(const jpoint_t *p);

    /* Setzt den Punkt auf die angegebenen Werte */
    void set (float x, float y, float z);

	/* Addiert einen anderen Punkt hinzu */
    void add(const jpoint_t *add);

	/* Addiert die angegebenen Werte zum Punkt hinzu */
    void add(float addx, float addy, float addz);
    
    /* Subtrahiert einen anderen Punkt */
    void sub(const jpoint_t *sub);

	/* Subtrahiert die angegebenen Werte vom Punkt */
    void sub(float subx, float suby, float subz);
    
    /* Skaliert den Punkt zum Ursprung um den Faktor s */
    void scale(float s);

    /* Skaliert den Punkt zum angegeben Punkt um den Faktor s */
    void scale(float s, const jpoint_t *p);

    /* Rotiert den Punkt um den Ursprung um a, entlang der X-Achse */
    void rotateX(float a);

    /* Rotiert den Punkt um den Ursprung um a, entlang der Y-Achse */
    void rotateY(float a);

    /* Rotiert den Punkt um den Ursprung um a, entlang der Z-Achse */
    void rotateZ(float a);

    /* Rotiert den Punkt um den Ursprung um a, b und c */
    void rotate(float a, float b, float c);

    /* Rotiert den Punkt um den angegeben Punkt um a */
    void rotate(float a, jpoint_t *p);

    /* Wendet eine 4x4-Matrix auf den Punkt an */
    void applyMatrix(const jmatrix_t *m);

    /* returns the length of the vector */
    float getLength();

    /* makes the vector have unit length */
    void normalize();

    /* Calculates the cross product of the two given vectors */
    void vcross(const jpoint_t *p1, const jpoint_t *p2);

    /* Gibt die Koordinaten des Punkts auf stderr aus */
    void dump(std::ostream & out = std::cerr);
};


#endif
