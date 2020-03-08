import math
import pygame

pygame.init()
display_width = 200
display_height = 200
gameDisplay = pygame.display.set_mode((display_width, display_height))
clock = pygame.time.Clock()

MIN_POS = 0
MAX_POS = 200
pos_range = (MAX_POS - MIN_POS) / 2
ctr_pos = (MAX_POS + MIN_POS) / 2

# dir = [(1, 0), (0.7, 0.7), (0, 1), (-0.7, 0.7), (-1, 0), (-0.7, -0.7), (0, -1), (0.7, -0.7),(0,0)]
dir = [(1, 0), (1, 1), (0, 1), (-1, 1), (-1, 0), (-1, -1), (0, -1), (1, -1),(0,0)]
# keyset = [3, 2, 1, 4, 7, 8, 9, 6]
keyset = [6,9,8,7,4,1,2,3,5]
dir_num = len(dir)


def get_command_from_key(key):
    command = -1
    for i in range(dir_num):
        if key == keyset[i]:
            command = i
    return command


def constrain(data, min_pos, max_pos):
    ret = data
    if data < min_pos:
        ret = min_pos
    elif data > max_pos:
        ret = max_pos
    return ret



def tilt_field(command, pos_l, pos_r):
    target = [i * pos_range + ctr_pos for i in dir[command]]
    act = -1
    qt_l = target[0] - pos_l
    qt_r = target[1] - pos_r
    # print(f'QT = ({qt_l},{qt_r}) ')
    if qt_l ==0 and qt_r ==0:
        return pos_l, pos_r, act
    elif qt_l > 0:
        if qt_r > 0:
            if qt_r < qt_l:
                act_index = [0, 1]
            else:
                act_index = [1, 2]
        else:
            if qt_r < -qt_l:
                act_index = [6, 7]
            else:
                act_index = [7, 0]
    else:
        if qt_r > 0:
            if qt_r > -qt_l:
                act_index = [2, 3]
            else:
                act_index = [3, 4]
        else:
            if qt_r > qt_l:
                act_index = [4, 5]
            else:
                act_index = [5, 6]

    # q_next_tのnormを比較
    min_d =1000000
    for i in act_index:
        if i%2 == 0:
            d = norm([qt_l,qt_r], [dir[i][0], dir[i][1]])
        else:
            d = norm([qt_l,qt_r], [dir[i][0]*0.7, dir[i][1]*0.7])
        if d <min_d:
            min_d = d
            act = i
    pos_l =constrain(pos_l + dir[act][0], MIN_POS, MAX_POS)
    pos_r = constrain(pos_r + dir[act][1], MIN_POS, MAX_POS)
    return pos_l, pos_r, act
def main():
    pos_l = 100
    pos_r = 150
    gameDisplay.fill((255, 255, 255))
    pygame.display.update()
    print("input key")
    while True:
        for event in pygame.event.get():
            if event.type == pygame.KEYDOWN:
                for i in range(10):
                    key = event.key - 256
                    command = get_command_from_key(int(key))
                    pos_l, pos_r, act = tilt_field(command, pos_l, pos_r)
                    print(f'action:{act}, pos({pos_l}, {pos_r})')
                    gameDisplay.fill((255, 255, 255))
                    x = int(display_width / 2 * (1 + (pos_l - ctr_pos) / pos_range))
                    y = int(display_height / 2 * (1 - (pos_r - ctr_pos) / pos_range))
                    pygame.draw.circle(gameDisplay, (0, 0, 0), [x, y], 5)
                    pygame.display.update()
                    # clock.tick(10)
                    print("input key")

def norm(qt, dir):
    return (qt[0]-dir[0])*(qt[0]-dir[0]) + (qt[1]-dir[1])*(qt[1]-dir[1])

# def main():
    # pos_l = 9
    # pos_r = -8
    # print(f'pos({pos_l}, {pos_r})')
    # while True:
    #     print("input key")
    #     try:
    #         key = input()
    #         command = get_command_from_key(int(key))
    #         if command != -1:
    #             for i in range(3):
    #                 pos_l, pos_r, act = tilt_field(command, pos_l, pos_r)
    #                 print(f'action:{act}, pos({pos_l}, {pos_r})')
    #     except ValueError:
    #         pass

if __name__ == "__main__":
    main()
