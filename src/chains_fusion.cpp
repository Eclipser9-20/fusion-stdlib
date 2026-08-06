// Chains bindings for Fusion — real exported `namespace chains` functions with Fusion-friendly
// int args (colors as r,g,b,a channels; no Color/Config/Sprite objects crossing the boundary).
// So `import chains` then `chains::fillRect(x,y,w,h,r,g,b,a)` binds natively. Wraps the C-ABI in
// chains_c.h; the Chains repo is untouched (this lives on the Fusion side, like bindings/python).
#include "chains_c.h"

namespace chains {
    // window / loop
    int  open(const char* title, int w, int h) { return chains_open(title, w, h, 16000000, 1); }   // ~60fps, autosync
    int  open(const char* title, int w, int h, long frameNanos, int autosync) { return chains_open(title, w, h, frameNanos, autosync); }
    // (tick/present/close already exist in Chains' C++ API as chains::tick/present/close — call them directly)
    void fullscreen(int on) { chains_fullscreen(on); }
    void resizable(int on)  { chains_resizable(on); }

    // 2D drawing — colors as separate int channels
    void clear(int r, int g, int b) { chains_clear(r, g, b); }
    void plot(int x, int y, int r, int g, int b, int a) { chains_plot(x, y, r, g, b, a); }
    void fillRect(int x, int y, int w, int h, int r, int g, int b, int a)   { chains_fill_rect(x, y, w, h, r, g, b, a); }
    void strokeRect(int x, int y, int w, int h, int r, int g, int b, int a) { chains_stroke_rect(x, y, w, h, r, g, b, a); }
    void line(int x0, int y0, int x1, int y1, int r, int g, int b, int a)   { chains_line(x0, y0, x1, y1, r, g, b, a); }
    void circle(int cx, int cy, int radius, int r, int g, int b, int a, int filled) { chains_circle(cx, cy, radius, r, g, b, a, filled); }

    // sprites
    int  spriteSolid(int w, int h, int r, int g, int b, int a) { return chains_sprite_solid(w, h, r, g, b, a); }
    void draw(int sprite, int x, int y) { chains_draw(sprite, (float)x, (float)y); }   // int coords -> float internally

    // input — mouseX/mouseY fold Chains' float coords to ints (key/keyPressed/mouse already exist in Chains)
    int mouseX() { return (int)chains_mouse_x(); }
    int mouseY() { return (int)chains_mouse_y(); }
}
