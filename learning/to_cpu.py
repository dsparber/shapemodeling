import torch
import glob
import os
import argparse
from model import PointNetAutoencoder

model = PointNetAutoencoder(2319)
model.load_state_dict(torch.load('model.pt'))
model = model.to('cpu')
torch.save(model.state_dict(), 'model.pt')
