import ik
import unittest

class TestNode(unittest.TestCase):
    def test_default_construct(self):
        n = ik.Node()
        self.assertIsNone(n.algorithm)
        self.assertIsNone(n.constraint)
        self.assertIsNone(n.effector)
        self.assertIsNone(n.pole)

    def test_create_child(self):
        n1 = ik.Node()
        n2 = n1.create_child()
