import ik
import unittest

class TestEffector(unittest.TestCase):
    def test_create_effector(self):
        s = ik.Solver("FABRIK")
        e = ik.Effector(s)

    def test_assign_tuple_to_target_position(self):
        s = ik.Solver("FABRIK")
        e = ik.Effector(s)
        e.target_position = (1, 2, 3)
        self.assertEquals(e.target_position, ik.Vec3(1, 2, 3))

