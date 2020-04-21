"""Use A-star algorithm."""
import heapq as hq
import typing

import scipy.spatial as spatial

from . import util


def is_passable(pixel) -> bool:
    """
    Judge whether we can pass through the pixel or not.
    Parameters
    ----------
    pixel
        The value of the pixel in the image. Unless the maze image is
        grayscale, the grayscale integer is passed.
    """
    return pixel > 200


def heuristic_cost(point1: util.Point,
                   point2: util.Point) -> float:
    return spatial.distance.euclidean(point1, point2)


class Node:
    __slots__ = ("_point", "_cost", "_heuristic_cost", "_parent")

    def __init__(self,
                 point: util.Point,
                 cost: int,
                 heuristic_cost: float,
                 parent):
        self._point = point
        self._cost = cost
        self._heuristic_cost = heuristic_cost
        self._parent = parent

    @property
    def priority_score(self):
        return self._cost + self._heuristic_cost

    @property
    def _compare_values(self):
        return (self.priority_score,
                self.cost,
                self.point)

    def __eq__(self, other):
        return self._compare_values < other._compare_values

    def __lt__(self, other):
        return self._compare_values < other._compare_values


# Define getters of `Node` class.
for attr in Node.__slots__:
    util.def_getter(Node, attr)


def get_path(maze_img,
             start_point: util.Point,
             goal_point: util.Point) -> typing.List[util.Point]:
    goal_node = _get_goal_node(maze_img, start_point, goal_point)
    if goal_node is None:
        raise Exception("The goal is not found.")

    path_reverse = [goal_node.point]
    parent_node = goal_node.parent
    while(parent_node is not None):
        path_reverse.append(parent_node.point)
        parent_node = parent_node.parent

    return list(reversed(path_reverse))


def _get_goal_node(maze_img,
                   start_point: util.Point,
                   goal_point: util.Point) -> Node:

    def is_goal(node: Node):
        return node.point == goal_point

    start_node = Node(point=start_point,
                      cost=0,
                      heuristic_cost=heuristic_cost(goal_point, start_point),
                      parent=None)
    if is_goal(start_node):
        return start_node

    shape = maze_img.shape
    open_node_map = {start_node.point: start_node}
    node_hq = [start_node]


    while (len(node_hq) > 0):
        center_node = hq.heappop(node_hq)
        # print ( f'node {center_node.point}' )
        next_cost = center_node.cost + 1
        next_nodes = [Node(point=p,
                           cost=next_cost,
                           heuristic_cost=heuristic_cost(goal_point, p),
                           parent=center_node)
                      for p in center_node.point.udlr()
                      if p.y >= 0
                      and p.x >= 0
                      and p.y < shape[0]
                      and p.x < shape[1]
                      and open_node_map.get(p) is None
                      and is_passable(maze_img[p])]
        for node in next_nodes:
            if is_goal(node):
                return node
            open_node_map[node.point] = node
            # Visualize cost for debugging.
            # maze_img[node.point] = 255 - node.cost
            hq.heappush(node_hq, node)

    return None