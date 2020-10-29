__author__ = "TheComet"

import ik
import pygame
from Updateable import Updateable
from time import time


def rotate_points(points, q):
    cos_a = 1 - 2 * (q.x * q.x + q.y * q.y)
    sin_a = 2 * (q.w * q.x + q.y * q.z)

    for point in points:
        yield point[0]*cos_a - point[1]*sin_a, \
              point[0]*sin_a + point[1]*cos_a


def scale_points(points, scale):
    for point in points:
        yield scale * point[0], \
              scale * point[1]


def translate_points(points, pos):
    for point in points:
        yield point[0] + pos[0], \
              point[1] + pos[1]


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
        pygame.draw.line(surface, color, start, end, 2)


def __draw_tree(surface, diamond, bone, acc_pos, acc_rot):
    rot = acc_rot * bone.rotation
    pos = acc_pos + bone.position

    # transform shape into world space
    transformed_diamond = list(rotate_points(diamond, rot))
    transformed_diamond = list(scale_points(transformed_diamond, bone.length))
    transformed_diamond = list(translate_points(transformed_diamond, (pos.y, pos.z)))

    # get tail and head position of bone so we can draw circles there
    tail_pos = transform_to_screen(transformed_diamond[0])
    head_pos = transform_to_screen(transformed_diamond[2])

    # draw line if bone has an offset
    pygame.draw.line(surface, (100, 100, 100), transform_to_screen((acc_pos.y, acc_pos.z)), tail_pos, 1)

    # draw the shape
    draw_closed_shape(surface, transformed_diamond, (100, 100, 255))
    pygame.draw.circle(surface, (100, 100, 255), tail_pos, 4, 2)
    pygame.draw.circle(surface, (100, 100, 255), head_pos, 4, 2)

    # child bone position is relative to the head of the current bone
    pos = ik.Vec3(0, transformed_diamond[2][0], transformed_diamond[2][1])

    for child in bone.children:
        __draw_tree(surface, diamond, child, pos, rot)


def draw_tree(surface, root):
    diamond = (
        (0, 0),
        (-0.1, 0.16),
        (0, 1),
        (0.1, 0.16)
    )
    __draw_tree(surface, diamond, root, ik.Vec3(), ik.Quat())


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
