import ik
import unittest

class TestPose(unittest.TestCase):
    def test_save_pose(self):
        n1 = ik.Node(position=ik.Vec3(1, 2, 3), rotation=ik.Quat(1, 2, 3, 4))
        n2 = n1.create_child(position=ik.Vec3(4, 5, 6), rotation=ik.Quat(4, 5, 6, 6))
        n3 = n2.create_child(position=ik.Vec3(7, 8, 9), rotation=ik.Quat(7, 8, 9, 9))

        p = ik.Pose(n1)
