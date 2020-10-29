__author__ = "TheComet"

import ik
import pygame
from math import pi
from Updateable import Updateable
from time import time


def transform_points(points, bone):
    q = bone.rotation
    cos_a = 1 - 2 * (q.x * q.x + q.y * q.y)
    sin_a = 2 * (q.w * q.x + q.y * q.z)
    s = bone.length
    tx = bone.position.y
    ty = bone.position.z

    for point in points:
        x = point[0]
        y = point[1]
        x, y = s*x, s*y
        x, y = x*cos_a - y*sin_a, x*sin_a + y*cos_a
        x, y = x+tx, y+ty
        yield x, y


def transform_to_screen(point):
    w, h = pygame.display.get_surface().get_size()
    return (
        int((point[0] + 1) * w * 0.5+0.5),
        int((1 - point[1]) * h * 0.5+0.5)
    )


def transform_from_screen(point):
    w, h = pygame.display.get_surface().get_size()
    return (
        (point[0] - 0.5) / 0.5 / w - 1,
        (0.5 - point[1]) / 0.5 / h + 1
    )


def draw_closed_shape(surface, points_list, color):
    for i, point in enumerate(points_list):
        start = transform_to_screen(points_list[i-1])
        end = transform_to_screen(point)
        pygame.draw.line(surface, color, start, end, 1)


def draw_tree(surface, bone):
    diamond_points = list(transform_points((
        (0, 0),
        (-0.1, 0.16),
        (0, 1),
        (0.1, 0.16)
    ), bone))
    base_pos = transform_to_screen(diamond_points[0])
    tip_pos = transform_to_screen(diamond_points[2])

    draw_closed_shape(surface, diamond_points, (100, 100, 255))
    pygame.draw.circle(surface, (100, 100, 255), base_pos, 4, 1)
    pygame.draw.circle(surface, (100, 100, 255), tip_pos, 4, 1)


def draw_effectors(surface, effectors):
    for e in effectors:
        c = (200, 255, 0)
        off = ik.Vec3(0, 0, 1) * e.target_rotation * 0.04
        start = transform_to_screen((e.target_position.y, e.target_position.z))
        end = transform_to_screen((e.target_position.y + off.y, e.target_position.z + off.z))
        pygame.draw.circle(surface, c, start, 5, 1)
        pygame.draw.line(surface, c, start, end, 1)


class Tree(Updateable):
    def __init__(self, root):
        self.root = root
        #self.initial_pose = ik.Pose(root)

        font = pygame.font.SysFont(None, 32)

        tstart = time()
        self.solver = ik.Solver(root)
        self.build_img = font.render(f"build() took {time()-tstart}", True, (255, 255, 255))

        self.root_color     = (255, 100, 255)
        self.bone_color     = (100, 100, 255)
        self.effector_color = (200, 255, 0)

        self.grabbed_effector = None
        self.grabbed_root_bone = None

        def get_effectors(bone):
            for child in bone.children:
                yield from get_effectors(child)
            if bone.effector is not None:
                yield bone.effector
        self.effectors = list(get_effectors(root))

        self.bones_img = font.render(f"bones: {self.root.count}", True, (255, 255, 255))
        self.effectors_img = font.render(f"end effectors: {len(self.effectors)}", True, (255, 255, 255))

    def process_event(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN:
            pos = transform_from_screen(event.pos)
            self.grabbed_effector = self.__find_grabbable_effector(pos)
            if self.grabbed_effector is not None:
                self.grabbed_effector.target_position.y = pos[0]
                self.grabbed_effector.target_position.z = pos[1]
                return True

            if (self.root.position.y - pos[0])**2 + (self.root.position.z - pos[1])**2 < 0.001:
                self.grabbed_root_bone = self.root
                self.grabbed_root_bone.position.y = pos[0]
                self.grabbed_root_bone.position.z = pos[1]
                return True

        if event.type == pygame.MOUSEBUTTONUP:
            self.grabbed_effector = None
            self.grabbed_root_bone = None
        if event.type == pygame.MOUSEMOTION:
            pos = transform_from_screen(event.pos)
            if self.grabbed_effector is not None:
                self.grabbed_effector.target_position.y = pos[0]
                self.grabbed_effector.target_position.z = pos[1]
            if self.grabbed_root_bone is not None:
                self.grabbed_root_bone.position.y = pos[0]
                self.grabbed_root_bone.position.z = pos[1]

    def __find_grabbable_effector(self, pos):
        for e in self.effectors:
            if (e.target_position.y-pos[0])**2 + (e.target_position.z-pos[1])**2 < 0.001:
                return e

    def update(self, time_step):
        #self.initial_pose.apply(self.root)
        tstart = time()
        self.solver.solve()

    def draw(self, surface):
        draw_tree(surface, self.root)
        draw_effectors(surface, self.effectors)
        surface.blit(self.build_img, (10, 10))
        surface.blit(self.bones_img, (10, 42))
        surface.blit(self.effectors_img, (10, 74))
