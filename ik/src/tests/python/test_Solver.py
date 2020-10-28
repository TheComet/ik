import ik
import unittest
from math import pi, isclose


class CustomAssertions:
    def assertFloatEqual(self, a, b):
        if not isclose(a, b, rel_tol=1e-7, abs_tol=1e-7):
            raise AssertionError(f"{a} != {b}")


class TestSolver(unittest.TestCase, CustomAssertions):
    pass
