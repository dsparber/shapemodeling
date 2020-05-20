import torch
from data.face_dataset import FaceDataset
from model.autoencoder import Autoencoder
from torch.nn import MSELoss
from torch.nn.functional import pad


batch_size = 8
data_path = '../data/aligned_faces_example/example4'

if __name__ == "__main__":
    dataset = FaceDataset(data_path)
    data_loader = torch.utils.data.DataLoader(dataset, batch_size=batch_size, shuffle=False)
    
    num_points = dataset.num_points()
    model = Autoencoder(num_points)
    model.load_state_dict(torch.load('model.pt'))
    
    with torch.set_grad_enabled(False):

        for x in data_loader:
            padding = model.stride() - (x.size(2) % model.stride())
            x_pad = pad(x, [padding, 0, 0, 0, 0, 0], mode='constant', value=0)
            z = model.encode(x_pad)
            print(z)
