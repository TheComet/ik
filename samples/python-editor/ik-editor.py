__author__ = "TheComet"

import pygame
import ik
from Window import Window

pygame.init()
ik.log.severity = ik.DEBUG
window = Window(1280, 960)
window.enter_main_loop()
pygame.quit()
