__author__ = "TheComet"

import pygame
import ik
import random
from Updateable import Updateable
from Tree import Tree
from math import pi, sqrt
from time import time


def one_bone_example(pos):
    bone = ik.Bone(position=ik.Vec3(0, pos[0], pos[1]), length=0.3)
    bone.algorithm = ik.Algorithm(ik.ONE_BONE, constraints=True)
    bone.effector = ik.Effector(target_position=ik.Vec3(0, pos[0], pos[1]+0.4))
    bone.constraints = ik.HingeConstraint(axis=ik.Vec3(1, 0, 0), min_angle=-pi/4, max_angle=pi/4)
    return bone


def two_bone_example1(pos):
    base = ik.Bone(position=ik.Vec3(0, pos[0], pos[1]), length=0.18)
    tip = base.create_child(position=ik.Vec3(0, 0, 0), length=0.24)

    base.constraints = ik.StiffConstraint(rotation=ik.Quat((1, 0, 0), pi/5))

    base.algorithm = ik.Algorithm(ik.TWO_BONE, max_iterations=20, constraints=True)
    tip.effector = ik.Effector(target_position=ik.Vec3(0, pos[0], pos[1]+sqrt(0.1)))
    return base

def two_bone_example2(pos):
    base = ik.Bone(position=ik.Vec3(0, pos[0], pos[1]), length=0.18)
    tip = base.create_child(position=ik.Vec3(0, 0.1, 0.1), length=0.24)

    base.algorithm = ik.Algorithm(ik.TWO_BONE)
    tip.effector = ik.Effector(target_position=ik.Vec3(0, pos[0], pos[1]+sqrt(0.1)))
    return base

def two_bone_example3(pos):
    base1 = ik.Bone(position=ik.Vec3(0, pos[0], pos[1]), length=0.18)
    tip1 = base1.create_child(position=ik.Vec3(0, 0.1, 0.1), length=0.24)
    tip1.effector = ik.Effector(target_position=ik.Vec3(0, pos[0], pos[1]+sqrt(0.1)))

    base2 = tip1.create_child(position=ik.Vec3(0, 0.1, 0), length=0.18)
    tip2 = base2.create_child(position=ik.Vec3(0, 0.1, 0.1), length=0.24)
    tip2.effector = ik.Effector(target_position=ik.Vec3(0, pos[0], pos[1]+sqrt(0.1)))

    base3 = tip1.create_child(position=ik.Vec3(0, 0.1, 0), length=0.18)
    tip3 = base3.create_child(position=ik.Vec3(0, 0.1, 0.1), length=0.24)
    tip3.effector = ik.Effector(target_position=ik.Vec3(0, pos[0], pos[1]+sqrt(0.1)))

    base1.algorithm = ik.Algorithm(ik.TWO_BONE)
    return base1


def long_chain_example(pos, bone_num, bone_len):
    tip = root = ik.Bone(position=ik.Vec3(0, pos[0], pos[1]), length=bone_len)
    for i in range(bone_num-1):
        tip = tip.create_child(length=bone_len, position=ik.Vec3(0, 0.05, 0.05))
        #tip = tip.create_child(length=bone_len)

    root.children[0].constraints = ik.StiffConstraint(rotation=ik.Quat((1, 0, 0), pi/5))

    root.algorithm = ik.Algorithm(ik.FABRIK, max_iterations=50)
    tip.effector = ik.Effector(target_position=ik.Vec3(0, pos[0], pos[1]+bone_num*bone_len))
    return root


def double_effectors_example(pos, chain_len):
    mid1 = root = ik.Bone(position=ik.Vec3(0, pos[0], pos[1]), length=0.2)
    for i in range(chain_len-1):
        mid1 = mid1.create_child(length=0.15)

    tip1 = mid1
    for i in range(chain_len):
        #tip1 = tip1.create_child(length=0.15, position=ik.Vec3(0, 0.1, 0.1))
        tip1 = tip1.create_child(length=0.15)

    tip2 = mid1
    for i in range(chain_len):
        tip2 = tip2.create_child(length=0.15)

    for child in mid1.children:
        child.constraints = ik.StiffConstraint(rotation=ik.Quat((1, 0, 0), 0))

    root.algorithm = ik.Algorithm(ik.FABRIK, max_iterations=40, constraints=True)
    tip1.effector = ik.Effector(target_position=ik.Vec3(0, pos[0]+.4, pos[1]+.2))
    tip2.effector = ik.Effector(target_position=ik.Vec3(0, pos[0]-.2, pos[1]+.2))

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


