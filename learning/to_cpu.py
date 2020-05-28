import torch
import glob
import os
import argparse
from model import Autoencoder

model = Autoencoder(2319)
model.load_state_dict(torch.load('model.pt'))
model = model.to('cpu')
torch.save(model.state_dict(), 'model.pt')

