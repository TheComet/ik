import ik
import unittest
from math import pi, isclose


class CustomAssertions:
    def assertFloatEqual(self, a, b):
        if not isclose(a, b, rel_tol=1e-7, abs_tol=1e-7):
            raise AssertionError(f"{a} != {b}")


class TestSolver(unittest.TestCase, CustomAssertions):
    def test_one_bone(self):
        pass

    def test_combine(self):
        root = ik.Node(position=ik.Vec3(0, 0, 0), rotation=ik.Quat((1, 0, 0), pi))
        tip1 = root.create_child(position=ik.Vec3(0, 0, 2))
        tip2 = root.create_child(position=ik.Vec3(0, 0, 2))

        root.algorithm = ik.Algorithm(ik.ONE_BONE)
        tip1.effector = ik.Effector(target_position=ik.Vec3(0, -2, 2))
        tip2.effector = ik.Effector(target_position=ik.Vec3(0, 2, 2))

        s = ik.Solver(root)
        s.solve()

    def test_fabrik(self):
        rn = ik.Node()
        mid_rot = ik.Quat((2, 6, 1), pi/5)
        mid = rn.create_child(position=ik.Vec3(0, 0, 1)).create_child(position=ik.Vec3(0, 0, 1), rotation=mid_rot)
        midr = mid.create_child(position=ik.Vec3(0, 1, 1))
        midl = mid.create_child(position=ik.Vec3(0, -1, 1))
        tip1 = midr.create_child(position=ik.Vec3(0, 0, 1))
        tip2 = midl.create_child(position=ik.Vec3(0, 0, 1))

        tip1.effector = ik.Effector()
        tip2.effector = ik.Effector()
        rn.algorithm = ik.Algorithm(ik.FABRIK)

        s = ik.Solver(rn)
        print(rn)
        s.solve()
        print(rn)

        # mid rotation will have been averaged
        self.assertFloatEqual(mid.rotation.x, mid_rot.x)
        self.assertFloatEqual(mid.rotation.y, mid_rot.y)
        self.assertFloatEqual(mid.rotation.z, mid_rot.z)
        self.assertFloatEqual(mid.rotation.w, mid_rot.w)

        self.assertFloatEqual(midr.position.x, 0.0)
        self.assertFloatEqual(midr.position.y, 1.0)
        self.assertFloatEqual(midr.position.z, 1.0)

        self.assertFloatEqual(midl.position.x, 0.0)
        self.assertFloatEqual(midl.position.y, -1.0)
        self.assertFloatEqual(midl.position.z, 1.0)
