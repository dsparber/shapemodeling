import torch
import os
import argparse
from config import get_model
from data.obj_utils import load_obj

model = get_model()

def export(latents):
    lines = [' '.join([str(v.item()) for v in z]) + '\n' for z in latents]
    with open('latents.txt', mode='w') as f:
        f.writelines(lines)

def encode(paths):
    latents = []
    with torch.set_grad_enabled(False):
        for path in paths:
            x, _ = load_obj(path)
            x = x.permute(1, 0)
            x = x.view(1, x.size(0), x.size(1))
            model.eval()
            z = model.encode(x)
            latents.append(z[0])

    export(latents)
    return latents


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Process some integers.')
    parser.add_argument('paths', metavar='N', type=str, nargs='+')
    args = parser.parse_args()
    encode(args.paths)
