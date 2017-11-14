#ifndef MUI_UIWIDGET
#define MUI_UIWIDGET

namespace GUI {
  struct Rect
  {
    Rect (float _top=1.0, float _right=1.0, float _bottom=-1.0, float _left=-1.0)
      : top(_top), right(_right), bottom(_bottom), left(_left) {}

    float top;
    float right;
    float bottom;
    float left;
  };



  class Widget
  {
  public:
    Widget() {};
    ~Widget() {};

    Rect getRect() { return rect; }
    virtual void setRect(const Rect _rect) {
      rect = _rect;
    }

    virtual void draw() {};

    bool isGrabbed() { return grabbed; };

  protected:
    Rect rect;
    bool grabbed;
  };

}

#endif
