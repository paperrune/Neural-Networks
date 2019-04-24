## Results
### Keras
```
_________________________________________________________________
Layer (type)                 Output Shape              Param #
=================================================================
input_1 (InputLayer)         (None, 32, 32, 3)         0
_________________________________________________________________
conv2d_1 (Conv2D)            (None, 32, 32, 32)        896
_________________________________________________________________
batch_normalization_1 (Batch (None, 32, 32, 32)        128
_________________________________________________________________
activation_1 (Activation)    (None, 32, 32, 32)        0
_________________________________________________________________
conv2d_2 (Conv2D)            (None, 32, 32, 32)        9248
_________________________________________________________________
batch_normalization_2 (Batch (None, 32, 32, 32)        128
_________________________________________________________________
activation_2 (Activation)    (None, 32, 32, 32)        0
_________________________________________________________________
max_pooling2d_1 (MaxPooling2 (None, 16, 16, 32)        0
_________________________________________________________________
conv2d_3 (Conv2D)            (None, 16, 16, 64)        18496
_________________________________________________________________
batch_normalization_3 (Batch (None, 16, 16, 64)        256
_________________________________________________________________
activation_3 (Activation)    (None, 16, 16, 64)        0
_________________________________________________________________
conv2d_4 (Conv2D)            (None, 16, 16, 64)        36928
_________________________________________________________________
batch_normalization_4 (Batch (None, 16, 16, 64)        256
_________________________________________________________________
activation_4 (Activation)    (None, 16, 16, 64)        0
_________________________________________________________________
max_pooling2d_2 (MaxPooling2 (None, 8, 8, 64)          0
_________________________________________________________________
conv2d_5 (Conv2D)            (None, 8, 8, 128)         73856
_________________________________________________________________
batch_normalization_5 (Batch (None, 8, 8, 128)         512
_________________________________________________________________
activation_5 (Activation)    (None, 8, 8, 128)         0
_________________________________________________________________
conv2d_6 (Conv2D)            (None, 8, 8, 128)         147584
_________________________________________________________________
batch_normalization_6 (Batch (None, 8, 8, 128)         512
_________________________________________________________________
activation_6 (Activation)    (None, 8, 8, 128)         0
_________________________________________________________________
class_activation_map (Conv2D (None, 8, 8, 10)          1290
_________________________________________________________________
global_average_pooling2d_1 ( (None, 10)                0
_________________________________________________________________
activation_7 (Activation)    (None, 10)                0
=================================================================
Total params: 290,090
Trainable params: 289,194
Non-trainable params: 896
_________________________________________________________________
Epoch 1/50
391/391 [==============================] - 13s 32ms/step - loss: 1.4531 - acc: 0.4730 - val_loss: 1.4552 - val_acc: 0.4988
Epoch 2/50
391/391 [==============================] - 11s 29ms/step - loss: 1.0591 - acc: 0.6263 - val_loss: 1.1196 - val_acc: 0.6067
Epoch 3/50
391/391 [==============================] - 11s 29ms/step - loss: 0.8974 - acc: 0.6854 - val_loss: 1.0018 - val_acc: 0.6700
Epoch 4/50
391/391 [==============================] - 11s 29ms/step - loss: 0.8009 - acc: 0.7205 - val_loss: 1.1544 - val_acc: 0.6364
Epoch 5/50
391/391 [==============================] - 11s 29ms/step - loss: 0.7414 - acc: 0.7403 - val_loss: 0.7099 - val_acc: 0.7537
Epoch 6/50
391/391 [==============================] - 11s 29ms/step - loss: 0.6886 - acc: 0.7611 - val_loss: 0.7454 - val_acc: 0.7432
Epoch 7/50
391/391 [==============================] - 11s 29ms/step - loss: 0.6598 - acc: 0.7691 - val_loss: 0.9083 - val_acc: 0.7071
Epoch 8/50
391/391 [==============================] - 11s 29ms/step - loss: 0.6231 - acc: 0.7836 - val_loss: 0.8142 - val_acc: 0.7366
Epoch 9/50
391/391 [==============================] - 11s 29ms/step - loss: 0.6027 - acc: 0.7904 - val_loss: 0.6067 - val_acc: 0.7951
Epoch 10/50
391/391 [==============================] - 11s 29ms/step - loss: 0.5737 - acc: 0.8013 - val_loss: 0.8846 - val_acc: 0.7154
Epoch 11/50
391/391 [==============================] - 11s 29ms/step - loss: 0.5531 - acc: 0.8062 - val_loss: 0.8995 - val_acc: 0.7125
Epoch 12/50
391/391 [==============================] - 11s 28ms/step - loss: 0.5367 - acc: 0.8122 - val_loss: 0.6102 - val_acc: 0.8024
Epoch 13/50
391/391 [==============================] - 11s 29ms/step - loss: 0.5224 - acc: 0.8169 - val_loss: 0.6424 - val_acc: 0.7952
Epoch 14/50
391/391 [==============================] - 11s 29ms/step - loss: 0.5046 - acc: 0.8252 - val_loss: 0.6700 - val_acc: 0.7807
Epoch 15/50
391/391 [==============================] - 11s 29ms/step - loss: 0.4882 - acc: 0.8313 - val_loss: 0.6034 - val_acc: 0.8134
Epoch 16/50
391/391 [==============================] - 11s 29ms/step - loss: 0.4834 - acc: 0.8307 - val_loss: 0.7046 - val_acc: 0.7781
Epoch 17/50
391/391 [==============================] - 11s 29ms/step - loss: 0.4701 - acc: 0.8358 - val_loss: 0.6749 - val_acc: 0.7912
Epoch 18/50
391/391 [==============================] - 11s 29ms/step - loss: 0.4569 - acc: 0.8408 - val_loss: 0.4998 - val_acc: 0.8358
Epoch 19/50
391/391 [==============================] - 11s 29ms/step - loss: 0.4507 - acc: 0.8443 - val_loss: 0.5564 - val_acc: 0.8110
Epoch 20/50
391/391 [==============================] - 11s 29ms/step - loss: 0.4402 - acc: 0.8458 - val_loss: 0.6298 - val_acc: 0.8044
Epoch 21/50
391/391 [==============================] - 11s 29ms/step - loss: 0.4372 - acc: 0.8460 - val_loss: 0.6142 - val_acc: 0.8108
Epoch 22/50
391/391 [==============================] - 11s 29ms/step - loss: 0.4232 - acc: 0.8524 - val_loss: 0.5577 - val_acc: 0.8183
Epoch 23/50
391/391 [==============================] - 11s 28ms/step - loss: 0.4212 - acc: 0.8522 - val_loss: 0.5529 - val_acc: 0.8238
Epoch 24/50
391/391 [==============================] - 11s 28ms/step - loss: 0.4094 - acc: 0.8587 - val_loss: 0.5477 - val_acc: 0.8210
Epoch 25/50
391/391 [==============================] - 11s 28ms/step - loss: 0.4036 - acc: 0.8596 - val_loss: 0.4672 - val_acc: 0.8438
Epoch 26/50
391/391 [==============================] - 11s 28ms/step - loss: 0.3941 - acc: 0.8640 - val_loss: 0.6045 - val_acc: 0.8069
Epoch 27/50
391/391 [==============================] - 11s 29ms/step - loss: 0.3873 - acc: 0.8640 - val_loss: 0.5411 - val_acc: 0.8286
Epoch 28/50
391/391 [==============================] - 11s 29ms/step - loss: 0.3862 - acc: 0.8646 - val_loss: 0.4457 - val_acc: 0.8490
Epoch 29/50
391/391 [==============================] - 12s 29ms/step - loss: 0.3799 - acc: 0.8679 - val_loss: 0.5033 - val_acc: 0.8421
Epoch 30/50
391/391 [==============================] - 11s 29ms/step - loss: 0.3726 - acc: 0.8693 - val_loss: 0.4899 - val_acc: 0.8440
Epoch 31/50
391/391 [==============================] - 11s 29ms/step - loss: 0.3671 - acc: 0.8714 - val_loss: 0.5132 - val_acc: 0.8461
Epoch 32/50
391/391 [==============================] - 11s 29ms/step - loss: 0.3581 - acc: 0.8747 - val_loss: 0.7509 - val_acc: 0.7797
Epoch 33/50
391/391 [==============================] - 11s 29ms/step - loss: 0.3554 - acc: 0.8761 - val_loss: 0.5108 - val_acc: 0.8384
Epoch 34/50
391/391 [==============================] - 11s 29ms/step - loss: 0.3565 - acc: 0.8748 - val_loss: 0.4610 - val_acc: 0.8454
Epoch 35/50
391/391 [==============================] - 11s 29ms/step - loss: 0.3480 - acc: 0.8775 - val_loss: 0.4389 - val_acc: 0.8600
Epoch 36/50
391/391 [==============================] - 11s 29ms/step - loss: 0.3457 - acc: 0.8785 - val_loss: 0.5565 - val_acc: 0.8236
Epoch 37/50
391/391 [==============================] - 11s 29ms/step - loss: 0.3382 - acc: 0.8814 - val_loss: 0.5360 - val_acc: 0.8323
Epoch 38/50
391/391 [==============================] - 11s 29ms/step - loss: 0.3403 - acc: 0.8806 - val_loss: 0.5193 - val_acc: 0.8389
Epoch 39/50
391/391 [==============================] - 11s 29ms/step - loss: 0.3309 - acc: 0.8849 - val_loss: 0.4628 - val_acc: 0.8556
Epoch 40/50
391/391 [==============================] - 11s 29ms/step - loss: 0.3284 - acc: 0.8849 - val_loss: 0.5607 - val_acc: 0.8317
Epoch 41/50
391/391 [==============================] - 11s 29ms/step - loss: 0.3302 - acc: 0.8842 - val_loss: 0.5484 - val_acc: 0.8309
Epoch 42/50
391/391 [==============================] - 11s 29ms/step - loss: 0.3239 - acc: 0.8869 - val_loss: 0.4856 - val_acc: 0.8473
Epoch 43/50
391/391 [==============================] - 11s 29ms/step - loss: 0.3144 - acc: 0.8889 - val_loss: 0.5802 - val_acc: 0.8256
Epoch 44/50
391/391 [==============================] - 11s 29ms/step - loss: 0.3138 - acc: 0.8901 - val_loss: 0.5827 - val_acc: 0.8340
Epoch 45/50
391/391 [==============================] - 11s 29ms/step - loss: 0.3146 - acc: 0.8900 - val_loss: 0.3856 - val_acc: 0.8749
Epoch 46/50
391/391 [==============================] - 11s 29ms/step - loss: 0.3092 - acc: 0.8918 - val_loss: 0.5224 - val_acc: 0.8448
Epoch 47/50
391/391 [==============================] - 11s 29ms/step - loss: 0.3101 - acc: 0.8899 - val_loss: 0.4795 - val_acc: 0.8427
Epoch 48/50
391/391 [==============================] - 11s 29ms/step - loss: 0.3036 - acc: 0.8928 - val_loss: 0.4200 - val_acc: 0.8654
Epoch 49/50
391/391 [==============================] - 11s 28ms/step - loss: 0.3004 - acc: 0.8960 - val_loss: 0.4243 - val_acc: 0.8677
Epoch 50/50
391/391 [==============================] - 11s 29ms/step - loss: 0.2977 - acc: 0.8955 - val_loss: 0.4256 - val_acc: 0.8668
```

### Tensorflow
```

```