def combined_solvers(pos, segment_len):
    root = ik.Node(position=ik.Vec3(0, pos[0], pos[1]), rotation=ik.Quat((1, 0, 0), pi))
    #mid = root.create_child(position=ik.Vec3(0, 0, segment_len))
    mid = root
    mid1 = mid.create_child(position=ik.Vec3(0, segment_len, segment_len), rotation=ik.Quat((1, 0, 0), pi/8))
    mid2 = mid.create_child(position=ik.Vec3(0, 0, segment_len), rotation=ik.Quat((1, 0, 0), pi/8))
    mid3 = mid.create_child(position=ik.Vec3(0, -segment_len, segment_len), rotation=ik.Quat((1, 0, 0), pi/8))
    tip1 = mid1.create_child(position=ik.Vec3(0, 0, segment_len), rotation=ik.Quat((1, 0, 0), pi/8))
    tip2 = mid2.create_child(position=ik.Vec3(0, 0, segment_len), rotation=ik.Quat((1, 0, 0), pi/8))
    tip3 = mid3.create_child(position=ik.Vec3(0, 0, segment_len), rotation=ik.Quat((1, 0, 0), pi/8))

    root.algorithm = ik.Algorithm(ik.FABRIK)
    mid1.effector = ik.Effector(target_position=ik.Vec3(0, pos[0]-segment_len, pos[1]-segment_len), chain_length=0)
    mid3.effector = ik.Effector(target_position=ik.Vec3(0, pos[0]+segment_len, pos[1]-segment_len), chain_length=0)

    return root


def embedded_effectors(pos):
    def random_pos():
        return ik.Vec3(0, random.random()*0.0, random.random()*0.0)
    root = ik.Bone(length=0.1, position=ik.Vec3(0, pos[0], pos[1]))
    b1 = root.create_child(length=0.1, position=random_pos())
    b2 = b1.create_child(length=0.1, position=random_pos())
    b3 = b2.create_child(length=0.1, position=random_pos())
    b4 = b3.create_child(length=0.1, position=random_pos())
    b5 = b4.create_child(length=0.1, position=random_pos())
    b6 = b5.create_child(length=0.1, position=random_pos())
    tip = b6.create_child(length=0.1, position=random_pos())

    root.algorithm = ik.Algorithm(ik.FABRIK)
    root.effector = ik.Effector(target_position=ik.Vec3(0, pos[0], pos[1]+0.1))

    b1.algorithm = ik.Algorithm(ik.FABRIK)
    b2.effector = ik.Effector(target_position=ik.Vec3(0, pos[0], pos[1]+0.3))

    b3.algorithm = ik.Algorithm(ik.FABRIK)
    b5.effector = ik.Effector(target_position=ik.Vec3(0, pos[0], pos[1]+0.6))

    b6.algorithm = ik.Algorithm(ik.FABRIK)
    tip.effector = ik.Effector(target_position=ik.Vec3(0, pos[0], pos[1]+0.8))

    return root



def double_embedded_effectors(pos, chain_len):
    mid1 = root = ik.Node(position=ik.Vec3(0, pos[0], pos[1]), rotation=ik.Quat((1, 0, 0), pi))
    for i in range(chain_len):
        mid1 = mid1.create_child(position=ik.Vec3(0, 0, 50))

    tip1 = mid1
    for i in range(chain_len):
        tip1 = tip1.create_child(position=ik.Vec3(0, 0, 50))

    tip2 = mid1
    for i in range(chain_len):
        tip2 = tip2.create_child(position=ik.Vec3(0, 0, 50))

    mid1.create_child(position=ik.Vec3(0, 0, 50))

    root.algorithm = ik.Algorithm(ik.FABRIK, max_iterations=20)
    mid1.parent.effector = ik.Effector(target_position=ik.Vec3(0, pos[0], pos[1]-chain_len*50+50))

    mid1.algorithm = ik.Algorithm(ik.FABRIK)
    tip1.effector = ik.Effector(target_position=ik.Vec3(0, pos[0]-chain_len*12, pos[1]-chain_len*50*2))
    tip2.effector = ik.Effector(target_position=ik.Vec3(0, pos[0]+chain_len*12, pos[1]-chain_len*50*2))

    return root


