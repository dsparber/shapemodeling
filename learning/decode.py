import torch
import glob
import os
import argparse
from data.face_dataset import FaceDataset
from model import PointNetAutoencoder
from torch.nn import MSELoss
from torch.nn.functional import pad
from data.obj_utils import write_obj, load_obj

model = PointNetAutoencoder(2319)
model.load_state_dict(torch.load('model.pt'))
data_path = '../data/warped_meshes/'

def export(x):
    template = glob.glob(os.path.join(data_path, "*.obj"))[0]
    _, f = load_obj(template)
    write_obj('learning_out.obj', x, f)

def decode(z):
    with torch.set_grad_enabled(False):
        model.eval()
        z_tensor = torch.tensor([z]).view(1, len(z))
        x = model.decode(z_tensor)
        x = x[0].transpose(1, 0)
        export(x)
        return x


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Process some integers.')
    parser.add_argument('z', metavar='N', type=float, nargs='+')
    args = parser.parse_args()
    z = args.z
    decode(z)

