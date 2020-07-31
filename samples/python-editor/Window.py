__author__ = "TheComet"

import pygame
import ik
import random
from Updateable import Updateable
from TreeEditor import TreeEditor
from Tree import Tree
from math import pi, sqrt
from time import time


def one_bone_example(pos):
    root = ik.Node(position=ik.Vec3(0, pos[0], pos[1]), rotation=ik.Quat((1, 0, 0), pi))
    tip = root.create_child(position=ik.Vec3(0, 0, 50))

    root.algorithm = ik.Algorithm(ik.ONE_BONE, constraints=True)
    tip.effector = ik.Effector(target_position=ik.Vec3(0, pos[0], pos[1] - 50), target_rotation=ik.Quat((1, 0, 0), pi))
    tip.constraints = ik.HingeConstraint(axis=ik.Vec3(1, 0, 0), min_angle=-pi/4, max_angle=pi/4)
    return root


def two_bone_example(pos):
    root = ik.Node(position=ik.Vec3(0, pos[0], pos[1]), rotation=ik.Quat((1, 0, 0), pi))
    mid = root.create_child(position=ik.Vec3(0, 0, 50))
    tip = mid.create_child(position=ik.Vec3(0, 0, 50))

    root.algorithm = ik.Algorithm(ik.FABRIK)
    tip.effector = ik.Effector(target_position=ik.Vec3(0, pos[0], pos[1] - 100), target_rotation=ik.Quat((1, 0, 0), pi))
    return root

def long_chain_example(pos, chain_len, segment_len):
    tip = root = ik.Node(position=ik.Vec3(0, pos[0], pos[1]), rotation=ik.Quat((1, 0, 0), pi))
    for i in range(chain_len):
        tip = tip.create_child(position=ik.Vec3(0, 0, segment_len))

    root.children[0].children[0].children[0].constraints = ik.StiffConstraint()

    root.algorithm = ik.Algorithm(ik.FABRIK, max_iterations=20, constraints=True)
    tip.effector = ik.Effector(target_position=ik.Vec3(0, pos[0], pos[1] - segment_len*chain_len), target_rotation=ik.Quat((1, 0, 0), pi))
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
    tip1.effector = ik.Effector(target_position=ik.Vec3(0, pos[0]-chain_len*12, pos[1]-chain_len*50*2))
    tip2.effector = ik.Effector(target_position=ik.Vec3(0, pos[0]+chain_len*12, pos[1]-chain_len*50*2))

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
    tip1.effector = ik.Effector(target_position=ik.Vec3(0, pos[0]-100, pos[1]-chain_len*50*3))
    tip2.effector = ik.Effector(target_position=ik.Vec3(0, pos[0]+100, pos[1]-chain_len*50*3))
    tip3.effector = ik.Effector(target_position=ik.Vec3(0, pos[0]+200, pos[1]-chain_len*50*3))

    return root

def too_many_effectors_example(pos, chain_len, segment_len, max_depth):
    root = ik.Node(position=ik.Vec3(0, pos[0], pos[1]), rotation=ik.Quat((1, 0, 0), pi))
    def branchoff(node, chain_len, depth, x):
        if depth == 0:
            dist = max_depth*chain_len*segment_len
            y = sqrt(dist**2 - x**2)
            node.effector = ik.Effector(target_position=ik.Vec3(0, pos[0]+x, pos[1]-y))
            return
        for i in range(chain_len):
            node = node.create_child(position=ik.Vec3(0, 0, segment_len))
        branchoff(node, chain_len, depth-1, x-depth**2*segment_len/12)
        #branchoff(node, chain_len, depth-1, x)
        branchoff(node, chain_len, depth-1, x+depth**2*segment_len/12)
    tstart = time()
    branchoff(root, chain_len, max_depth, 0)
    print(f"tree took {time()-tstart} to build")
    root.algorithm = ik.Algorithm(ik.FABRIK)

    return root

class Window(Updateable):
    def __init__(self, width, height):
        self.dimensions = width, height
        self.screen = pygame.display.set_mode(self.dimensions)

        self.__updateables = [
            self,
            #Tree(one_bone_example((100, height - 200))),
            #Tree(two_bone_example((300, height - 200))),
            Tree(long_chain_example((width/2, height - 200), 4, 80))
            #Tree(double_effectors_example((700, height - 200), 3)),
            #Tree(multiple_effectors_example((900, height - 200), 4))
            #Tree(too_many_effectors_example((width/2, height-100), 8, 8, 11))
        ]

        self.__last_time_updated = None
        self.__running = False

    def enter_main_loop(self):
        self.__last_time_updated = time()
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
        new_time = time()
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
