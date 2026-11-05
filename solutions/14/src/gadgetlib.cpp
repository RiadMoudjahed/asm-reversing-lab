// Exercise 14 test library, built into exercises/14-shape-shifter/bin/gadgetlib.o. It exports
// three functions with C linkage that the test program uses; everything else is hidden.
namespace {
#include "classes.h"
}

extern "C" {

// The value that the first word of an object of this class must hold.
__attribute__((visibility("default"))) const void *gadget_vptr(int kind)
{
    switch (kind & 3) {
    case 0: { Device o(0); return *reinterpret_cast<const void *const *>(&o); }
    case 1: { Sensor o(0, 0); return *reinterpret_cast<const void *const *>(&o); }
    case 2: { Camera o(0, 0, 0); return *reinterpret_cast<const void *const *>(&o); }
    default: { Lamp o(0, 0); return *reinterpret_cast<const void *const *>(&o); }
    }
}

// Calls a virtual function through the object's own vtable: slot 0 is power, 1 is cost,
// 2 is tag.
__attribute__((visibility("default"))) int gadget_slot(const void *obj, int slot, int arg)
{
    const Device *d = static_cast<const Device *>(obj);
    switch (slot) {
    case 0: return d->power(arg);
    case 1: return d->cost(arg);
    default: return d->tag(arg);
    }
}

}
