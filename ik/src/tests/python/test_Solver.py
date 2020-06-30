import ik
import unittest
from math import pi


class CustomAssertions:
    def assertFloatEqual(self, a, b):
        if not isclose(a, b, rel_tol=1e-7, abs_tol=0.0):
            raise AssertionError(f"{a} != {b}")


class TestSolver(unittest.TestCase, CustomAssertions):
    def test_one_bone(self):
        pass

    def test_fabrik(self):
        rn = ik.Node()
        mid = rn.create_child(position=ik.Vec3(0, 0, 1)).create_child(position=ik.Vec3(0, 0, 1), rotation=ik.Quat((0, 1, 0), pi/2))
        tip1 = mid.create_child(position=ik.Vec3(0, 1, 1)).create_child(position=ik.Vec3(0, 0, 1))
        tip2 = mid.create_child(position=ik.Vec3(0, -1, 1)).create_child(position=ik.Vec3(0, 0, 1))

        tip1.effector = ik.Effector()
        tip2.effector = ik.Effector()
        rn.algorithm = ik.Algorithm(ik.FABRIK)

        s = ik.Solver(rn)
        print(rn)
        s.solve()
        print(rn)
