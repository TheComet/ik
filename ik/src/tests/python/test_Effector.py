import ik
import unittest

class TestEffector(unittest.TestCase):
    def test_construct_with_chain_length(self):
        e = ik.Effector(chain_length=2)

    def test_assign_target_position(self):
        e = ik.Effector()
        v = e.target_position
        v.x = 4.0
        e.target_position = ik.Vec3()
        self.assertFloatEqual(v.x, 4.0)
