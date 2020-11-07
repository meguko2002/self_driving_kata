import cv2 as cv
import numpy as np

from mylib import maze, util

def draw_path_fromto(input, path, color):
    delta_square = [util.Point(y, x)
                    for y in range(3)
                    for x in range(3)]
    spread_path = [p + d
                   for p in path
                   for d in delta_square]
    output = np.ndarray.copy(input)
    for p in spread_path:
        output[p] = color
    return output

def paint_path(input):
    """白黒画像配列をカラー画像配列に変換して返す。灰色の経路部分は茶色に着色。"""
    output = np.ndarray(shape=input.shape + (3,))
    for iy, row in enumerate(input):
        for ix, grayscale in enumerate(row):
            if grayscale > 150:
                output[iy, ix] = [255, 255, 255]  # 白 → 白(BGR)
            elif grayscale < 150:
                output[iy, ix] = [0, 0, 0]        # 黒 → 黒(BGR)
            else:
                output[iy, ix] = [0, 0, 255]    # 灰 → red(BGR)
    return output


def main(inputfile, outputfile):
    PATHCOLOR = 150  # gray
    # start_point = util.Point(22, 70)
    # goal_point = util.Point(55, 80)
    start_point = util.Point(33, 65)
    goal_point = util.Point(56, 880)

    input = cv.imread (inputfile, 0)
    path = maze.get_path ( input , start_point , goal_point )
    output_gray = draw_path_fromto(input ,path, PATHCOLOR)
    output = paint_path(output_gray)

    cv.imwrite ( outputfile , output_gray)
    cv.imwrite ( outputfile , output)

if __name__ == "__main__":

    input_image = './input/4777.png'
    output_image = './output/route2.jpg'
    main(input_image, output_image)
    # main('./input/2chika.jpg',  './output/route2.jpg')

