import ik
import unittest

class TestBone(unittest.TestCase):
    def test_default_construct(self):
        n = ik.Bone()

        self.assertEqual(n.position.x, 0.0)
        self.assertEqual(n.position.y, 0.0)
        self.assertEqual(n.position.z, 0.0)

        self.assertEqual(n.rotation.x, 0.0)
        self.assertEqual(n.rotation.y, 0.0)
        self.assertEqual(n.rotation.z, 0.0)
        self.assertEqual(n.rotation.w, 1.0)

        self.assertEqual(n.length, 0.0)

    def test_construct_with_position(self):
        n = ik.Bone(position=ik.Vec3(1, 2, 3))
        self.assertEqual(n.position.x, 1.0)
        self.assertEqual(n.position.y, 2.0)
        self.assertEqual(n.position.z, 3.0)

    def test_construct_with_rotation(self):
        n = ik.Bone(rotation=ik.Quat(1, 2, 3, 4))
        self.assertEqual(n.rotation.x, 1.0)
        self.assertEqual(n.rotation.y, 2.0)
        self.assertEqual(n.rotation.z, 3.0)
        self.assertEqual(n.rotation.w, 4.0)

    def test_construct_with_length(self):
        n = ik.Bone(length=2.5)
        self.assertEqual(n.length, 2.5)

    def test_construct_with_effector(self):
        n = ik.Bone(effector=ik.Effector())

    def test_repr(self):
        n1 = ik.Bone()
        n2 = n1.create_child()
        n3 = n2.create_child()
        s = repr(n1)
