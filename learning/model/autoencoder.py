import torch
import torch.nn as nn
from .reshape import Reshape

class Autoencoder(nn.Module):
    def __init__(self):
        super(Autoencoder, self).__init__()
        self.total_stride = 2 ** 8
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

            nn.Conv1d(in_channels=32, out_channels=16, stride=2, kernel_size=3, padding=1),
            nn.BatchNorm1d(16),
            nn.ReLU(True),
            nn.Conv1d(in_channels=16, out_channels=8, stride=2, kernel_size=3, padding=1),
            nn.BatchNorm1d(8),
            nn.ReLU(True),
            nn.Conv1d(in_channels=8, out_channels=4, stride=2, kernel_size=3, padding=1),
            nn.BatchNorm1d(4),
            nn.ReLU(True),
            nn.Conv1d(in_channels=4, out_channels=2, stride=2, kernel_size=3, padding=1),
            nn.BatchNorm1d(2),
            nn.ReLU(True),
            nn.Conv1d(in_channels=2, out_channels=1, stride=2, kernel_size=3, padding=1),
            nn.BatchNorm1d(1),
            nn.Sigmoid()
        )
        self.decoder = nn.Sequential(
            nn.ConvTranspose1d(in_channels=1, out_channels=2, stride=2, kernel_size=3, padding=1, output_padding=1),
            nn.BatchNorm1d(2),
            nn.ReLU(True),
            nn.ConvTranspose1d(in_channels=2, out_channels=4, stride=2, kernel_size=3, padding=1, output_padding=1),
            nn.BatchNorm1d(4),
            nn.ReLU(True),
            nn.ConvTranspose1d(in_channels=4, out_channels=8, stride=2, kernel_size=3, padding=1, output_padding=1),
            nn.BatchNorm1d(8),
            nn.ReLU(True),
            nn.ConvTranspose1d(in_channels=8, out_channels=16, stride=2, kernel_size=3, padding=1, output_padding=1),
            nn.BatchNorm1d(16),
            nn.ReLU(True),
            nn.ConvTranspose1d(in_channels=16, out_channels=32, stride=2, kernel_size=3, padding=1, output_padding=1),
            nn.BatchNorm1d(32),
            nn.ReLU(True),

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
        return self.encoder(x)

    def decode(self, z):
        return self.decoder(z)

    def stride(self):
        return self.total_stride
