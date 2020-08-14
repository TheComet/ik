import ik
import unittest
from math import isclose

class CustomAssertions:
    def assertFloatEqual(self, a, b):
        if not isclose(a, b, rel_tol=1e-7, abs_tol=0.0):
            raise AssertionError(f"{a} != {b}")

class TestEffector(unittest.TestCase, CustomAssertions):
    def test_construct_with_chain_length(self):
        e = ik.Effector(chain_length=2)

    def test_assign_target_position(self):
        e = ik.Effector()
        v = e.target_position
        v.x = 4.0
        e.target_position = ik.Vec3()
        self.assertFloatEqual(v.x, 4.0)
