# AI-Based-Real-time-Posture-Monitoring-System

# Posture Tracking System

This project implements a sensor-agnostic posture tracking system capable of detecting five distinct postures using data from 3-axis sensors. The system is designed to work with various sensor inputs and has been trained and evaluated using an IMU sensor unit embedded in an Arduino board.

## Features

- Detects five postures: supine, prone, side (right or left), sitting, and unknown
- Sensor-agnostic model that works with 3-channel input from various 3-axis sensors
- Developed and trained using Python in Google Colab

## System Design

### Deep Learning Model Architecture

The neural network is implemented using Keras with the following structure:

1. Input Layer
2. Multiple Dense layers with varying units
3. Dropout layers for regularization
4. Output Dense layer (5 units) with softmax activation

### Training Parameters

- Optimizer: Adam (learning rate: 0.001)
- Loss Function: Categorical Crossentropy
- Batch Size: 32
- Epochs: 100 (with early stopping and learning rate reduction)

## Implementation Details

- EarlyStopping and ReduceLROnPlateau callbacks are used to prevent overfitting and optimize learning
- Model performance is visualized using matplotlib
- Training and evaluation are performed in Google Colab for efficient computation

## Getting Started

1. Clone this repository
2. Open the Jupyter notebook in Google Colab
3. Follow the instructions in the notebook to train and evaluate the model

## Requirements

- Python 3.x
- TensorFlow 2.x
- Keras
- Numpy
- Matplotlib
- Google Colab (for training and evaluation)

## Contributing

Contributions to improve the model or extend its capabilities are welcome. Please feel free to submit pull requests or open issues for discussion.
