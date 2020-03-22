import collections


Point = collections.namedtuple("Point", "y x")
Point._delta_udlr = (
    Point(-1, 0),
    Point(1, 0),
    Point(0, -1),
    Point(0, 1),
    Point(-1 , -1) ,
    Point ( 1 , -1 ) ,
    Point ( -1 , 1 ) ,
    Point ( 1 , 1 )
)


def _Point____add__(self, other):
    return Point(self.y + other.y,
                 self.x + other.x)


def _Point__udlr(self):
    """Get the four next points. UDLR means Up, Down, Left and Right."""
    return [self + delta for delta in self._delta_udlr]


Point.__add__ = _Point____add__
Point.udlr = _Point__udlr


def def_getter(cls, attr_name: str):
    """
    Define a getter property of a given class.
    e.g.
    In below code, code 1 is same as code 2.
    # Code 1
    def_getter(C, '_foo')
    # Code 2
    class C:
        @property
        def foo(self):
            return self._foo
    """
    prop_name = attr_name.lstrip("_")

    def getter(self):
        return getattr(self, attr_name)

    setattr(cls, prop_name, property(getter))