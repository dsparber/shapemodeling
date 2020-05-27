import torch
from data.face_dataset import FaceDataset
from model import PointNetAutoencoder
from torch.nn import MSELoss
from torch.nn.functional import pad


batch_size = 8
data_path = '../data/warped_meshes/'

if __name__ == "__main__":
    dataset = FaceDataset(data_path)
    data_loader = torch.utils.data.DataLoader(dataset, batch_size=batch_size, shuffle=False)
    
    num_points = dataset.num_points()
    model = PointNetAutoencoder(num_points)
    model.load_state_dict(torch.load('model.pt'))
    
    with torch.set_grad_enabled(False):

        for x in data_loader:
            model.eval()
            z = model.encode(x)
            print(z)
