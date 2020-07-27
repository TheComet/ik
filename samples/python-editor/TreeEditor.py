__author__ = "TheComet"

import ik
import pygame
from Updateable import Updateable


class TreeEditor(Updateable):
    def __init__(self):
        self.root = ik.Node(position=ik.Vec3(0, 1280/2, 940))
        self.root_color = (255, 100, 255)
        self.node_color = (100, 100, 255)

    def process_event(self, event):
        pass

    def update(self, time_step):
        pass

    def draw(self, surface):
        self.__draw_tree(surface, self.root, ik.Vec3(), ik.Quat())

    def __draw_tree(self, surface, node, acc_pos, acc_rot):
        # Root node color is different because it can't be deleted
        color = self.root_color if node is self.root else self.node_color

        # ik lib uses 3D coordinates; we're storing our 2D coordinates in the Y and Z components.
        pygame.draw.circle(surface, color, (int(node.position.y), int(node.position.z)), 5, 1)
