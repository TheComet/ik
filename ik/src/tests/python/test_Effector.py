import ik
import unittest

class TestEffector(unittest.TestCase):
    def test_construct_with_chain_length(self):
        e = ik.Effector(chain_length=2)

