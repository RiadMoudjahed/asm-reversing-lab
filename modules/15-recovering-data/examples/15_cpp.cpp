// Purpose : a small C++ class hierarchy to show objects, vtables and virtual calls.
// Build   : see the Makefile; the lesson reads the -O1 output and the object file.
struct Shape {
    int id;
    Shape(int i) : id(i) {}
    virtual ~Shape() {}
    virtual int area() const { return 0; }
    virtual int sides() const { return 0; }
    int ident() const { return id; }
};

struct Rect : Shape {
    int w, h;
    Rect(int i, int w_, int h_) : Shape(i), w(w_), h(h_) {}
    int area() const override { return w * h; }
    int sides() const override { return 4; }
};

struct Square : Rect {
    Square(int i, int s) : Rect(i, s, s) {}
    int sides() const override { return 4 + 0; }
};

int total_area(const Shape *s) { return s->area() + s->sides(); }

int make_and_measure(void)
{
    Rect r(1, 3, 4);
    Square q(2, 5);
    return total_area(&r) + total_area(&q);
}
