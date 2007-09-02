#ifndef IFONT_H
#define IFONT_H

#include <modules/math/Vector.h>
#include "IFontMetrics.h"

struct IFont : public IFontMetrics {
    /// An enumeration specifying symbolic values for the 'pos' parameter of IFont::drawString()
    enum Anchor {
        HMASK   = 0x0f,
        VMASK   = 0xf0,

        LEFT    = 0x00,
        RIGHT   = 0x01,
        HCENTER = 0x02,
        TOP     = 0x00,
        BOTTOM  = 0x10,
        VCENTER = 0x20
    };
    
    /// Draws a string at a specified pen position with specified color, alpha and alignment.
    virtual void drawString(
        const char *str,                                    ///< The string to draw
        const Vector2 & pos,                                ///< The pen position to draw at
        const Vector3 & color=Vector3(1,1,1),               ///< The RGB color to draw in
        float alpha=1,                                      ///< The opacity to draw with
        int   anchor = TOP | LEFT,                          ///< Which point of the text box 'pos' specifies
        float startofline_x=0,                              ///< X coordinate to reset pen to after a newline
        Vector2 * out_pos=0                                 ///< (Optional) pen position after drawing
        )=0;
};

#endif
