import torch
from data.face_dataset import FaceDataset
from model import Autoencoder
from torch.nn import MSELoss
from torch.nn.functional import pad


batch_size = 32
epochs = 1000
learning_rate = 1e-3
data_path = '../data/warped_meshes/'
pretrained_model = 'checkpoints/model-300.pt'

if __name__ == "__main__":
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    dataset = FaceDataset(data_path)
    data_loader = torch.utils.data.DataLoader(dataset, batch_size=batch_size, shuffle=True)

    num_points = dataset.num_points()
    
    model = Autoencoder(num_points)
    if pretrained_model is not None:
        model.load_state_dict(torch.load(pretrained_model))
    model = model.to(device)
    loss_fn = MSELoss()
    optimizer = torch.optim.Adamax(model.parameters(), lr=learning_rate, eps=1e-7)

    for epoch in range(epochs):
        for batch_id, x in enumerate(data_loader):
            x = x.to(device)
            x_hat = model.forward(x)
            criterion = loss_fn(x, x_hat)

            optimizer.zero_grad()
            criterion.backward()
            optimizer.step()
        
        print("{}: {}".format(epoch, criterion.item()))
        
        if epoch % 100 == 0:
            torch.save(model.state_dict(), 'checkpoints/model-{}.pt'.format(epoch))

    torch.save(model.state_dict(), 'model.pt')