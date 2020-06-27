import ik
import unittest
from math import pi, sqrt, isclose

class CustomAssertions:
    def assertFloatEqual(self, a, b):
        if not isclose(a, b, rel_tol=1e-7, abs_tol=0.0):
            raise AssertionError(f"{a} != {b}")

class TestQuat(unittest.TestCase, CustomAssertions):
    def test_default_construct(self):
        q = ik.Quat()
        self.assertEqual(q.x, 0.0)
        self.assertEqual(q.y, 0.0)
        self.assertEqual(q.z, 0.0)
        self.assertEqual(q.w, 1.0)

    def test_construct_with_four_values(self):
        q = ik.Quat(1, 2, 3, 4)
        self.assertEqual(q.x, 1.0)
        self.assertEqual(q.y, 2.0)
        self.assertEqual(q.z, 3.0)
        self.assertEqual(q.w, 4.0)

    def test_construct_with_other_quaternion(self):
        q1 = ik.Quat(1, 2, 3, 4)
        q2 = ik.Quat(q1)
        self.assertIsNot(q1, q2)
        self.assertEqual(q2.x, 1.0)
        self.assertEqual(q2.y, 2.0)
        self.assertEqual(q2.z, 3.0)
        self.assertEqual(q2.w, 4.0)

    def test_construct_with_four_values_tuple(self):
        q = ik.Quat((1, 2, 3, 4))
        self.assertEqual(q.x, 1.0)
        self.assertEqual(q.y, 2.0)
        self.assertEqual(q.z, 3.0)
        self.assertEqual(q.w, 4.0)

    def test_construct_with_axis_angle_tuple(self):
        q = ik.Quat((4, 0, 0), pi/2)
        self.assertFloatEqual(q.x, 1.0/sqrt(2))
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 0.0)
        self.assertFloatEqual(q.w, 1.0/sqrt(2))

    def test_construct_with_axis_angle_vec3(self):
        q = ik.Quat(ik.Vec3(4, 0, 0), pi/2)
        self.assertFloatEqual(q.x, 1.0/sqrt(2))
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 0.0)
        self.assertFloatEqual(q.w, 1.0/sqrt(2))

    def test_construct_with_vec3_vec3(self):
        q = ik.Quat(ik.Vec3(2, 0, 0), ik.Vec3(0, 2, 0))
        self.assertFloatEqual(q.x, 0.0)
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 1.0/sqrt(2))
        self.assertFloatEqual(q.w, 1.0/sqrt(2))

    def test_construct_with_tuple_vec3(self):
        q = ik.Quat((2, 0, 0), ik.Vec3(0, 2, 0))
        self.assertFloatEqual(q.x, 0.0)
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 1.0/sqrt(2))
        self.assertFloatEqual(q.w, 1.0/sqrt(2))

    def test_construct_with_vec3_tuple(self):
        q = ik.Quat(ik.Vec3(2, 0, 0), (0, 2, 0))
        self.assertFloatEqual(q.x, 0.0)
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 1.0/sqrt(2))
        self.assertFloatEqual(q.w, 1.0/sqrt(2))

    def test_construct_with_tuple_tuple(self):
        q = ik.Quat((2, 0, 0), (0, 2, 0))
        self.assertFloatEqual(q.x, 0.0)
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 1.0/sqrt(2))
        self.assertFloatEqual(q.w, 1.0/sqrt(2))

    def test_construct_with_vector(self):
        q = ik.Quat(ik.Vec3(0, 1, 0))
        self.assertFloatEqual(q.x, -1.0/sqrt(2))
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 0.0)
        self.assertFloatEqual(q.w, 1.0/sqrt(2))

    def test_construct_with_invalid_types_fails(self):
        with self.assertRaises(TypeError):
            q = ik.Quat("haha", "b", "c", "d")
        with self.assertRaises(TypeError):
            q = ik.Quat("haha")

    def test_construct_with_two_values_fails(self):
        with self.assertRaises(TypeError):
            q = ik.Quat(1, 2)

    def test_set_identity(self):
        q = ik.Quat(1, 2, 3, 4)
        q.set_identity()
        self.assertEqual(q.x, 0.0)
        self.assertEqual(q.y, 0.0)
        self.assertEqual(q.z, 0.0)
        self.assertEqual(q.w, 1.0)

    def test_set_with_four_values(self):
        q = ik.Quat()
        q.set(1, 2, 3, 4)
        self.assertEqual(q.x, 1.0)
        self.assertEqual(q.y, 2.0)
        self.assertEqual(q.z, 3.0)
        self.assertEqual(q.w, 4.0)

    def test_set_with_other_quaternion(self):
        q1 = ik.Quat(1, 2, 3, 4)
        q2 = ik.Quat()
        q2.set(q1)
        self.assertIsNot(q1, q2)
        self.assertEqual(q2.x, 1.0)
        self.assertEqual(q2.y, 2.0)
        self.assertEqual(q2.z, 3.0)
        self.assertEqual(q2.w, 4.0)

    def test_set_with_four_values_tuple(self):
        q = ik.Quat()
        q.set((1, 2, 3, 4))
        self.assertEqual(q.x, 1.0)
        self.assertEqual(q.y, 2.0)
        self.assertEqual(q.z, 3.0)
        self.assertEqual(q.w, 4.0)

    def test_set_with_axis_angle_tuple(self):
        q = ik.Quat()
        q.set((4, 0, 0), pi/2)
        self.assertFloatEqual(q.x, 1.0/sqrt(2))
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 0.0)
        self.assertFloatEqual(q.w, 1.0/sqrt(2))

    def test_set_with_axis_angle_vec3(self):
        q = ik.Quat()
        q.set(ik.Vec3(4, 0, 0), pi/2)
        self.assertFloatEqual(q.x, 1.0/sqrt(2))
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 0.0)
        self.assertFloatEqual(q.w, 1.0/sqrt(2))

    def test_set_with_vec3_vec3(self):
        q = ik.Quat()
        q.set(ik.Vec3(2, 0, 0), ik.Vec3(0, 2, 0))
        self.assertFloatEqual(q.x, 0.0)
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 1.0/sqrt(2))
        self.assertFloatEqual(q.w, 1.0/sqrt(2))

    def test_set_with_tuple_vec3(self):
        q = ik.Quat()
        q.set((2, 0, 0), ik.Vec3(0, 2, 0))
        self.assertFloatEqual(q.x, 0.0)
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 1.0/sqrt(2))
        self.assertFloatEqual(q.w, 1.0/sqrt(2))

    def test_set_with_vec3_tuple(self):
        q = ik.Quat()
        q.set(ik.Vec3(2, 0, 0), (0, 2, 0))
        self.assertFloatEqual(q.x, 0.0)
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 1.0/sqrt(2))
        self.assertFloatEqual(q.w, 1.0/sqrt(2))

    def test_set_with_tuple_tuple(self):
        q = ik.Quat()
        q.set((2, 0, 0), (0, 2, 0))
        self.assertFloatEqual(q.x, 0.0)
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 1.0/sqrt(2))
        self.assertFloatEqual(q.w, 1.0/sqrt(2))

    def test_set_with_vector(self):
        q = ik.Quat()
        q.set(ik.Vec3(0, 1, 0))
        self.assertFloatEqual(q.x, -1.0/sqrt(2))
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 0.0)
        self.assertFloatEqual(q.w, 1.0/sqrt(2))

    def test_set_with_invalid_types_fails(self):
        q = ik.Quat()
        with self.assertRaises(TypeError):
            q.set("haha", "b", "c", "d")

    def test_set_with_two_values_fails(self):
        q = ik.Quat()
        with self.assertRaises(TypeError):
            q.set(1, 2)

    def test_add_meth(self):
        q = ik.Quat(1, 2, 3, 4)
        q.add(ik.Quat(1, 2, 3, 4))
        self.assertEqual(q.x, 2)
        self.assertEqual(q.y, 4)
        self.assertEqual(q.z, 6)
        self.assertEqual(q.w, 8)

    def test_add_inplace(self):
        q = ik.Quat(1, 2, 3, 4)
        q += ik.Quat(1, 2, 3, 4)
        self.assertEqual(q.x, 2)
        self.assertEqual(q.y, 4)
        self.assertEqual(q.z, 6)
        self.assertEqual(q.w, 8)

    def test_add(self):
        q1 = ik.Quat(1, 2, 3, 4)
        q2 = ik.Quat(1, 2, 3, 4)
        q3 = q1 + q2
        self.assertIsNot(q3, q1)
        self.assertIsNot(q3, q2)
        self.assertEqual(q3.x, 2)
        self.assertEqual(q3.y, 4)
        self.assertEqual(q3.z, 6)
        self.assertEqual(q3.w, 8)

    def test_sub_meth(self):
        q = ik.Quat(1, 2, 3, 4)
        q.sub(ik.Quat(4, 3, 2, 1))
        self.assertEqual(q.x, -3)
        self.assertEqual(q.y, -1)
        self.assertEqual(q.z, 1)
        self.assertEqual(q.w, 3)

    def test_sub_inplace(self):
        q = ik.Quat(1, 2, 3, 4)
        q -= ik.Quat(4, 3, 2, 1)
        self.assertEqual(q.x, -3)
        self.assertEqual(q.y, -1)
        self.assertEqual(q.z, 1)
        self.assertEqual(q.w, 3)

    def test_sub(self):
        q1 = ik.Quat(1, 2, 3, 4)
        q2 = ik.Quat(4, 3, 2, 1)
        q3 = q1 - q2
        self.assertIsNot(q3, q1)
        self.assertIsNot(q3, q2)
        self.assertEqual(q3.x, -3)
        self.assertEqual(q3.y, -1)
        self.assertEqual(q3.z, 1)
        self.assertEqual(q3.w, 3)

    def test_mul_meth(self):
        q = ik.Quat((1, 0, 0), pi/4)
        q.mul(ik.Quat((1, 0, 0), pi/4))
        self.assertFloatEqual(q.x, 1.0/sqrt(2))
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 0.0)
        self.assertFloatEqual(q.w, 1.0/sqrt(2))

    def test_mul_inplace(self):
        q = ik.Quat((1, 0, 0), pi/4)
        q *= ik.Quat((1, 0, 0), pi/4)
        self.assertFloatEqual(q.x, 1.0/sqrt(2))
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 0.0)
        self.assertFloatEqual(q.w, 1.0/sqrt(2))

    def test_mul(self):
        q1 = ik.Quat((1, 0, 0), pi/4)
        q2 = ik.Quat((1, 0, 0), pi/4)
        q3 = q1 * q2
        self.assertIsNot(q3, q1)
        self.assertIsNot(q3, q2)
        self.assertFloatEqual(q3.x, 1.0/sqrt(2))
        self.assertFloatEqual(q3.y, 0.0)
        self.assertFloatEqual(q3.z, 0.0)
        self.assertFloatEqual(q3.w, 1.0/sqrt(2))

    def test_div_meth(self):
        q = ik.Quat((1, 0, 0), pi/4)
        q.div(ik.Quat((1, 0, 0), pi/4))
        self.assertFloatEqual(q.x, 0.0)
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 0.0)
        self.assertFloatEqual(q.w, 1.0)

    def test_div_inplace(self):
        q = ik.Quat((1, 0, 0), pi/4)
        q /= ik.Quat((1, 0, 0), pi/4)
        self.assertFloatEqual(q.x, 0.0)
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 0.0)
        self.assertFloatEqual(q.w, 1.0)

    def test_div(self):
        q1 = ik.Quat((1, 0, 0), pi/4)
        q2 = ik.Quat((1, 0, 0), pi/4)
        q3 = q1 / q2
        self.assertIsNot(q3, q1)
        self.assertIsNot(q3, q2)
        self.assertFloatEqual(q3.x, 0.0)
        self.assertFloatEqual(q3.y, 0.0)
        self.assertFloatEqual(q3.z, 0.0)
        self.assertFloatEqual(q3.w, 1.0)

    def test_pow_meth(self):
        q = ik.Quat((1, 0, 0), pi/4)
        q.pow(66)
        self.assertFloatEqual(q.x, 1.0/sqrt(2))
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 0.0)
        self.assertFloatEqual(q.w, 1.0/sqrt(2))

    def test_pow_inplace(self):
        q = ik.Quat((1, 0, 0), pi/4)
        q **= 66
        self.assertFloatEqual(q.x, 1.0/sqrt(2))
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 0.0)
        self.assertFloatEqual(q.w, 1.0/sqrt(2))

    def test_pow(self):
        q1 = ik.Quat((1, 0, 0), pi/4)
        q2 = ik.Quat((1, 0, 0), pi/4)
        q3 = q1 ** 66
        self.assertIsNot(q3, q1)
        self.assertIsNot(q3, q2)
        self.assertFloatEqual(q3.x, 1.0/sqrt(2))
        self.assertFloatEqual(q3.y, 0.0)
        self.assertFloatEqual(q3.z, 0.0)
        self.assertFloatEqual(q3.w, 1.0/sqrt(2))

    def test_negate(self):
        q = ik.Quat(1, 2, 3, 4)
        q2 = -q
        self.assertIsNot(q2, q)
        self.assertEqual(q.x, 1)
        self.assertEqual(q.y, 2)
        self.assertEqual(q.z, 3)
        self.assertEqual(q.w, 4)
        self.assertEqual(q2.x, -1)
        self.assertEqual(q2.y, -2)
        self.assertEqual(q2.z, -3)
        self.assertEqual(q2.w, -4)

    def test_negate_meth(self):
        q = ik.Quat(1, 2, 3, 4)
        q.negate()
        self.assertEqual(q.x, -1)
        self.assertEqual(q.y, -2)
        self.assertEqual(q.z, -3)
        self.assertEqual(q.w, -4)

    def test_negated(self):
        q = ik.Quat(1, 2, 3, 4)
        q2 = q.negated()
        self.assertIsNot(q2, q)
        self.assertEqual(q.x, 1)
        self.assertEqual(q.y, 2)
        self.assertEqual(q.z, 3)
        self.assertEqual(q.w, 4)
        self.assertEqual(q2.x, -1)
        self.assertEqual(q2.y, -2)
        self.assertEqual(q2.z, -3)
        self.assertEqual(q2.w, -4)

    def test_mag(self):
        q = ik.Quat(1, 2, 3, 4)
        self.assertFloatEqual(abs(q), sqrt(30))

    def test_mag_meth(self):
        q = ik.Quat(1, 2, 3, 4)
        self.assertFloatEqual(q.mag(), sqrt(30))

    def test_dot(self):
        q1 = ik.Quat(1, 2, 3, 4)
        q2 = ik.Quat(5, 6, 7, 8)
        self.assertFloatEqual(q1.dot(q2), 70)

    def test_conjugate(self):
        q = ik.Quat(1, 2, 3, 4)
        q.conjugate()
        self.assertEqual(q.x, -1)
        self.assertEqual(q.y, -2)
        self.assertEqual(q.z, -3)
        self.assertEqual(q.w, 4)

    def test_conjugated(self):
        q1 = ik.Quat(1, 2, 3, 4)
        q2 = q1.conjugated()
        self.assertIsNot(q2, q1)
        self.assertEqual(q1.x, 1)
        self.assertEqual(q1.y, 2)
        self.assertEqual(q1.z, 3)
        self.assertEqual(q1.w, 4)
        self.assertEqual(q2.x, -1)
        self.assertEqual(q2.y, -2)
        self.assertEqual(q2.z, -3)
        self.assertEqual(q2.w, 4)

    def test_invert(self):
        q = ik.Quat((1, 0, 0), pi/2) * 2
        mag2 = q.dot(q)
        q.invert()
        self.assertFloatEqual(q.x, -2.0/sqrt(2) / mag2)
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 0.0)
        self.assertFloatEqual(q.w, 2.0/sqrt(2) / mag2)

    def test_inverted(self):
        q1 = ik.Quat((1, 0, 0), pi/2) * 2
        mag2 = q1.dot(q1)
        q2 = q1.inverted()
        self.assertIsNot(q2, q1)
        self.assertFloatEqual(q1.x, 2.0/sqrt(2))
        self.assertFloatEqual(q1.y, 0.0)
        self.assertFloatEqual(q1.z, 0.0)
        self.assertFloatEqual(q1.w, 2.0/sqrt(2))
        self.assertFloatEqual(q2.x, -2.0/sqrt(2) / mag2)
        self.assertFloatEqual(q2.y, 0.0)
        self.assertFloatEqual(q2.z, 0.0)
        self.assertFloatEqual(q2.w, 2.0/sqrt(2) / mag2)

    def test_normalize(self):
        q = ik.Quat((1, 0, 0), pi/2) * 2
        q.normalize()
        self.assertFloatEqual(q.x, 1.0/sqrt(2))
        self.assertFloatEqual(q.y, 0.0)
        self.assertFloatEqual(q.z, 0.0)
        self.assertFloatEqual(q.w, 1.0/sqrt(2))

    def test_normalized(self):
        q1 = ik.Quat((1, 0, 0), pi/2) * 2
        q2 = q1.normalized()
        self.assertIsNot(q2, q1)
        self.assertFloatEqual(q1.x, 2/sqrt(2))
        self.assertFloatEqual(q1.y, 0.0)
        self.assertFloatEqual(q1.z, 0.0)
        self.assertFloatEqual(q1.w, 2/sqrt(2))
        self.assertFloatEqual(q2.x, 1.0/sqrt(2))
        self.assertFloatEqual(q2.y, 0.0)
        self.assertFloatEqual(q2.z, 0.0)
        self.assertFloatEqual(q2.w, 1.0/sqrt(2))

    def test_ensure_positive_sign(self):
        q = ik.Quat(1, 2, 3, -4)
        q.ensure_positive_sign()
        self.assertEqual(q.x, -1.0)
        self.assertEqual(q.y, -2.0)
        self.assertEqual(q.z, -3.0)
        self.assertEqual(q.w, 4.0)
