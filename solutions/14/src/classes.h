// Exercise 14: the class hierarchy shared by the challenge program and the test library.
#pragma once

struct Device {
    int id;
    explicit Device(int i) : id(i) {}
    virtual int power(int m) const { return id * m; }
    virtual int cost(int q) const { return q + id; }
    virtual int tag(int s) const { return s ^ id; }
};

struct Sensor : Device {
    int range;
    Sensor(int i, int r) : Device(i), range(r) {}
    int power(int m) const override { return range * m + id; }
    int tag(int s) const override { return (s ^ id) + range; }
};

struct Camera : Sensor {
    int pixels;
    Camera(int i, int r, int p) : Sensor(i, r), pixels(p) {}
    int cost(int q) const override { return pixels / 4 + q; }
    int tag(int s) const override { return Sensor::tag(s) * 2 + pixels; }
};

struct Lamp : Device {
    int lumens;
    Lamp(int i, int l) : Device(i), lumens(l) {}
    int power(int m) const override { return lumens * m; }
    int cost(int q) const override { return lumens + q * 2; }
};
