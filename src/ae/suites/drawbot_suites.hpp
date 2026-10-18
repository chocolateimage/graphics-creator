#pragma once
#include "ae/ae_base.hpp"

struct DrawbotDrawSuite1 {
    DrawbotDrawSuite1();

    void *getSupplier;
    void *getSurface;
};

struct DrawbotSupplierSuite1 {
    DrawbotSupplierSuite1();

    void *newPen;
    void *newBrush;
    void *supportsText;
    void *getDefaultFontSize;
    void *newDefaultFont;
    void *newImageFromBuffer;
    void *newPath;
    void *supportsPixelLayoutBGRA;
    void *prefersPixelLayoutBGRA;
    void *supportsPixelLayoutARGB;
    void *prefersPixelLayoutARGB;
    void *retainObject;
    void *releaseObject;
};

struct DrawbotSurfaceSuite2 {
    DrawbotSurfaceSuite2();

    void *pushStateStack;
    void *popStateStack;
    void *paintRect;
    void *fillPath;
    void *strokePath;
    void *clip;
    void *getClipBounds;
    void *isWithinClipBounds;
    void *transform;
    void *drawString;
    void *drawImage;
    void *setInterpolationPolicy;
    void *getInterpolationPolicy;
    void *setAntialiasPolicy;
    void *getAntialiasPolicy;
    void *flush;
    void *getTransformToScreenScale;
};

struct DrawbotPathSuite1 {
    DrawbotPathSuite1();

    void *moveTo;
    void *lineTo;
    void *bezierTo;
    void *addRect;
    void *addArc;
    void *close;
};
