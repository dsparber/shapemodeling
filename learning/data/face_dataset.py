from torch.utils.data import Dataset
from . import load_obj
import torch
import os
import glob


class FaceDataset(Dataset):

    def __init__(self, path):
        super(FaceDataset, self).__init__()
        self.path = path
        self.files = glob.glob(os.path.join(path, "*.obj"))
        
        tensors = []
        for path in self.files:
            vertices, _ = load_obj(path)
            tensors.append(vertices.transpose(1, 0))

        self.mean_tensor = sum(tensors) / len(tensors)      
        self.tensors = tensors

    def __len__(self):
        return len(self.tensors)

    def __getitem__(self, index):
        return self.tensors[index]
    
    def get_mean(self):
        return self.mean_tensor

    def num_points(self):
        return self.tensors[0].size(1)

    @staticmethod
    def _line_to_point(line):
        values = line.split(' ')[1:]
        return [float(value) for value in values]