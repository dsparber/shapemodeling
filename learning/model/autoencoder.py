import torch
import torch.nn as nn
from .fully_connected import FullyConnected

class Autoencoder(nn.Module):
    def __init__(self, num_points):
        super(Autoencoder, self).__init__()
        self.total_stride = 2 ** 3
        self.num_points = num_points
        self.encoder = nn.Sequential(
            nn.Conv1d(in_channels=3, out_channels=64, stride=2, kernel_size=5, padding=2),
            nn.BatchNorm1d(64),
            nn.ReLU(True),
            nn.Conv1d(in_channels=64, out_channels=64, stride=2, kernel_size=5, padding=2),
            nn.BatchNorm1d(64),
            nn.ReLU(True),
            nn.Conv1d(in_channels=64, out_channels=64, stride=1, kernel_size=5, padding=2),
            nn.BatchNorm1d(64),
            nn.ReLU(True),
            nn.Conv1d(in_channels=64, out_channels=64, stride=1, kernel_size=5, padding=2),
            nn.BatchNorm1d(64),
            nn.ReLU(True),
            nn.Conv1d(in_channels=64, out_channels=32, stride=2, kernel_size=3, padding=1),
            nn.BatchNorm1d(32),
            nn.ReLU(True),
        )
        self.fully_connected = FullyConnected(input_dim=(32, (num_points + self.get_padding()) // self.total_stride), latent_dim=10)
        self.decoder = nn.Sequential(
            nn.ConvTranspose1d(in_channels=32, out_channels=64, stride=2, kernel_size=3, padding=1, output_padding=1),
            nn.BatchNorm1d(64),
            nn.ReLU(True),
            nn.ConvTranspose1d(in_channels=64, out_channels=64, stride=1, kernel_size=5, padding=2),
            nn.BatchNorm1d(64),
            nn.ReLU(True),
            nn.ConvTranspose1d(in_channels=64, out_channels=64, stride=1, kernel_size=5, padding=2),
            nn.BatchNorm1d(64),
            nn.ReLU(True),
            nn.ConvTranspose1d(in_channels=64, out_channels=64, stride=2, kernel_size=5, padding=2, output_padding=1),
            nn.BatchNorm1d(64),
            nn.ReLU(True),
            nn.ConvTranspose1d(in_channels=64, out_channels=3, stride=2, kernel_size=5, padding=2, output_padding=1),
            nn.BatchNorm1d(3),
        )


    def forward(self, x):
        z = self.encode(x)
        x_hat = self.decode(z)
        return x_hat

    def encode(self, x):
        y = self.encoder(x)
        z = self.fully_connected.encode(y)
        return z

    def decode(self, z):
        y = self.fully_connected.decode(z)
        x_hat = self.decoder(y)
        return x_hat

    def stride(self):
        return self.total_stride

    def get_padding(self):
        return self.stride() - (self.num_points % self.stride())

