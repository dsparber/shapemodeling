import torch
import argparse
from config import get_model, get_faces
from data.obj_utils import write_obj

model = get_model()
f = get_faces()


def export(x):
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
    parser.add_argument('latents', metavar='N', type=float, nargs='+')
    args = parser.parse_args()
    decode(args.latents)

