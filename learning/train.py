import torch
from data.face_dataset import FaceDataset
from model import PointNetAutoencoder
from torch.nn import MSELoss
from torch.nn.functional import pad


batch_size = 8
epochs = 10000
learning_rate = 1e-3
data_path = '../data/aligned_faces_example/example4'

if __name__ == "__main__":
    dataset = FaceDataset(data_path)
    data_loader = torch.utils.data.DataLoader(dataset, batch_size=batch_size, shuffle=True)

    num_points = dataset.num_points()
    
    model = PointNetAutoencoder(num_points)
    loss_fn = MSELoss()
    optimizer = torch.optim.Adamax(model.parameters(), lr=learning_rate, eps=1e-7)

    for epoch in range(epochs):
        for batch_id, x in enumerate(data_loader):
            x_hat = model.forward(x)
            criterion = loss_fn(x, x_hat)

            optimizer.zero_grad()
            criterion.backward()
            optimizer.step()
        
        if epoch % 10 == 0:
            print("{}: {}".format(epoch, criterion.item()))

    torch.save(model.state_dict(), 'model.pt')