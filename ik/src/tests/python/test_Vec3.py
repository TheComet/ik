import ik
import unittest

class TestVec3(unittest.TestCase):
    def default_construct(self):
        v = ik.Vec3()
        self.assertEqual(v.x, 0.0)
        self.assertEqual(v.y, 0.0)
        self.assertEqual(v.z, 0.0)

    def construct_with_two_values(self):
        v = ik.Vec3(1, 2)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)
        self.assertEqual(v.z, 0.0);

    def construct_with_three_values(self):
        v = ik.Vec3(1, 2, 3)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)
        self.assertEqual(v.z, 3.0)

    def construct_with_invalid_types(self):
        with self.assertRaises(TypeError):
            v = ik.Vec3("haha", "b", "c")

    def set_zero(self):
        v = ik.Vec3(1, 2, 3)
        v.set_zero()
        self.assertEqual(v.x, 0.0)
        self.assertEqual(v.y, 0.0)
        self.assertEqual(v.z, 0.0)

    def set_using_another_vector(self):
        v = ik.Vec3()
        v.set(ik.Vec3(4, 5, 6))
        self.assertEqual(v.x, 4)
        self.assertEqual(v.x, 5)
        self.assertEqual(v.x, 6)

    def set_using_tuple(self):
        v = ik.Vec3()
        v.set((4, 5, 6))
        self.assertEqual(v.x, 4)
        self.assertEqual(v.x, 5)
        self.assertEqual(v.x, 6)

suite = unittest.TestSuite()
suite.addTest(TestVec3())
unittest.TextTestRunner().run(suite)

