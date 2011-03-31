#ifndef IFONTMETRICS_H
#define IFONTMETRICS_H

#include <string>
#include <modules/math/Vector.h>
#include <object.h>

struct IFontMetrics : virtual public Object {
    /// Returns the width in pixels of a specfic character.
    virtual float getCharWidth(char c)=0;
    /// Returns the width of the widest character (usually 'W') in the font for rough estimations.
    virtual float getMaxCharWidth()=0;
    /// Returns the height in pixels of a line.
    virtual float getLineHeight()=0;
    /// Returns how far to advance the pen position after drawing this character
    virtual float getCharAdvance(char c)=0;

    /// Returns the space required for rendering a string in pixels.
    /// If there are newline characters in the string,
    /// returns the width of the widest line and the height of all lines.
    /// @param str          [in]    the string to process
    /// @param out_width    [out]   the width of the rendered text
    /// @param out_height   [out]   the height of the rendered text
    virtual void getStringDims(const char *str,
                               float *out_width=0,
                               float *out_height=0)=0;
    /// Returns the length of a prefix of str that fits into the desired width.
    /// @param str the sting that is to be constrained
    /// @param max_width the width in pixels
    /// @param partial whether the last character is allowed to fit partially into max_width
    /// @note str may not contain newline characters!
    virtual int constrainString(const char *str, float max_width, bool partial=false)=0;
};

#endif
