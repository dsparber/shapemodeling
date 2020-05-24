import torch
import torch.nn as nn
from functools import reduce

class FullyConnected(nn.Module):
    def __init__(self, input_dim, latent_dim):
        super(FullyConnected, self).__init__()
        self.input_dim = input_dim
        self.latent_dim = latent_dim
        self.input_size = reduce(lambda a, b: a * b, input_dim, 1)

        self.encoder = nn.Sequential(
            nn.Linear(in_features=self.input_size, out_features=self.input_size),
            nn.ReLU(True),
            nn.Linear(in_features=self.input_size, out_features=self.input_size),
            nn.ReLU(True),
            nn.Linear(in_features=self.input_size, out_features=self.input_size),
            nn.ReLU(True),
            nn.Linear(in_features=self.input_size, out_features=self.latent_dim),
            nn.Tanh(),
        )
        self.decoder = nn.Sequential(
            nn.Linear(in_features=self.latent_dim, out_features=self.input_size),
            nn.ReLU(True),
            nn.Linear(in_features=self.input_size, out_features=self.input_size ),
            nn.ReLU(True),
            nn.Linear(in_features=self.input_size, out_features=self.input_size),
            nn.ReLU(True),
            nn.Linear(in_features=self.input_size, out_features=self.input_size),
            nn.ReLU(True),
        )


    def forward(self, x):
        z = self.encode(x)
        x_hat = self.decode(z)
        return x_hat

    def encode(self, x):
        x = x.view(x.size(0), self.input_size)
        return self.encoder(x)

    def decode(self, z):
        x_hat = self.decoder(z)
        x_hat = x_hat.view(x_hat.size(0), self.input_dim[0], self.input_dim[1])
        return x_hat

    def stride(self):
        return self.total_stride
