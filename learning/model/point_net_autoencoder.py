import torch
import torch.nn as nn
from .fully_connected import FullyConnected

class PointNetAutoencoder(nn.Module):
    def __init__(self, num_points):
        super(PointNetAutoencoder, self).__init__()
        self.num_points = num_points
        self.point_net_encoder = nn.Sequential(
            nn.Conv1d(in_channels=3, out_channels=64, stride=1, kernel_size=1),
            nn.BatchNorm1d(64),
            nn.ReLU(True),
            nn.Conv1d(in_channels=64, out_channels=64, stride=1, kernel_size=1),
            nn.BatchNorm1d(64),
            nn.ReLU(True),
            nn.Conv1d(in_channels=64, out_channels=64, stride=1, kernel_size=1),
            nn.BatchNorm1d(64),
            nn.ReLU(True),
            nn.Conv1d(in_channels=64, out_channels=128, stride=1, kernel_size=1),
            nn.BatchNorm1d(128),
            nn.ReLU(True),
            nn.Conv1d(in_channels=128, out_channels=1024, stride=1, kernel_size=1),
            nn.BatchNorm1d(1024),
            nn.ReLU(True),
            nn.MaxPool1d(kernel_size=num_points)
        )
        self.fully_connected_encoder = nn.Sequential(
            nn.Linear(in_features=1024, out_features=512),
            nn.Dropout(p=0.3),
            nn.ReLU(True),
            nn.Linear(in_features=512, out_features=256),
            nn.Dropout(p=0.3),
            nn.ReLU(True),
            nn.Linear(in_features=256, out_features=10),
            nn.Tanh(),
        )
        self.decoder = nn.Sequential(
            nn.Linear(in_features=10, out_features=256),
            nn.BatchNorm1d(256),
            nn.ReLU(True),
            nn.Linear(in_features=256, out_features=512),
            nn.BatchNorm1d(512),
            nn.ReLU(True),
            nn.Linear(in_features=512, out_features=1024),
            nn.BatchNorm1d(1024),
            nn.ReLU(True),
            nn.Linear(in_features=1024, out_features=1024),
            nn.BatchNorm1d(1024),
            nn.ReLU(True),
            nn.Linear(in_features=1024, out_features=1024),
            nn.BatchNorm1d(1024),
            nn.ReLU(True),
            nn.Linear(in_features=1024, out_features=num_points * 3),
        )


    def forward(self, x):
        z = self.encode(x)
        x_hat = self.decode(z)
        return x_hat

    def encode(self, x):
        y = self.point_net_encoder(x)
        y = y.view(y.size(0), -1)
        z = self.fully_connected_encoder(y)
        return z

    def decode(self, z):
        x_hat = self.decoder(z)
        x_hat = x_hat.view(x_hat.size(0), 3, self.num_points)
        return x_hat

    def stride(self):
        return self.total_stride

