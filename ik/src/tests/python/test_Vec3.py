import ik
import unittest
from math import pi, sqrt, isclose, sin, cos

class CustomAssertions:
    def assertFloatEqual(self, a, b):
        if not isclose(a, b, rel_tol=1e-7, abs_tol=0.0):
            raise AssertionError(f"{a} != {b}")

class TestVec3(unittest.TestCase, CustomAssertions):
    def test_default_construct(self):
        v = ik.Vec3()
        self.assertEqual(v.x, 0.0)
        self.assertEqual(v.y, 0.0)
        self.assertEqual(v.z, 0.0)

    def test_construct_with_3_values(self):
        v = ik.Vec3(1, 2, 3)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)
        self.assertEqual(v.z, 3.0)

    def test_construct_with_other_vector(self):
        v1 = ik.Vec3(1, 2, 3)
        v2 = ik.Vec3(v1)
        self.assertIsNot(v1, v2)
        self.assertEqual(v2.x, 1.0)
        self.assertEqual(v2.y, 2.0)
        self.assertEqual(v2.z, 3.0)

    def test_construct_with_other_vector_tuple(self):
        v1 = ik.Vec3((1, 2, 3))
        v2 = ik.Vec3([1, 2, 3])
        self.assertEqual(v1.x, 1.0)
        self.assertEqual(v1.y, 2.0)
        self.assertEqual(v1.z, 3.0)
        self.assertEqual(v2.x, 1.0)
        self.assertEqual(v2.y, 2.0)
        self.assertEqual(v2.z, 3.0)

    def test_construct_with_4_values(self):
        v = ik.Vec3(0, sin(pi/8), 0, cos(pi/8))
        self.assertFloatEqual(v.x, 1.0/sqrt(2))
        self.assertFloatEqual(v.y, 0.0)
        self.assertFloatEqual(v.z, 1.0/sqrt(2))

    def test_construct_with_quaternion(self):
        q = ik.Quat(0, sin(pi/8), 0, cos(pi/8))
        v = ik.Vec3(q)
        self.assertFloatEqual(v.x, 1.0/sqrt(2))
        self.assertFloatEqual(v.y, 0.0)
        self.assertFloatEqual(v.z, 1.0/sqrt(2))

    def test_construct_with_quaternion_tuple(self):
        q = (0, sin(pi/8), 0, cos(pi/8))
        v = ik.Vec3(q)
        self.assertFloatEqual(v.x, 1.0/sqrt(2))
        self.assertFloatEqual(v.y, 0.0)
        self.assertFloatEqual(v.z, 1.0/sqrt(2))

    def test_construct_with_two_values_fails(self):
        with self.assertRaises(TypeError):
            v = ik.Vec3(1, 2)

    def test_construct_with_invalid_types(self):
        with self.assertRaises(TypeError):
            v = ik.Vec3("haha", "b", "c")
        with self.assertRaises(TypeError):
            v = ik.Vec3("heh")

    def test_construct_with_too_many_values(self):
        with self.assertRaises(TypeError):
            v = ik.Vec3(1, 2, 3, 4, 5)

    def test_set_zero(self):
        v = ik.Vec3(1, 2, 3)
        v.set_zero()
        self.assertEqual(v.x, 0.0)
        self.assertEqual(v.y, 0.0)
        self.assertEqual(v.z, 0.0)

    def test_set_with_3_values(self):
        v = ik.Vec3()
        v.set(1, 2, 3)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)
        self.assertEqual(v.z, 3.0)

    def test_set_with_other_vector(self):
        v1 = ik.Vec3(1, 2, 3)
        v2 = ik.Vec3()
        v2.set(v1)
        self.assertIsNot(v1, v2)
        self.assertEqual(v2.x, 1.0)
        self.assertEqual(v2.y, 2.0)
        self.assertEqual(v2.z, 3.0)

    def test_set_with_other_vector_tuple(self):
        v1 = (1, 2, 3)
        v2 = ik.Vec3()
        v2.set(v1)
        self.assertEqual(v2.x, 1.0)
        self.assertEqual(v2.y, 2.0)
        self.assertEqual(v2.z, 3.0)

    def test_set_with_4_values(self):
        v = ik.Vec3()
        v.set(0, sin(pi/8), 0, cos(pi/8))
        self.assertFloatEqual(v.x, 1.0/sqrt(2))
        self.assertFloatEqual(v.y, 0.0)
        self.assertFloatEqual(v.z, 1.0/sqrt(2))

    def test_set_with_quaternion(self):
        q = ik.Quat(0, sin(pi/8), 0, cos(pi/8))
        v = ik.Vec3()
        v.set(q)
        self.assertFloatEqual(v.x, 1.0/sqrt(2))
        self.assertFloatEqual(v.y, 0.0)
        self.assertFloatEqual(v.z, 1.0/sqrt(2))

    def test_set_with_quaternion_tuple(self):
        q = (0, sin(pi/8), 0, cos(pi/8))
        v = ik.Vec3()
        v.set(q)
        self.assertFloatEqual(v.x, 1.0/sqrt(2))
        self.assertFloatEqual(v.y, 0.0)
        self.assertFloatEqual(v.z, 1.0/sqrt(2))

    def test_set_with_two_values_fails(self):
        v = ik.Vec3()
        with self.assertRaises(TypeError):
            v.set(1, 2)

    def test_set_with_invalid_types(self):
        v = ik.Vec3()
        with self.assertRaises(TypeError):
            v.set("haha", "b", "c")
        with self.assertRaises(TypeError):
            v.set("haha")

    def test_set_with_too_many_values(self):
        v = ik.Vec3()
        with self.assertRaises(TypeError):
            v.set(1, 2, 3, 4, 5)

    def test_add_vec(self):
        v1 = ik.Vec3(1, 2, 3)
        v2 = ik.Vec3(4, 5, 6)
        v3 = v1 + v2
        self.assertIsNot(v3, v1)
        self.assertIsNot(v3, v2)
        self.assertEqual(v1.x, 1.0)
        self.assertEqual(v1.y, 2.0)
        self.assertEqual(v1.z, 3.0)
        self.assertEqual(v2.x, 4.0)
        self.assertEqual(v2.y, 5.0)
        self.assertEqual(v2.z, 6.0)
        self.assertEqual(v3.x, 5.0)
        self.assertEqual(v3.y, 7.0)
        self.assertEqual(v3.z, 9.0)

    def test_add_vec_inplace(self):
        v = ik.Vec3(1, 2, 3)
        v += ik.Vec3(4, 5, 6)
        self.assertEqual(v.x, 5.0)
        self.assertEqual(v.y, 7.0)
        self.assertEqual(v.z, 9.0)

    def test_add_vec_meth(self):
        v = ik.Vec3(1, 2, 3)
        v.add(ik.Vec3(4, 5, 6))
        self.assertEqual(v.x, 5.0)
        self.assertEqual(v.y, 7.0)
        self.assertEqual(v.z, 9.0)

    def test_add_scalar(self):
        v1 = ik.Vec3(1, 2, 3)
        v2 = v1 + 5
        self.assertIsNot(v2, v1)
        self.assertEqual(v1.x, 1.0)
        self.assertEqual(v1.y, 2.0)
        self.assertEqual(v1.z, 3.0)
        self.assertEqual(v2.x, 6.0)
        self.assertEqual(v2.y, 7.0)
        self.assertEqual(v2.z, 8.0)

    def test_add_scalar_inplace(self):
        v = ik.Vec3(1, 2, 3)
        v += 5
        self.assertEqual(v.x, 6.0)
        self.assertEqual(v.y, 7.0)
        self.assertEqual(v.z, 8.0)

    def test_add_scalar_meth(self):
        v = ik.Vec3(1, 2, 3)
        v.add(5)
        self.assertEqual(v.x, 6.0)
        self.assertEqual(v.y, 7.0)
        self.assertEqual(v.z, 8.0)

    def test_sub_vec(self):
        v1 = ik.Vec3(1, 2, 3)
        v2 = ik.Vec3(4, 5, 6)
        v3 = v1 - v2
        self.assertIsNot(v3, v1)
        self.assertIsNot(v3, v2)
        self.assertEqual(v1.x, 1.0)
        self.assertEqual(v1.y, 2.0)
        self.assertEqual(v1.z, 3.0)
        self.assertEqual(v2.x, 4.0)
        self.assertEqual(v2.y, 5.0)
        self.assertEqual(v2.z, 6.0)
        self.assertEqual(v3.x, -3.0)
        self.assertEqual(v3.y, -3.0)
        self.assertEqual(v3.z, -3.0)

    def test_sub_vec_inplace(self):
        v = ik.Vec3(1, 2, 3)
        v -= ik.Vec3(4, 5, 6)
        self.assertEqual(v.x, -3.0)
        self.assertEqual(v.y, -3.0)
        self.assertEqual(v.z, -3.0)

    def test_sub_vec_meth(self):
        v = ik.Vec3(1, 2, 3)
        v.sub(ik.Vec3(4, 5, 6))
        self.assertEqual(v.x, -3.0)
        self.assertEqual(v.y, -3.0)
        self.assertEqual(v.z, -3.0)

    def test_sub_scalar(self):
        v1 = ik.Vec3(1, 2, 3)
        v2 = v1 - 5
        self.assertIsNot(v2, v1)
        self.assertEqual(v1.x, 1.0)
        self.assertEqual(v1.y, 2.0)
        self.assertEqual(v1.z, 3.0)
        self.assertEqual(v2.x, -4.0)
        self.assertEqual(v2.y, -3.0)
        self.assertEqual(v2.z, -2.0)

    def test_sub_scalar_inplace(self):
        v = ik.Vec3(1, 2, 3)
        v -= 5
        self.assertEqual(v.x, -4.0)
        self.assertEqual(v.y, -3.0)
        self.assertEqual(v.z, -2.0)

    def test_sub_scalar_meth(self):
        v = ik.Vec3(1, 2, 3)
        v.sub(5)
        self.assertEqual(v.x, -4.0)
        self.assertEqual(v.y, -3.0)
        self.assertEqual(v.z, -2.0)

    def test_mul_vec(self):
        v1 = ik.Vec3(1, 2, 3)
        v2 = ik.Vec3(4, 5, 6)
        v3 = v1 * v2
        self.assertIsNot(v3, v1)
        self.assertIsNot(v3, v2)
        self.assertEqual(v1.x, 1.0)
        self.assertEqual(v1.y, 2.0)
        self.assertEqual(v1.z, 3.0)
        self.assertEqual(v2.x, 4.0)
        self.assertEqual(v2.y, 5.0)
        self.assertEqual(v2.z, 6.0)
        self.assertEqual(v3.x, 4.0)
        self.assertEqual(v3.y, 10.0)
        self.assertEqual(v3.z, 18.0)

    def test_mul_vec_inplace(self):
        v = ik.Vec3(1, 2, 3)
        v *= ik.Vec3(4, 5, 6)
        self.assertEqual(v.x, 4.0)
        self.assertEqual(v.y, 10.0)
        self.assertEqual(v.z, 18.0)

    def test_mul_vec_meth(self):
        v = ik.Vec3(1, 2, 3)
        v.mul(ik.Vec3(4, 5, 6))
        self.assertEqual(v.x, 4.0)
        self.assertEqual(v.y, 10.0)
        self.assertEqual(v.z, 18.0)

    def test_mul_scalar(self):
        v1 = ik.Vec3(1, 2, 3)
        v2 = v1 * 5
        self.assertIsNot(v2, v1)
        self.assertEqual(v1.x, 1.0)
        self.assertEqual(v1.y, 2.0)
        self.assertEqual(v1.z, 3.0)
        self.assertEqual(v2.x, 5.0)
        self.assertEqual(v2.y, 10.0)
        self.assertEqual(v2.z, 15.0)

    def test_mul_scalar_inplace(self):
        v = ik.Vec3(1, 2, 3)
        v *= 5
        self.assertEqual(v.x, 5.0)
        self.assertEqual(v.y, 10.0)
        self.assertEqual(v.z, 15.0)

    def test_mul_scalar_meth(self):
        v = ik.Vec3(1, 2, 3)
        v.mul(5)
        self.assertEqual(v.x, 5.0)
        self.assertEqual(v.y, 10.0)
        self.assertEqual(v.z, 15.0)

    def test_div_vec(self):
        v1 = ik.Vec3(1, 2, 3)
        v2 = ik.Vec3(4, 5, 6)
        v3 = v1 / v2
        self.assertIsNot(v3, v1)
        self.assertIsNot(v3, v2)
        self.assertEqual(v1.x, 1.0)
        self.assertEqual(v1.y, 2.0)
        self.assertEqual(v1.z, 3.0)
        self.assertEqual(v2.x, 4.0)
        self.assertEqual(v2.y, 5.0)
        self.assertEqual(v2.z, 6.0)
        self.assertFloatEqual(v3.x, 0.25)
        self.assertFloatEqual(v3.y, 0.4)
        self.assertFloatEqual(v3.z, 0.5)

    def test_div_vec_inplace(self):
        v = ik.Vec3(1, 2, 3)
        v /= ik.Vec3(4, 5, 6)
        self.assertFloatEqual(v.x, 0.25)
        self.assertFloatEqual(v.y, 0.4)
        self.assertFloatEqual(v.z, 0.5)

    def test_div_vec_meth(self):
        v = ik.Vec3(1, 2, 3)
        v.div(ik.Vec3(4, 5, 6))
        self.assertFloatEqual(v.x, 0.25)
        self.assertFloatEqual(v.y, 0.4)
        self.assertFloatEqual(v.z, 0.5)

    def test_div_scalar(self):
        v1 = ik.Vec3(1, 2, 3)
        v2 = v1 / 5
        self.assertIsNot(v2, v1)
        self.assertEqual(v1.x, 1.0)
        self.assertEqual(v1.y, 2.0)
        self.assertEqual(v1.z, 3.0)
        self.assertFloatEqual(v2.x, 0.2)
        self.assertFloatEqual(v2.y, 0.4)
        self.assertFloatEqual(v2.z, 0.6)

    def test_div_scalar_inplace(self):
        v = ik.Vec3(1, 2, 3)
        v /= 5
        self.assertFloatEqual(v.x, 0.2)
        self.assertFloatEqual(v.y, 0.4)
        self.assertFloatEqual(v.z, 0.6)

    def test_div_scalar_meth(self):
        v = ik.Vec3(1, 2, 3)
        v.div(5)
        self.assertFloatEqual(v.x, 0.2)
        self.assertFloatEqual(v.y, 0.4)
        self.assertFloatEqual(v.z, 0.6)

    def test_length_squared(self):
        v = ik.Vec3(1, 2, 3)
        l = v.length_squared()
        self.assertFloatEqual(l, 14.0)

    def test_length(self):
        v = ik.Vec3(1, 2, 3)
        l = v.length()
        self.assertFloatEqual(l, sqrt(14))

    def test_dot(self):
        v1 = ik.Vec3(1, 2, 3)
        v2 = ik.Vec3(4, 5, 6)
        l = v1.dot(v2)
        self.assertFloatEqual(l, 32.0)

    def test_normalize(self):
        v = ik.Vec3(1, 2, 3)
        v.normalize()
        self.assertFloatEqual(v.x, 1/sqrt(14))
        self.assertFloatEqual(v.y, 2/sqrt(14))
        self.assertFloatEqual(v.z, 3/sqrt(14))

    def test_normalized(self):
        v1 = ik.Vec3(1, 2, 3)
        v2 = v1.normalized()
        self.assertIsNot(v1, v2)
        self.assertEqual(v1.x, 1.0)
        self.assertEqual(v1.y, 2.0)
        self.assertEqual(v1.z, 3.0)
        self.assertFloatEqual(v2.x, 1/sqrt(14))
        self.assertFloatEqual(v2.y, 2/sqrt(14))
        self.assertFloatEqual(v2.z, 3/sqrt(14))

    def test_cross(self):
        v1 = ik.Vec3(1, 2, 3)
        v2 = ik.Vec3(4, 5, 6)
        v1.cross(v2)
        self.assertIsNot(v1, v2)
        self.assertEqual(v1.x, -3.0)
        self.assertEqual(v1.y, 6.0)
        self.assertEqual(v1.z, -3.0)
        self.assertEqual(v2.x, 4.0)
        self.assertEqual(v2.y, 5.0)
        self.assertEqual(v2.z, 6.0)

    def test_crossed(self):
        v1 = ik.Vec3(1, 2, 3)
        v2 = ik.Vec3(4, 5, 6)
        v3 = v1.crossed(v2)
        self.assertIsNot(v3, v1)
        self.assertIsNot(v3, v2)
        self.assertEqual(v1.x, 1.0)
        self.assertEqual(v1.y, 2.0)
        self.assertEqual(v1.z, 3.0)
        self.assertEqual(v2.x, 4.0)
        self.assertEqual(v2.y, 5.0)
        self.assertEqual(v2.z, 6.0)
        self.assertFloatEqual(v3.x, -3.0)
        self.assertFloatEqual(v3.y, 6.0)
        self.assertFloatEqual(v3.z, -3.0)

    def test_rotate_meth(self):
        v = ik.Vec3(1, 0, 0)
        q = ik.Quat((0, 0, 1), pi/4)
        v.rotate(q)
        self.assertFloatEqual(v.x, 1.0/sqrt(2))
        self.assertFloatEqual(v.y, 1.0/sqrt(2))
        self.assertFloatEqual(v.z, 0.0)

    def test_rotated(self):
        v1 = ik.Vec3(1, 0, 0)
        q = ik.Quat((0, 0, 1), pi/4)
        v2 = v1.rotated(q)
        self.assertIsNot(v2, v1)
        self.assertEqual(v1.x, 1.0)
        self.assertEqual(v1.y, 0.0)
        self.assertEqual(v1.z, 0.0)
        self.assertFloatEqual(v2.x, 1.0/sqrt(2))
        self.assertFloatEqual(v2.y, 1.0/sqrt(2))
        self.assertFloatEqual(v2.z, 0.0)

    def test_mul_rotate(self):
        v1 = ik.Vec3(1, 0, 0)
        q = ik.Quat((0, 0, 1), pi/4)
        v2 = v1 * q
        self.assertIsNot(v2, v1)
        self.assertEqual(v1.x, 1.0)
        self.assertEqual(v1.y, 0.0)
        self.assertEqual(v1.z, 0.0)
        self.assertFloatEqual(v2.x, 1.0/sqrt(2))
        self.assertFloatEqual(v2.y, 1.0/sqrt(2))
        self.assertFloatEqual(v2.z, 0.0)

    def test_mul_rotate_inplace(self):
        v = ik.Vec3(1, 0, 0)
        q = ik.Quat((0, 0, 1), pi/4)
        v *= q
        self.assertFloatEqual(v.x, 1.0/sqrt(2))
        self.assertFloatEqual(v.y, 1.0/sqrt(2))
        self.assertFloatEqual(v.z, 0.0)
