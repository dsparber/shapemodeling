import torch
import glob
import os
from model import Autoencoder
from data.obj_utils import load_obj


high_res = False


def get_model():
    n = 22779 if high_res else 2319
    saved_model = 'model_high_res.pt' if high_res else 'model.pt'
    model = Autoencoder(n)
    model.load_state_dict(torch.load(saved_model))

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model = model.to(device)
    return model

def get_faces():
    data_path = '../data/warped_meshes_high_res/' if high_res else '../data/warped_meshes/'
    template = glob.glob(os.path.join(data_path, "*.obj"))[0]
    _, f = load_obj(template)
    return f