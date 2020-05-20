import torch
from data.face_dataset import FaceDataset
from model.autoencoder import Autoencoder
from torch.nn import MSELoss
from torch.nn.functional import pad

model = Autoencoder(2319)
model.load_state_dict(torch.load('model.pt'))

def decode(z, padding):
    with torch.set_grad_enabled(False):
        z_tensor = torch.tensor(z).view(1, 1, len(z))
        x = model.decode(z_tensor)
        x = x[0].transpose(1, 0)
        x = x[slice(padding, x.size(0))]
        return x


if __name__ == "__main__":

    z = [0.3562, 0.4305, 0.5411, 0.7245, 0.6885, 0.6791, 0.4416, 0.2936, 0.2498, 0.2990]
    print(decode(z, 0))

