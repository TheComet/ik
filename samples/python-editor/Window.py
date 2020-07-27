__author__ = "TheComet"

import pygame
import time
import ik
import random
from Updateable import Updateable
from TreeEditor import TreeEditor
from Tree import Tree
from math import pi


def one_bone_example(pos):
    root = ik.Node(position=ik.Vec3(0, pos[0], pos[1]), rotation=ik.Quat((1, 0, 0), pi))
    tip = root.create_child(position=ik.Vec3(0, 0, 50))

    root.algorithm = ik.Algorithm(ik.ONE_BONE)
    tip.effector = ik.Effector(target_position=ik.Vec3(0, pos[0], pos[1] - 50), target_rotation=ik.Quat((1, 0, 0), pi))
    return root


def two_bone_example(pos):
    root = ik.Node(position=ik.Vec3(0, pos[0], pos[1]), rotation=ik.Quat((1, 0, 0), pi))
    mid = root.create_child(position=ik.Vec3(0, 0, 50))
    tip = mid.create_child(position=ik.Vec3(0, 0, 50))

    root.algorithm = ik.Algorithm(ik.TWO_BONE)
    tip.effector = ik.Effector(target_position=ik.Vec3(0, pos[0], pos[1] - 100), target_rotation=ik.Quat((1, 0, 0), pi))
    return root

def long_chain_example(pos, chain_len):
    tip = root = ik.Node(position=ik.Vec3(0, pos[0], pos[1]), rotation=ik.Quat((1, 0, 0), pi))
    for i in range(chain_len):
        tip = tip.create_child(position=ik.Vec3(0, 0, 20*(random.random()+0.5)))

    root.algorithm = ik.Algorithm(ik.FABRIK)
    tip.effector = ik.Effector(target_position=ik.Vec3(0, pos[0], pos[1] - 20*chain_len), target_rotation=ik.Quat((1, 0, 0), pi))
    return root

def double_effectors_example(pos, chain_len):
    mid1 = root = ik.Node(position=ik.Vec3(0, pos[0], pos[1]), rotation=ik.Quat((1, 0, 0), pi))
    for i in range(chain_len):
        mid1 = mid1.create_child(position=ik.Vec3(0, 0, 50))

    tip1 = mid1
    for i in range(chain_len):
        tip1 = tip1.create_child(position=ik.Vec3(0, 0, 50))

    tip2 = mid1
    for i in range(chain_len):
        tip2 = tip2.create_child(position=ik.Vec3(0, 0, 50))

    root.algorithm = ik.Algorithm(ik.FABRIK, max_iterations=50)
    tip1.effector = ik.Effector(target_position=ik.Vec3(0, pos[0]-100, pos[1]-300))
    tip2.effector = ik.Effector(target_position=ik.Vec3(0, pos[0]+100, pos[1]-300))

    return root

def multiple_effectors_example(pos, chain_len):
    mid1 = root = ik.Node(position=ik.Vec3(0, pos[0], pos[1]), rotation=ik.Quat((1, 0, 0), pi))
    for i in range(chain_len):
        mid1 = mid1.create_child(position=ik.Vec3(0, 0, 50))

    mid2 = mid1
    for i in range(chain_len):
        mid2 = mid2.create_child(position=ik.Vec3(0, 0, 50))

    tip1 = mid1
    for i in range(chain_len):
        tip1 = tip1.create_child(position=ik.Vec3(0, 0, 50))

    tip2 = mid2
    for i in range(chain_len):
        tip2 = tip2.create_child(position=ik.Vec3(0, 0, 50))

    tip3 = mid2
    for i in range(chain_len):
        tip3 = tip3.create_child(position=ik.Vec3(0, 0, 50))

    root.algorithm = ik.Algorithm(ik.FABRIK, max_iterations=50)
    tip1.effector = ik.Effector(target_position=ik.Vec3(0, pos[0]-100, pos[1]-300))
    tip2.effector = ik.Effector(target_position=ik.Vec3(0, pos[0]+100, pos[1]-300))
    tip3.effector = ik.Effector(target_position=ik.Vec3(0, pos[0]+200, pos[1]-300))

    return root

class Window(Updateable):
    def __init__(self, width, height):
        self.dimensions = width, height
        self.screen = pygame.display.set_mode(self.dimensions)

        self.__updateables = [
            self,
            Tree(one_bone_example((100, height - 200))),
            Tree(two_bone_example((300, height - 200))),
            Tree(long_chain_example((500, height - 200), 24)),
            Tree(double_effectors_example((700, height - 200), 1))
            #Tree(multiple_effectors_example((900, height - 200), 4))
        ]

        self.__last_time_updated = None
        self.__running = False

    def enter_main_loop(self):
        self.__last_time_updated = time.time()
        self.__running = True
        while self.__running:
            self.__process_events()
            self.__update()
            self.__draw()

    def __process_events(self):
        for event in pygame.event.get():
            for updatedable in self.__updateables:
                if updatedable.process_event(event):
                    break

    def __update(self):
        dt = self.__update_timestep()
        for updateable in self.__updateables:
            updateable.update(dt)

    def __update_timestep(self):
        new_time = time.time()
        dt = new_time - self.__last_time_updated
        self.__last_time_updated = new_time
        return dt

    def __draw(self):
        self.screen.fill((0, 0, 0))

        for updateable in self.__updateables:
            updateable.draw(self.screen)

        pygame.display.flip()

    def process_event(self, event):
        if event.type == pygame.QUIT:
            self.__running = False
