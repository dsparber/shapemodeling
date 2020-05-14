import torch
from data.face_dataset import FaceDataset
from model.autoencoder import Autoencoder
from torch.nn import MSELoss
from torch.nn.functional import pad


batch_size = 27
epochs = 10000
learning_rate = 1e-2
data_path = '../data/aligned_faces_example/example4'

if __name__ == "__main__":
    dataset = FaceDataset(data_path)
    data_loader = torch.utils.data.DataLoader(dataset, batch_size=batch_size, shuffle=True)
    
    model = Autoencoder()
    loss_fn = MSELoss()
    optimizer = torch.optim.Adamax(model.parameters(), lr=learning_rate, eps=1e-7)

    for epoch in range(epochs):
        for batch_id, x in enumerate(data_loader):
            padding = model.stride() - (x.size(2) % model.stride())
            x_pad = pad(x, [padding, 0, 0, 0, 0, 0], mode='constant', value=0)
            x_hat = model.forward(x_pad)
            criterion = loss_fn(x_pad, x_hat)

            optimizer.zero_grad()
            criterion.backward()
            optimizer.step()
        
        if epoch % 10 == 0:
            print("{}: {}".format(epoch, criterion.item()))

    torch.save(model.state_dict(), 'model.pt')