def human_example(pos):
    root = ik.Node(position=ik.Vec3(0, pos[0], pos[1]), rotation=ik.Quat((1, 0, 0), pi))
    pelvis = root.create_child(position=ik.Vec3(0, 0, 100))

    hip_r = pelvis.create_child(position=ik.Vec3(0, -20, 0), rotation=ik.Quat((1, 0, 0), pi))
    knee_r = hip_r.create_child(position=ik.Vec3(0, 0, 50))
    foot_r = knee_r.create_child(position=ik.Vec3(0, 0, 50), rotation=ik.Quat((1, 0, 0), -pi/5))
    toe_r = foot_r.create_child(position=ik.Vec3(0, 0, 20))

    hip_l = pelvis.create_child(position=ik.Vec3(0, 20, 0), rotation=ik.Quat((1, 0, 0), -pi))
    knee_l = hip_l.create_child(position=ik.Vec3(0, 0, 50))
    foot_l = knee_l.create_child(position=ik.Vec3(0, 0, 50), rotation=ik.Quat((1, 0, 0), pi/5))
    toe_l = foot_l.create_child(position=ik.Vec3(0, 0, 20))

    toe_l.effector = ik.Effector(target_position=ik.Vec3(0, pos[0]-40, pos[1]+5), chain_length=4)
    toe_r.effector = ik.Effector(target_position=ik.Vec3(0, pos[0]+40, pos[1]+5), chain_length=4)

    spine1 = pelvis.create_child(position=ik.Vec3(0, 0, 30))
    spine2 = spine1.create_child(position=ik.Vec3(0, 0, 30))
    spine3 = spine2.create_child(position=ik.Vec3(0, 0, 30))

    shoulder_r = spine3.create_child(position=ik.Vec3(0, -20, 0), rotation=ik.Quat((1, 0, 0), pi-pi/8))
    elbow_r = shoulder_r.create_child(position=ik.Vec3(0, 0, 50))
    hand_r = elbow_r.create_child(position=ik.Vec3(0, 0, 50))

    shoulder_l = spine3.create_child(position=ik.Vec3(0, 20, 0), rotation=ik.Quat((1, 0, 0), -pi+pi/8))
    elbow_l = shoulder_l.create_child(position=ik.Vec3(0, 0, 50))
    hand_l = elbow_l.create_child(position=ik.Vec3(0, 0, 50))

    hand_r.effector = ik.Effector(target_position=ik.Vec3(0, pos[0]+40, pos[1]-100), chain_length=5)
    hand_l.effector = ik.Effector(target_position=ik.Vec3(0, pos[0]-40, pos[1]-100), chain_length=5)

    neck = spine3.create_child(position=ik.Vec3(0, 0, 20))

    root.algorithm = ik.Algorithm(ik.FABRIK)
    return root


class Window(Updateable):
    def __init__(self, width, height):
        self.dimensions = width, height
        self.screen = pygame.display.set_mode(self.dimensions)

        self.__updateables = [
            self,
            #Tree(one_bone_example((-0.5, -0.5))),
            #Tree(two_bone_example1((-0.25, -0.5))),
            #Tree(two_bone_example2((0.25, -0.5))),
            #Tree(two_bone_example3((0.5, -0.5))),
            #Tree(long_chain_example((0, 0), 3, 0.15)),
            Tree(double_effectors_example((0, 0), 2)),
            #Tree(multiple_effectors_example((900, height - 200), 4))
            #Tree(too_many_effectors_example((width/2, height-100), 8, 8, 11))
            #Tree(combined_solvers((width/2, height-200), 80))
            #Tree(human_example((width/2, height-200)))
            #Tree(embedded_effectors((-0.8, -0.8)))
            #Tree(double_embedded_effectors((width/2, height-200), 3))
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
