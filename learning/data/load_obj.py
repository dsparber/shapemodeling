from torch.utils.data import Dataset
import torch
import os
import glob

def _parse_line_float(line):
    values = line.split(' ')[1:]
    return [float(value) for value in values]

def _parse_line_int(line):
    values = line.split(' ')[1:]
    return [int(value) - 1 for value in values]

def load_obj(path):

    with open(path, mode='r') as f:
        lines = f.readlines()
    vertex_lines = filter(lambda line: line.startswith("v"), lines)
    face_lines = filter(lambda line: line.startswith("f"), lines)
    vertices = list(map(_parse_line_float, vertex_lines))
    faces = list(map(_parse_line_int, face_lines))
    v = torch.tensor(vertices)
    f = torch.tensor(faces)

    return v, f