#ifndef IFONT_H
#define IFONT_H

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
    virtual void drawString(
        const char *str,                                    ///< The string to draw
        const Vector2 & pos,                                ///< The position to draw at
        const Vector3 & color=Vector3(1,1,1),               ///< The RGB color to draw in
        float alpha=1,                                      ///< The opacity to draw with
        int   anchor = TOP | LEFT                           ///< Which point of the text box 'pos' specifies
        )=0;
};

#endif
