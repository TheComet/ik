__author__ = "TheComet"

import pygame
from Window import Window

pygame.init()
window = Window(1280, 960)
window.enter_main_loop()
pygame.quit()
