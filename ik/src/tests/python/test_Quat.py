import ik
import unittest

class TestQuat(unittest.TestCase):
    def test_default_construct(self):
        q = ik.Quat()
        self.assertEqual(q.x, 0.0)
        self.assertEqual(q.y, 0.0)
        self.assertEqual(q.z, 0.0)
        self.assertEqual(q.w, 1.0)

    def test_construct_with_two_values_fails(self):
        with self.assertRaises(TypeError):
            q = ik.Quat(1, 2)

    def test_construct_with_four_values(self):
        q = ik.Quat(1, 2, 3, 4)
        self.assertEqual(q.x, 1.0)
        self.assertEqual(q.y, 2.0)
        self.assertEqual(q.z, 3.0)
        self.assertEqual(q.w, 4.0)

    def test_construct_with_invalid_types(self):
        with self.assertRaises(TypeError):
            q = ik.Quat("haha", "b", "c", "d")

    def test_construct_with_other_vector(self):
        q1 = ik.Quat(1, 2, 3, 4)
        q2 = ik.Quat(q1)
        self.assertIsNot(q1, q2)
        self.assertEqual(q2.x, 1.0)
        self.assertEqual(q2.y, 2.0)
        self.assertEqual(q2.z, 3.0)
        self.assertEqual(q2.w, 4.0)

    def test_set_identity(self):
        q = ik.Quat(1, 2, 3, 4)
        q.set_identity()
        self.assertEqual(q.x, 0.0)
        self.assertEqual(q.y, 0.0)
        self.assertEqual(q.z, 0.0)
        self.assertEqual(q.w, 1.0)

    def test_set_other_quaternion(self):
        q = ik.Quat()
        q.set(ik.Quat(4, 5, 6, 7))
        self.assertEqual(q.x, 4.0)
        self.assertEqual(q.y, 5.0)
        self.assertEqual(q.z, 6.0)
        self.assertEqual(q.w, 7.0)

    def test_set_values(self):
        q = ik.Quat()
        q.set(4, 5, 6, 7)
        self.assertEqual(q.x, 4.0)
        self.assertEqual(q.y, 5.0)
        self.assertEqual(q.z, 6.0)
        self.assertEqual(q.w, 7.0)
